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
# @file   CTCLRingStatistics.cpp
# @brief  implement the RingStatistics constructor and instances.
# @author <fox@nscl.msu.edu>
*/

#include "CTCLRingStatistics.h"
#include "CStatusMessage.h"

#include <stdexcept>
#include <Exception.h>
#include <TCLException.h>
#include <zmq.hpp>
#include <sstream>

#include <tcl.h>

// Static data items:

// m_instanceNumber is used to create unique command names.

unsigned CTCLRingStatistics::m_instanceNumber(0);

// The zmq::context_t is required to generate objects:

zmq::context_t CTCLRingStatistics::m_zmqContext(1); 

/**
 * constructer(outer)
 *
 * @param interp - Interpreter on which the command is installed.
 * @param command - Command name string (argv0).
 */
CTCLRingStatistics::CTCLRingStatistics(CTCLInterpreter& interp, const char* cmd) :
    CTCLObjectProcessor(interp, cmd, true)
{}

/**
 * destructor - implies destructionof all instances too.
 */
CTCLRingStatistics::~CTCLRingStatistics()
{
    /**
     * loop over the registry killing off each instance object.
     * we're going to rely on the registry to kill off its elements
     * as std::map has a perfectly good destructor for that purpose.
     */
    for(auto p = m_registry.begin(); p != m_registry.end(); p++)
    {
        delete p->second;
    }
}
/**
 * operator()
 *    Fields the command.  The command is an ensemble that contains two
 *    subcommands:
 *    -   create - creates a new ring statistics objects and returns a 'handle'
 *        to it.  The object is entered in to the registry.  the handle is a
 *        new Tcl command.
 *    -   destroy - deletes an existing object, unregistering the command and
 *        removing the association between the command and an object fromt he
 *        registry.
 *  @param interp - References the interpreter on which this command will be
 *                  run and any object command registered/unregistered.
 *  @param objv   - The set of command words that make up the command.
 *  @note  This method will dispatch based on the subcommand to the create
 *         and destroy methods.  Exception handling is used for error management.
 *  @return int - TCL_OK - the object was created/destroyed.  TCL_ERROR
 *         something prevented the object from being create/destroyed.
 *         -  On successful creation, the new command/object handle is set as
 *            the command result.  On failure, the result is a human readable
 *            error message.
 *         -  On successful destructon, there is no result, however on error,
 *            the result is a human readable error message.
 *  @note for more informatioun see the create/destroy methods comment headers.
 */
int
CTCLRingStatistics::operator()(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    bindAll(interp, objv);
    try {
        // We need to have at least a subcommand:
        
        requireAtLeast(objv, 2, "Missing subcommand");
        std::string subcommand = objv[1];
        if (subcommand == "create") {
            create(interp, objv);
        } else if (subcommand == "destroy") {
            destroy(interp, objv);
        } else {
            throw  std::invalid_argument("Unrecognized subcommand");
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
    catch (...) {
        interp.setResult("Unanticipated C++ exception type caught");
        return TCL_ERROR;
    }
    
    return TCL_OK;
}
/*----------------------------------------------------------------------------
 *  Private utilities:
 *
 */

 /**
  * create
  *    Create a new instance of a CRingStatistics object wrapped in a Tcl command
  *    ensemble.
  *    -  Requires a zmq URI to which the ring statistics will connect
  *       as a client.  This results in the creation of a zmq::socket_t.
  *    -  An optional application name (defaulting to RingStatDaemon) can be
  *       supplied as well.
  *
  *     Action is to:
  *     -  Create a zmq::socket_t connected to the specified URI.
  *     -  Instantiate a CStatusDefinitions::RingStatistics object.
  *     -  Wrap that object in a CTCLRingStatistics::RingStatistics Tcl command
  *        object registered on this interpreter.  The command name is generated
  *        so that it is unique.
  *     -  Enter the command/object association into the registry.
  *     -  The command  name is the result on success.
  *     
  *  @param interp    - references the interpreter on which this command is
  *                     running and on which the new command is registered.
  *  @param objv      - Command words.   In addition to the base command and
  *                     the subcommand we must have a zmq URI and may have
  *                     an optional application name.
  *  @note all errors are signalled by throwing an exception.
  */
 void
 CTCLRingStatistics::create(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
 {
    CStatusDefinitions::RingStatistics* pApiObject(0);
    RingStatistics*                     pCommandObject(0);
    std::stringstream                   commandGenerator;
    
    // Validate the number of words we need on the command
    
    requireAtLeast(objv, 3, "Must have at least a zmq URI");
    requireAtMost(objv, 4, "Cannot have more that 4 command words");
    
    std::string uri = objv[2];
    std::string app ="RingStatDaemon";
    
    if (objv.size() == 4) {
        app = std::string(objv[3]);
    }
    
    // Create and connect the zmq socket, we're creating a PUB type socket.
    
    zmq::socket_t endpoint(m_zmqContext, ZMQ_PUB);
    endpoint.connect(uri.c_str());
    
    // This part is in a try /catch block so that resources can be recovered:
    
    try {
        
        // Make the underlying object:
        
        pApiObject = new CStatusDefinitions::RingStatistics(endpoint, app);
            
        // Wrap it in our command object:
    
        commandGenerator << "ringstat_" << m_instanceNumber++;
        pCommandObject  = new RingStatistics(
                interp, commandGenerator.str().c_str(),  pApiObject
        );
    }
    catch (...) {
        // Delete any objects that might have been instantiated dynamically:
        
        if (pCommandObject) delete pCommandObject; // deletes the api object
        else                delete pApiObject;
        
        throw;
    }
        
    // Register the object:
    
    m_registry[commandGenerator.str()] = pCommandObject;
    interp.setResult(commandGenerator.str());
    
    // Success means no exception.
 }
 /**
  * destroy
  *   Destroys a command object -- and by implication the underlying
  *   API object.
  *   -   We need a command/handle to destroy and that command must be in the
  *       registry.
  *   -   The object is removed from the registry.
  *   -   The object is destroyed.
  *
  * @param interp - The interpreter on which we are running and on which the
  *                 command we're being asked to destroy is registered.
  * @param objv   - The encapsulated command words that make up the command.
  */
 void
 CTCLRingStatistics::destroy(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
 {
    // We mut have the command to destroy:
    
    requireExactly(objv, 3);
    std::string command = objv[2];
    
    // If the command is not in the registry that's an error:
    
    auto pEntry = m_registry.find(command);
    if (pEntry == m_registry.end()) {
        throw std::invalid_argument("The command you're trying to delete does not exist");
    }
    // Since the object exists get it, remove the registry entry and
    // destroy the unerlying object:
    
    RingStatistics* pCommandObject = pEntry->second;
    m_registry.erase(pEntry);
    delete pCommandObject;
 }
 
 /*--------------------------------------------------------------------------
  *  Implementing the nested ringStatitics object:
  *
  */
 
 /**
  *  RingStatistics - constructor
  *     Create an instance of the command object:
  *
  *    @param  interp - references the interpreter on which this command is
  *                      being registered.
  *    @param  command - Command name.
  *    @param  pApi    - Pointer to an API object.  This must have been
  *                      dynamically generated and responsibility for its
  *                      destruction devolves to us.
  */
CTCLRingStatistics::RingStatistics::RingStatistics(
    CTCLInterpreter& interp, const char* command,
    CStatusDefinitions::RingStatistics* pApi
) :
 CTCLObjectProcessor(interp, command, true),
 m_pObject(pApi)
{
}
/**
 * RingStatistics - destructor
 *    Just deletes the api object:
 */
CTCLRingStatistics::RingStatistics::~RingStatistics()
{
    delete m_pObject;
}

/**
 * operator()
 *    Subcommand dispatcher.  This command is an ensemble with subcommands
 *    that reflect the meethods of the underlying api object:
 *    - startMessage - indicates the start of forming a message.
 *    - addProducer  - Adds a producer entry to the message.
 *    - addConsumer  - Adds a consumer entry to the message.
 *    - endMessage   - closes/sends the message.
 *
 *  This method just dispatches to an approprate action method that,  in turn
 *  marhsalls the command parameters into method parameters for the API object
 *  before invoking the appropriate API method.
 *
 * @param interp  - references the interpreter on which the command is running.
 * @parma objv    - The command words.  We  must have at least a subcommand
 *                  in addition to th base command.
 * @return int
 * @retval TCL_OK indicates the method run correctly.  No result is returned.
 * @retval TCL_ERROR indicates the method failed, an error message
 *                  is left in the result.
 *  @note exceptions are used to centralize error management.
 */
int
CTCLRingStatistics::RingStatistics::operator()(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    bindAll(interp, objv);
    try {
        requireAtLeast(objv, 2, "Command requires at least a subcommand");
        std::string subcommand = objv[1];
        
        if (subcommand == "startMessage") {
            startMessage(interp, objv);
        } else if (subcommand == "addProducer") {
            addProducer(interp, objv);
        } else if (subcommand == "addConsumer") {
            addConsumer(interp, objv);
        } else if (subcommand == "endMessage") {
            endMessage(interp, objv);
        } else {
            throw std::invalid_argument("Invalid subcommand for ring statistics");
        }
    }
    catch(std::exception& e) {
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
        interp.setResult("Unexpected C++ exception type caught");
        return TCL_ERROR;
    }
    return TCL_OK;
}
/**
 * RingStatistics - startMessage
 *    Begin a message.  In addition to the command and subcommand we need the
 *    name of a ring that is being reported by this message.
 *
 *  @param interp - interpreter that's running this command.
 *  @param objv   - Command words.
 */
void
CTCLRingStatistics::RingStatistics::startMessage(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    requireExactly(objv, 3, "The name of a ring is required");
    std::string ring = objv[2];
    
    m_pObject->startMessage(ring);
}
/**
 * RingStatistics - addProducer
 *    Add a producer to the status message.  Note that adding a producer is
 *    optional however only one producer can be added to the message.
 *    The command requires a list containing the words that make up the consumer
 *    command, a count of the operations (puts) performed on the ring and a count
 *    of the bytes sent into the ring.
 *
 * @param interp - interpreter on which this command is running.
 * @param objv   - Words that make up this command.
 */
void
CTCLRingStatistics::RingStatistics::addProducer(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    requireExactly(
        objv, 5,
        "adding a producer needs the producer command, an op count and the \
number of bytes put"
    );
    CTCLObject& command(objv[2]);           // marshall into vector of strings.
    
    // Marshall the command into the vector of strings required by the Api
    
    std::vector<std::string> cmdVector = stringVectorFromList(command);

    //  The operations and bytes need to be pulled out as uint64_t as they
    //  can get rather large:

    uint64_t ops = uint64FromObject(
        interp, objv[3], "Getting op count from command line"
    );
    uint64_t bytes = uint64FromObject(
        interp, objv[4], "Getting byte count from command line"
    );
    
    // Invoke the method on our object:
    
    m_pObject->addProducer(cmdVector, ops, bytes);
    
}
/**
 * RingStatistics - addConsumer
 *    Add a consumer to the message being formatted.  The addition of a consumer
 *    is optional.  Unlike producers any number of consumers can be added
 *    to the message.
 *
 * @param interp - the interpreter executing the command.
 * @param objv   - The command words.  There must be at least 5 parameters.
 *                 in addition to the base command and subcommand we need:
 *                 - List of strings making up the consumer command.
 *                 - Number of get operations done by that consumer.
 *                 - Number of bytes gotten by that consumer.
 */
void
CTCLRingStatistics::RingStatistics::addConsumer(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    requireExactly(
        objv, 5,
        "adding a consumer needs the command, number of gets and number of \
bytes gotten."
    );
    // Marshall the parameters:
    
    std::vector<std::string> command = stringVectorFromList(objv[2]);
    uint64_t                 ops     = uint64FromObject(
        interp, objv[3], "Getting the number of operations from the command"
    );
    uint64_t                 bytes   = uint64FromObject(
        interp, objv[4], "Getting the bytes transferred from the command"
    );
    
    // Now we can invoke the api method:
    
    m_pObject->addConsumer(command, ops, bytes);
    
}
/**
 * endMessage
 *    Ends the message being built up.  This is what actually causes the
 *    message to be sent out over whatever transport the underlying
 *    socket_t is using.  Note that messages are always PUB'd (see the
 *    outer class create method), however the actual connection was specified
 *    at creation time by supplying a URI to the creation operation.
 *
 * @param interp - interpreter that is executing this command.
 * @param objv   - Objects that represent the command words.  There should
 *                 not be any additional parameters above the base command
 *                 and subcommands.
 */
void
CTCLRingStatistics::RingStatistics::endMessage(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    requireExactly(objv, 2, "endMessage takes no additional command parameters");
    m_pObject->endMessage();
}

/*---------------------------------------------------------------------------
 *  Private utility methods of the RingStatistics nested class:
 */
/**
 * stringVectorFromList
 *    Turn a CTCLObject that contains a list to an std::vector<std::string.
 *    The object is assumed bound to an interpreter.
 *  @param obj - The object being analyzed.
 *  @return std::vector<std::string>
 */
std::vector<std::string>
CTCLRingStatistics::RingStatistics::stringVectorFromList(CTCLObject& obj)
{
    std::vector<std::string> result;
        for (int i = 0; i < obj.llength(); i++) {
        result.push_back(std::string(obj.lindex(i)));
    }
    return result;
}

/*
 * uint64FromObject
 *    Fetches a uint64_t from a CTCLObject.
 *
 *  @param interp - interpreter to use to parse the object.
 *  @param obj    - Object we're getting the value from.
 *  @param pDoing - String documenting what's being done.  This is part of the
 *                  error exception if the parse fails.
 *  @return uint64_t
 *  @throw  CTCLException if the parse fails.
 */
uint64_t
CTCLRingStatistics::RingStatistics::uint64FromObject(
    CTCLInterpreter& interp, CTCLObject& obj, const char* pDoing
)
{
    static_assert(
        sizeof(long) >= sizeof(uint64_t),
        "Long is not wide enough for a uint64_t"
    );   // Ensure we're not chopping.
    
    uint64_t result;
    Tcl_Obj* tclObj = obj.getObject();
    int status = Tcl_GetLongFromObj(interp.getInterpreter(), tclObj, reinterpret_cast<long*>(&result));
    if (status != TCL_OK) {
        throw CTCLException(
            interp, 0, "Failed to get number of operations from command line"
        );       
    }
    return result;
}