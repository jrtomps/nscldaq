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