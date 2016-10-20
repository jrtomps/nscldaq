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
# @file   CTCLReadoutStatistics.cpp
# @brief  Implement Tcl bindings to the CStatusDefinitions::ReadoutStatistics class
# @author <fox@nscl.msu.edu>
*/
#include "CTCLReadoutStatistics.h"
#include "CTCLRingStatistics.h"
#include <TCLInterpreter.h>
#include <TCLObject.h>
#include <stdexcept>
#include <Exception.h>
#include <tcl.h>
#include <sstream>

unsigned CTCLReadoutStatistics::m_instanceCounter(0);

/**
 * constructor
 *   Outer class constructor
 *  @param interp - interpreter on which the command will be registered.
 *  @param cmd    - Command name string.  This is the first word to invoke the cmd.
 */
CTCLReadoutStatistics::CTCLReadoutStatistics(CTCLInterpreter& interp, const char* cmd) :
    CTCLObjectProcessor(interp, cmd, true),
    m_testing(false)
{}

/**
 * destructor
 *   Outer class destructor - kill off all objects in the registry.
 *   We let the map destroy itself, however.
 */
CTCLReadoutStatistics::~CTCLReadoutStatistics()
{
    for (auto p = m_registry.begin(); p != m_registry.end(); p++) {
        delete p->second;
    }
}


/**
 * operator()
 *   Executes the command.  The command is an ensemble of two subcommands:
 *   - create - constructs a wrapped object and registers it.
 *   - destroy - destroys a wrapped object.
 *
 *  For more information about each of those subcommands see the methods
 *  with the same names.
 *
 * @param interp - interpreter on which the command is executing.
 * @param objv   - The command words.
 * @return int   - TCL_OK - successful completion, TCL_ERROR failure.
 * @note  This method just does a top level dispatch based on the subcommand.
 * @note  Exception handling is used to centralize/simplify error management.
 */
int
CTCLReadoutStatistics::operator()(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    bindAll(interp, objv);
    try {
        requireAtLeast(objv, 2, "There must be at least a subcommand");
        std::string subcommand = objv[1];
        
        if (subcommand == "create") {
            create(interp, objv);
        } else if (subcommand == "destroy") {
            destroy(interp, objv);
        } else {
            throw std::invalid_argument("Invalid subcommand");
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
    catch (...) {
        interp.setResult("Unexpected C++ exception type caught.");
        return TCL_ERROR;
    }
    return TCL_OK;
    
}
/*----------------------------------------------------------------------------
 * Private methods for CTCLReadoutStatistics
 */

/**
 * create
 *    Create a wrapped object.  The command requires a URI parameter that
 *    describes what the zmq::socket will be connected to as well as allowing
 *    an optional application name (defaults to "Readout").  Note that
 *    we are going to dynamically create and connect a zmq::socket after which
 *    the wrapper will be responsible for its storage.  The zmq::context_t we're
 *    using for the socket is CTCLRingStatistics::m_zmqContext.
 *
 *  @param interp - interpreter that is running the command.
 *  @param objv   - Command words that make up the command.
 *
 *  @note on successful completion the name of the new command is set as the
 *        interpreter result.
 *        
 */
void
CTCLReadoutStatistics::create(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    zmq::socket_t*                         pConnection(0);
    CStatusDefinitions::ReadoutStatistics* pApiObject(0);
    TCLReadoutStatistics*                  pCommandObject(0);
    std::string app = "Readout";
    
    requireAtLeast(objv, 3, "Must have at least a zmq URI");
    requireAtMost(objv, 4, "Must not have more than a zmq URI and an appliation name");
    
    std::string uri = objv[2];
    
    if (objv.size() == 4) {
        app = std::string(objv[3]);
    }
    std::stringstream commandName;
    // Create and connect the socket.  The actual socket type depends on the
    // m_testing flag as pub/sub can be tough to use in testing.
    
    pConnection = new zmq::socket_t(
        CTCLRingStatistics::m_zmqContext, m_testing ? ZMQ_PUSH : ZMQ_PUB
    );
    // From now on we need to be in a try/catch block so resources can be
    // released on throws:
    try {
        pConnection->connect(uri.c_str());
        pApiObject = new CStatusDefinitions::ReadoutStatistics(*pConnection, app);
        
        commandName << "readoutstats_" << m_instanceCounter++;
        pCommandObject = new TCLReadoutStatistics(
            interp, commandName.str().c_str(), pApiObject, pConnection
        );
        m_registry[commandName.str()] = pCommandObject;
    }
    catch(...) {
        if(pCommandObject) delete pCommandObject;    // Deletes api object and sock
        else  {
            delete pApiObject;
            delete pConnection;
        }
        throw;
    }
    // Return the command name.
    
    interp.setResult(commandName.str());
}
/**
 * destroy
 *    Destroys an existing wrapping of a CStatusDefinition::ReadoutStatistics
 *    object.  This requires the command as a parameter.  It is an error
 *    to try to destroy a command that does not exist.
 *
 *  @param interp - interpreter that is running the command.
 *  @param objv   - Command words that make up the command.
 */
void
CTCLReadoutStatistics::destroy(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    requireExactly(objv, 3);
    std::string command = objv[2];   // The command to delete.
    auto p = m_registry.find(command);
    if (p != m_registry.end()) {
        delete p->second;
        m_registry.erase(p);
    } else {
        throw std::invalid_argument("The command you're trying to delete does not exist");
    }
}
/*-----------------------------------------------------------------------------
 * Implementation of the TCLReadoutStatistics innner class. This
 * class wraps an instance of CStatusDefinitions::ReadoutStatistics
 * in a Tcl command ensemble who's subcommand reflect the methods of the
 * encapsulated object.
 */

/**
 * TCLReadoutStatistics - constructor
 *
 * @param interp   - interpreter on which the wrapper command will be registered.
 * @param cmd      - The command name string.
 * @param object   - Pointer to the object to wrap.
 * @param sock     - Pointer to a ZMQ socket to be used to transport the messages.
 */
CTCLReadoutStatistics::TCLReadoutStatistics::TCLReadoutStatistics(
    CTCLInterpreter& interp, const char* cmd,
    CStatusDefinitions::ReadoutStatistics* object, zmq::socket_t* sock
) :
    CTCLObjectProcessor(interp, cmd, true),
    m_pObject(object), m_pSocket(sock)
{}

/**
 * TCLReadoutStatistics - destructor
 *   Need to destroy the wrapped object and the socket it uses for
 *   communications:
 */
CTCLReadoutStatistics::TCLReadoutStatistics::~TCLReadoutStatistics()
{
    delete m_pObject;
    delete m_pSocket; 
}

/**
 * TCLReadoutStatistics::operator()
 *     Gains control to execute the command.  The command is an ensemble that
 *     has the following subcommands:
 *     -  beginRun - register the run information and emit a message with that
 *                   information.
 *     - emitStatistics -Emits the readout statistics for an active run.
 *
 *  @note that this method just dispatches to the appropriate command execution
 *  method (named the same as the subcommand).  It also includes a try/catch
 *  block that centralizes/simplifies error handling/management.
 *
 *  @param interp - references the interpreter that is executing the command.
 *  @param objv   - Contains the words of the command.
 *  @return int   - TCL_OK - the method/command succeeded, TCL_ERROR method failed.
 */
int
CTCLReadoutStatistics::TCLReadoutStatistics::operator()(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
) 
{
    bindAll(interp, objv);
    
    try {
        requireAtLeast(objv, 2, "Command needs at least a subcommand");
        std::string subcommand(objv[1]);
            
        if (subcommand == "beginRun") {
            beginRun(interp, objv);            
        } else if (subcommand == "emitStatistics") {
            emitStatistics(interp, objv);
        } else {
            throw std::invalid_argument("Invalid subcommand");
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
        interp.setResult("Unanticipated C++ exception caught");
        return TCL_ERROR;
    }
    return TCL_OK;
}
/**
 * TCLReadoutStatistics - beginRun
 *    - Registers the begin run information.
 *    - Emits a message with a header and run Id information.
 *
 *   Command line parameters (in addition to the base command and subcommand) are:
 *   - An integer run number.
 *   - A title string.  Note that the title string will be truncated to at most
 *     79 characters in keeping with current Readout limitations.  This truncation
 *     will be silently performed.
 *
 *  @param interp - references the interpreter that is executing the command.
 *  @param objv   - Contains the words of the command.
 *  @return int   - TCL_OK - the method/command succeeded, TCL_ERROR method failed.
 */
void
CTCLReadoutStatistics::TCLReadoutStatistics::beginRun(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    requireExactly(objv, 4, "Command needs a run number and a title string");
    
    int run = objv[2];
    std::string title = std::string(objv[3]);
    if (title.size() >= 80) {
        title = title.substr(0, 79);            // Truncate.
    }
    m_pObject->beginRun(std::uint64_t(run), title);
}
/**
 * TCLReadoutStatistics - emitStatistics
 *   Emit a statistics message consisting of a header, the run id and
 *   a statistics message part.  Note that the command needs the following
 *   parameters in addition to the base and subcommands:
 *   -  triggers - number of triggers - this is an integer that must fit in
 *                 an uint64_t
 *   -  events   - Number of events that have been read.  This is an integer that
 *                 must fit into a uint64_t
 *   -  bytes    - Number of bytes that have been read. This too is an integer that
 *                 must fit into a uint64_t.
 *
 *    @note if the beginRun subcommand has not been issued at least once, this
 *          command will fail.
 *
 *  @param interp - references the interpreter that is executing the command.
 *  @param objv   - Contains the words of the command.
 *  @return int   - TCL_OK - the method/command succeeded, TCL_ERROR method failed.
 */
void
CTCLReadoutStatistics::TCLReadoutStatistics::emitStatistics(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    requireExactly(objv, 5, "Need number of triggers, events and bytes read");
    

    uint64_t nTriggers = CTCLRingStatistics::uint64FromObject(
        interp, objv[2], "Getting trigger count."
    );
    uint64_t nEvents  = CTCLRingStatistics::uint64FromObject(
        interp, objv[3], "Getting event count."
    );
    uint64_t nBytes   = CTCLRingStatistics::uint64FromObject(
        interp, objv[4], "Getting byte count."
    );
    
    m_pObject->emitStatistics(nTriggers, nEvents, nBytes);
}
