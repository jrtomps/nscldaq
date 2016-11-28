/**

#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2013.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Author:
#            Ron Fox
#            NSCL
#            Michigan State University
#            East Lansing, MI 48824-1321

##
# @file   CTCLDecodeMessage.cpp
# @brief  Decode status messages into Tcl usable format.
# @author <fox@nscl.msu.edu>
*/

#include "CTCLDecodeMessage.h"
#include <TCLInterpreter.h>
#include <TCLInterpreterObject.h>
#include <stdexcept>
#include <Exception.h>
#include "CStatusMessage.h"
#include <tcl.h>
#include "TclUtilities.h"

/**
 * constructor
 *    Construct the new command
 *
 *  @param interp  - references the interpreter on which the command will be
 *                   registered.
 *  @param command - Base command string (note this is an ensemble).
 */
CTCLDecodeMessage::CTCLDecodeMessage(CTCLInterpreter& interp, const char* command) :
    CTCLObjectProcessor(interp, command, true)
{}

/**
 * destructor
 */
CTCLDecodeMessage::~CTCLDecodeMessage() {}

/**
 * operator()
 *    Called when the command is being executed.  
 *    - We decode the header part and dispatch to the appropriate
 *      decoding part.
 *
 *  @param interp - interpreter that is executing the command.
 *  @param objv   - Command words.
 *  @return int   - TCL_OK _ successful completion.   TCL_ERROR failed.
 *  @note In general the command result on success is a list of decoded message
 *        segments.  Note that the format of these will depend on the message
 *        type.
 */
int
CTCLDecodeMessage::operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    bindAll(interp, objv);
    
    // Exception handling both centralizes and simplifies error/result management./
    
    try {
        requireExactly(objv, 2, "statusdecode takes exactly one parameter - the message part list");
        bindAll(interp, objv);
        CTCLObject messageParts = objv[1];
        messageParts.Bind(interp);
        
        CTCLObject headerObj  = messageParts.lindex(0);
        headerObj.Bind(interp);
        
        const CStatusDefinitions::Header* header = extractHeader(headerObj);
        
        // Switch based on header type:
        
        if (header->s_type == CStatusDefinitions::MessageTypes::RING_STATISTICS) {
            decodeRingStatistics(interp, messageParts);
        } else if (header->s_type == CStatusDefinitions::MessageTypes::LOG_MESSAGE) {
            decodeLogMessage(interp, messageParts);
        } else if (header->s_type == CStatusDefinitions::MessageTypes::READOUT_STATISTICS) {
            decodeReadoutStatistics(interp, messageParts);
        } else if (header->s_type == CStatusDefinitions::MessageTypes::STATE_CHANGE) {
            decodeStateChange(interp, messageParts);
        } else {
            throw std::invalid_argument("Unsupported message type");
        }
        
    }
    catch (std::exception& e) {
        interp.setResult(e.what());
        return TCL_ERROR;
    }
    catch (CException& e) {
        interp.setResult(e.ReasonText());
        return TCL_ERROR;
    }
    catch (std::string msg) {
        interp.setResult(msg);
        return TCL_ERROR;
    }
    catch (const char* msg) {
        interp.setResult(msg);
        return TCL_ERROR;
    }
    catch(...) {
        interp.setResult("statusdecode - unanticipated C++ exception type");
        return TCL_ERROR;
    }
    
    return TCL_OK;
}

/*----------------------------------------------------------------------------
 * Private utilities
 */

/**
 * decodeRingStatistics
 *    Decodes a message that is a ring statistics message.
 *    This produces a list of dict objects. The first dict in the list is
 *    the header.  The second a ring identification, the remaining
 *    are consumer records.
 *
 *  @param interp  - Interpreter that is involved in Tcl_Obj/CTCLObject operations.
 *  @param obj     - CTCLObject that is a list of message parts.
 */
void
CTCLDecodeMessage::decodeRingStatistics(CTCLInterpreter& interp, CTCLObject& msg)
{
    CTCLObject result;
    result.Bind(interp);
    
    // First decode the message header and ring id parts.  Those are
    // always present:
    
    CTCLObject headerObj = msg.lindex(0);
    headerObj.Bind(interp);
    CTCLObject decodedHeader;
    decodedHeader = decodeHeader(interp, *extractHeader(headerObj));
    decodedHeader.Bind(interp);
    result += decodedHeader;
    
    CTCLObject idObj = msg.lindex(1);
    idObj.Bind(interp);
    CTCLObject decodedRingId;
    decodedRingId = decodeRingIdent(interp, *extractRingId(idObj));
    decodedRingId.Bind(interp);
    result += decodedRingId;
    
    // All remaining message parts are RingStatClients:
    
    for (int i = 2; i < msg.llength(); i++) {
        CTCLObject clientObj = msg.lindex(i);
        clientObj.Bind(interp);
        CTCLObject decodedClientObj =
            decodeRingClientInfo(interp, *extractRingClientInfo(clientObj));
        decodedClientObj.Bind(interp);
        
        result += decodedClientObj;
    }
    interp.setResult(result);
}
/**
 * decodeLogMessage
 *    Decodes a log message.  This is a header and a single log message body.
 * @param interp - interpreter running the command.
 * @param msg    - Object containing a list of message body parts.
 *
 * The result is set with the list of decoded message parts.
 */
void
CTCLDecodeMessage::decodeLogMessage(CTCLInterpreter& interp, CTCLObject& msg)
{
    CTCLObject result;
    result.Bind(interp);
    
    // Log messages have a header and a body only:
    
    CTCLObject hdrObj = msg.lindex(0);
    hdrObj.Bind(interp);
    
    CTCLObject bodyObj = msg.lindex(1);
    bodyObj.Bind(interp);
    
    // Add the decoded header to the result:
    
    CTCLObject decodedHeader = decodeHeader(interp, *extractHeader(hdrObj));
    decodedHeader.Bind(interp);
    result += decodedHeader;
    
    // Add the decoded body to the result too:
    
    CTCLObject decodedBody = decodeLogBody(interp, *extractLogMessageBody(bodyObj));
    decodedBody.Bind(interp);
    result += decodedBody;
    
    interp.setResult(result);
}
/**
 * decodeReadoutStatistics
 *   Readout statistics are messages with two or three parts.  The first part is,
 *   as always ta header while the second, mandatory part, identifies the
 *   run.  The final, optional part, identifies statistics associated with
 *   the run.
 *
 *  @param interp - TCLInterpreter that is running this command.
 *  @param msg    - list of raw message parts.
 *  
 *  @note On success, the interpreter result is set with the decoded message
 *        parts.  See the individual members used to decode each message part
 *        for information about the format of these decoded parts.
 *        On error, an exception is thrown.
 */
void
CTCLDecodeMessage::decodeReadoutStatistics(CTCLInterpreter& interp, CTCLObject& msg)
{
    CTCLObject result;
    result.Bind(interp);
    
    // Pull out the required message parts:
    
    CTCLObject hdrObj = msg.lindex(0);
    hdrObj.Bind(interp);
    
    CTCLObject runInfoObj = msg.lindex(1);
    runInfoObj.Bind(interp);
    
    // Add the decoded header to the result:
    
    CTCLObject decodedHeader = decodeHeader(interp, *extractHeader(hdrObj));
    decodedHeader.Bind(interp);
    result += decodedHeader;
    
    // Add the run id information to the result:
    
    CTCLObject decodedRunInfo = decodeRunStatInfo(interp, *extractRunStatInfo(runInfoObj));
    decodedRunInfo.Bind(interp);
    result += decodedRunInfo;
    
    // If there's a statistics part extract and decode it.
    
    if (msg.llength() >= 3) {
        CTCLObject runStatsObj = msg.lindex(2);
        runStatsObj.Bind(interp);
        CTCLObject readoutStats = decodeReadoutCounters(interp, *extractReadoutCounters(runStatsObj));
        readoutStats.Bind(interp);
        result += readoutStats;
    }
    
    interp.setResult(result);
}
/**
 * decodeStateChange
 *    decode a state change message into a pair of dicts, one for each of the
 *    message parts that are mandatory and all there are for a state change
 *    message.
 *
 *  @param interp - Interpreter that's running the command.
 *  @param msg    - Object containing the message parts.
 *  @note  - On success, the interpreteer result is set to the list of decoded
 *           message part dicts (see individual methods used for more information
 *           about the keys each dict has).  On failure an exception is thrown.
 */
void
CTCLDecodeMessage::decodeStateChange(CTCLInterpreter& interp, CTCLObject& msg)
{
    CTCLObject result;
    result.Bind(interp);
    
    CTCLObject headerObj = msg.lindex(0);
    headerObj.Bind(interp);
    
    CTCLObject bodyObj   = msg.lindex(1);
    bodyObj.Bind(interp);
    
    // Add the decoded header to the result:
    
    CTCLObject decodedHeader = decodeHeader(interp, *extractHeader(headerObj));
    decodedHeader.Bind(interp);
    result += decodedHeader;
    
    // Add the decoded body to the result as well:
    
    CTCLObject decodedBody = decodeStateChangeBody(
        interp,
        *extractStateChangeBody(bodyObj)
    );
    decodedBody.Bind(interp);
    result += decodedBody;
    
    // Return the resulting list:
    
    interp.setResult(result);
}
/**
 * extractHeader
 *    Given a TclObject containing a byte array that is actually a body header,
 *    extracts the body header and returns it.
 *
 *  @param headerObj                           - the CTCLObject containing the header.
 *  @return const CStatusDefinitions::Header*  - a pointer to the header.
 */
const CStatusDefinitions::Header*
CTCLDecodeMessage::extractHeader(CTCLObject& headerObj)
{
    int size;
    Tcl_Obj* headerByteArray = headerObj.getObject();
    CStatusDefinitions::Header* pHeader =
        reinterpret_cast<CStatusDefinitions::Header*>(
            Tcl_GetByteArrayFromObj(headerByteArray, &size)
        );
    
    if (size != sizeof(CStatusDefinitions::Header)) {
        throw std::invalid_argument("Message header is invalid -- wrong size");
    }
    
    return pHeader;              // Does bitwise copy construction.
}
/**
 * decodeHeader
 *    Creates a header dict. object.  The header dict has the following key/values:
 *    -  type        - Stringified type of message.
 *    -  severity    - Stringified severity of message.
 *    -  application - Application name.
 *    -  source      - Source of message (FQDN)
 *
 *  @param interp - interpreter used to play with object.
 *  @param hdr    - const reference to a header struct.
 */
CTCLObject
CTCLDecodeMessage::decodeHeader(
    CTCLInterpreter& interp, const CStatusDefinitions::Header& hdr
)
{
    Tcl_Obj* resultDict = Tcl_NewDictObj();
    CTCLObject result(resultDict);
    result.Bind(interp);
    
    std::string type     = TclMessageUtilities::messageTypeToString(hdr.s_type);
    std::string severity = TclMessageUtilities::severityToString(hdr.s_severity);
    
    //  Now we can build the dict:
    
    TclMessageUtilities::addToDictionary(interp, result, "type", type.c_str());
    TclMessageUtilities::addToDictionary(interp, result, "severity", severity.c_str());
    TclMessageUtilities::addToDictionary(interp, result, "application", hdr.s_application);
    TclMessageUtilities::addToDictionary(interp, result, "source", hdr.s_source);
    
    // Return the result (copy constructed)
    
    return result;
}
/**
 * extractRingId
 *    Extract a ring status id struct from a CTCLObject.  The Object is assumed
 *    to contain a byte array that is the struct we really want.
 *
 * @param ringidObj - CTCLObject& reference that contains the strut we want.
 * @return const CMessageDefinitions::RingStatIdentification* - pointer to the
 *          struct we need to extract.
 */
const CStatusDefinitions::RingStatIdentification*
CTCLDecodeMessage::extractRingId(CTCLObject& ringIdObj)
{
    Tcl_Obj* ringIdByteArray = ringIdObj.getObject();
    int size;
    
    return reinterpret_cast<CStatusDefinitions::RingStatIdentification*>(
        Tcl_GetByteArrayFromObj(ringIdByteArray, &size)
    );
}
/**
 * decodeRingIdent
 *   Decode a ring ident into a Tcl usable form.  The form chosen is dict
 *   with the following key/value pairs:
 *
 *   -  timestamp - a [clock seconds] timestamp
 *   -  name      - Name of the ring.
 *
 *  @param interp - interpreter used to diddle with objects.
 *  @param id     - References the id itself.
 *  @return CTCLObject containing the dict.
 */
CTCLObject
CTCLDecodeMessage::decodeRingIdent(
    CTCLInterpreter& interp,
    const CStatusDefinitions::RingStatIdentification& id
)
{
    CTCLObject result;
    result.Bind(interp);
    
    TclMessageUtilities::addToDictionary(interp, result, "timestamp", id.s_tod);
    TclMessageUtilities::addToDictionary(interp, result, "name", id.s_ringName);
    
    return result;
}
/**
 * extractRingClientInfo
 *    Given a CTCLObject that is believed to have a ring client struct,
 *    return a pointer to the underlying ring client struct.
 *
 *  @param clientObj - references the CTCLOBjectthat contains the struct.
 *  @return  const CStatusDefinitions::RingStatClient* - pointer to the underlying
 *      struct.
 */
const CStatusDefinitions::RingStatClient*
CTCLDecodeMessage::extractRingClientInfo(CTCLObject& clientObj)
{
    Tcl_Obj* clientByteArray = clientObj.getObject();
    int size;
    return reinterpret_cast<const CStatusDefinitions::RingStatClient*>(
        Tcl_GetByteArrayFromObj(clientByteArray, &size)
    );
}
/**
 * decodeRingClient
 *    Create a dict that describes a ring client message part.
 *    The dict will have the following key/value pairs.
 *
 *    - ops      - Operations performed by that client
 *    - bytes    - Bytes transferred by that client.
 *    - producer - Boolean that is true if the client is a producer.
 *    - command  - list of strings that make up the command word.
 *    - backlog  - Number of bytes of backlog (consumers).
 *    - pid      - PID of client.
 *
 *  @param interp - interpreter that will be used to encode/decode Tcl Objects.
 *  @param client - Pointer to the message part struct.
 *  @return CTCLObject - Resulting dict object.
 */
CTCLObject
CTCLDecodeMessage::decodeRingClientInfo(
    CTCLInterpreter& interp, const CStatusDefinitions::RingStatClient& client
)
{
    CTCLObject result;
    result.Bind(interp);
    
    TclMessageUtilities::addToDictionary(interp, result, "ops", client.s_operations);
    TclMessageUtilities::addToDictionary(interp, result, "bytes", client.s_bytes);
    TclMessageUtilities::addToDictionary(interp, result, "backlog", client.s_backlog);
    TclMessageUtilities::addToDictionary(interp, result, "pid", client.s_pid);
    TclMessageUtilities::addToDictionary(
        interp, result, "producer", client.s_isProducer
    );
    CTCLObject stringList =
        TclMessageUtilities::listFromStringList(interp, client.s_command);
    stringList.Bind(interp);
    TclMessageUtilities::addToDictionary(
        interp, result, "command", stringList 
    );
    
    
    return result;  
}
/**
 * extractLogMessageBody
 *    Converts a byte array object into a pointer to a log message body.
 *    It's up to the caller to ensure that's actually whaty the object contains.
 *
 *  @param obj - The byte array object.
 *  @return CStatusDefinitions::LogMessageBody*
 */
const CStatusDefinitions::LogMessageBody*
CTCLDecodeMessage::extractLogMessageBody(CTCLObject& logBodyObj)
{
    Tcl_Obj* clientByteArray = logBodyObj.getObject();
    int size;
    return reinterpret_cast<const CStatusDefinitions::LogMessageBody*>(
        Tcl_GetByteArrayFromObj(clientByteArray, &size)
    );
}

/**
 * decodeLogBody
 *    Given a log message body object, produces a TCL dict that represents that
 *    body. This will be a dict with the following key/values:
 *    - timestamp - [clock seconds] timestamp.
 *    - message   - text fo the message.
 *
 * @param interp - interpreter that will be used in object manipulation.
 * @param body   - reference to the body struct.
 * @return CTCLObject - Dict.
 */
CTCLObject
CTCLDecodeMessage::decodeLogBody(
    CTCLInterpreter& interp, const CStatusDefinitions::LogMessageBody& body
)
{
    CTCLObject result;
    result.Bind(interp);
    
    TclMessageUtilities::addToDictionary(interp, result, "timestamp", body.s_tod);
    TclMessageUtilities::addToDictionary(interp, result, "message", body.s_message);
    
    return result;
}
/**
 * extractRunStatInfo
 *    Given a CTCLObject reference, assume it contains a byte array,
 *    Pull a pointer to the bytes back out cast to a
 *    ReadoutStateRunInfo struct.
 *
 *  @param obj - The object we're extracting from.
 *  @return const CStatusDefinitions::ReadoutStatRunInfo*
 */
const CStatusDefinitions::ReadoutStatRunInfo*
CTCLDecodeMessage::extractRunStatInfo(CTCLObject& obj)
{
    Tcl_Obj* rawObject = obj.getObject();
    int size;
    return reinterpret_cast<const CStatusDefinitions::ReadoutStatRunInfo*>(
        Tcl_GetByteArrayFromObj(rawObject, &size)
    );
}
/**
 * decodeRunStateInfo
 *   Take a ReadoutStateRunInfo struct and turn it into a Tcl dict.
 *   The resulting dict is then returned to the caller.  The dict has the
 *   following fields:
 *   -  starttime   - [clock seconds] when the run started.
 *   -  run         - run number.
 *   -  title       - Run title.
 *
 *  @param interp   - interpreter used in object maniuplations.
 *  @param runInfo  - Run information message part.
 *  @return CTCLObject - see above.
 */
CTCLObject
CTCLDecodeMessage::decodeRunStatInfo(
    CTCLInterpreter& interp,
    const CStatusDefinitions::ReadoutStatRunInfo& runInfo
)
{
    CTCLObject result;
    result.Bind(interp);
    
    TclMessageUtilities::addToDictionary(
        interp, result, "starttime", runInfo.s_startTime
    );
    TclMessageUtilities::addToDictionary(interp, result, "run", runInfo.s_runNumber);
    TclMessageUtilities::addToDictionary(interp, result, "title", runInfo.s_title);
    
    return result;
}
/**
 * extractReadoutCounters
 *    Given a CTCLObject that is a byte array, returns a pointer to the object's
 *    data that is cast as a CStatusDefinitions::ReadoutStatCounters*.
 *
 *  @param obj  - Object we're operating on.
 *  @return const const CStatusDefinitions::ReadoutStatCounters*
 */
const CStatusDefinitions::ReadoutStatCounters*
CTCLDecodeMessage::extractReadoutCounters(CTCLObject& obj)
{
    Tcl_Obj* rawObject = obj.getObject();
    int size;
    
    return reinterpret_cast<const CStatusDefinitions::ReadoutStatCounters*>(
        Tcl_GetByteArrayFromObj(rawObject, &size)
    );
}
/**
 * decodeReadoutCounters
 *    Given a ReadoutStatCounters message part, produce/return a dict
 *    that contains the information in that message part.  The dict
 *    will have the following keys:
 *    -  timestamp   - Time at which the message was produced.
 *    -  elapsed     - Number of seconds into the run at which the message was produced.
 *    -  triggers    - Number of triggers processed.
 *    -  events      - Number of events produced.
 *    -  bytes       - Number of bytes produced.
 *
 *  @param interp   - interpreter used to perform object operations that need one
 *  @param counters - The decodeReadoutCounters message part.
 *  @return CTCLObject - The resulting dict (see above).
 */
CTCLObject
CTCLDecodeMessage::decodeReadoutCounters(
    CTCLInterpreter& interp,
    const CStatusDefinitions::ReadoutStatCounters& counters
)
{
    CTCLObject result;
    result.Bind(interp);
    
    TclMessageUtilities::addToDictionary(
        interp, result, "timestamp", counters.s_tod
    );
    TclMessageUtilities::addToDictionary(
        interp, result, "elapsed", counters.s_elapsedTime
    );
    TclMessageUtilities::addToDictionary(
        interp, result, "triggers", counters.s_triggers
    );
    TclMessageUtilities::addToDictionary(
        interp, result, "events", counters.s_events
    );
    TclMessageUtilities::addToDictionary(
        interp, result, "bytes", counters.s_bytes
    );
    
    return result;
}
/**
 * extractStateChangeBody
 *     Given a CTCLObject that has a byte array that is actually a
 *     CStatusDefinitions::StateChangeBody returns a pointer to that body.
 *
 *   @param obj - The object from which we're extracting the pointer.
 *   @return const CStatusDefinitions::StateChangeBody* 
 */
const CStatusDefinitions::StateChangeBody*
CTCLDecodeMessage::extractStateChangeBody(CTCLObject& obj)
{
    Tcl_Obj* rawObject = obj.getObject();
    int size;
    
    return reinterpret_cast<const CStatusDefinitions::StateChangeBody*>(
        Tcl_GetByteArrayFromObj(rawObject, &size)
    );
}
/**
 * decodeStateChangeBody
 *   Given a StateChangeBody reference, returns a CTCLObject that is a dict
 *   containng the following fields:
 *   - timestamp -  Time at which the state change item was emitted.
 *   - leaving   - State being left.
 *   - entering  - State being entered.
 *
 *  @param interp    - Interp being used to do object operations.
 *  @param body      - Body message part for a state change message.
 *  @return CTCLObject
 * 
 */
CTCLObject
CTCLDecodeMessage::decodeStateChangeBody(
    CTCLInterpreter& interp,
    const CStatusDefinitions::StateChangeBody& body
)
{
    CTCLObject result;
    result.Bind(interp);
    
    TclMessageUtilities::addToDictionary(
        interp, result, "timestamp", body.s_tod
    );
    TclMessageUtilities::addToDictionary(
        interp, result, "leaving", body.s_leaving
    );
    TclMessageUtilities::addToDictionary(
        interp, result, "entering", body.s_entering
    );
    
    return result;
}