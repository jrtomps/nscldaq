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
# @file   CTCLStateChangeMessage.cpp
# @brief  Implement Tcl bindings to State change messages.
# @author <fox@nscl.msu.edu>
*/

#include "CTCLStateChangeMessage.h"
#include "CTCLRingStatistics.h"
#include <TCLInterpreter.h>
#include <TCLObject.h>
#include <TclUtilities.h>
#include <Exception.h>
#include <stdexcept>
#include <sstream>

unsigned CTCLStateChangeMessage::m_instanceNumber(0);

/**
 * constructor
 *    Constructs a instance of the class responsible for creating and destroying
 *    object wrappings.
 * @param interp - interpreter on which the command will be registered.
 * @param command - Command name.
 */
CTCLStateChangeMessage::CTCLStateChangeMessage(
    CTCLInterpreter& interp, const char* command
) :
    CTCLObjectProcessor(interp, command, true),
    m_testing(false)
{}

/**
 * destructor
 *    Destroys an instance of the class responsible for creating an destroying
 *    object wrappings.
 */
CTCLStateChangeMessage::~CTCLStateChangeMessage()
{
    
    // Delete all the registered wrappings:
    
    for (auto p = m_registry.begin() ; p != m_registry.end(); p++) {
        delete p->second;
    }
    // The map destroys itself properly now that the objects
    // its elements point to are gone.
}

/**
 * operator()
 *     Control passes to this method when the script wants to either create
 *     or destroy an object wrapping.  The command ensemble has two subcommands:
 *     
 *     -  create - makes a new Tcl command that wraps an object.
 *     -  destroy - destroys an existing mapping.
 *
 *     This method only:
 *     - Sets up a try/catch block to centralize/simplify error handling.
 *     - dispatches to the appropriate command handler based on the subcommand.
 *
 *  @param interp  - References the interpreter that is executing this command.
 *  @param objv    - Command words.
 *  @return int    - TCL_OK - if the command succeeded or TCL_ERROR if not.
 *  @note see the individual subcommand handlers to see if something is
 *  returned as the result of the command.
 *
 */
int
CTCLStateChangeMessage::operator()(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    bindAll(interp, objv);
    try {
        requireAtLeast(objv, 2, "Command requires at least a subcommand");
        
        std::string subcommand(objv[1]);
        
        if (subcommand == "create") {
            create(interp, objv);
        } else if (subcommand == "destroy") {
            destroy(interp, objv);
        } else {
            throw std::invalid_argument("Invalid subcommand");
        }
    }
    catch (std::exception & e) {
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
    
    return TCL_OK;
}
/*-----------------------------------------------------------------------------
 *  Command handlers for construction/destruction.
 */

/**
 * create
 *    Creates a wrapping of a CStatusDefinitions::StateChange.  In addition
 *    to the command and the subcommand, the command must have a zmq URI
 *    and an application name.  Note that the actual socket type created depends
 *    on the state of m_testing (PUSH if true else PUB).
 *
 *    The created wrapping is recorded in the registry so that we:
 *    - Can know it actually exists and can find it to destroy it.
 *    - When we are unregistered we can cleanup any existing objects.
 *
 *  @param interp  - References the interpreter that is executing this command.
 *  @param objv    - Command words.
 */
void
CTCLStateChangeMessage::create(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    requireExactly(objv, 4, "create requires a URI and an application name");
    std::string uri(objv[2]);
    std::string app(objv[3]);
    
    std::stringstream                cmdName;
    zmq::socket_t*                   pSocket(0);
    CStatusDefinitions::StateChange* pApiObject(0);
    TCLStateChangeMessage*           pWrapper(0);
    
    // Create the command name:
    
    cmdName << "statechange_" << m_instanceNumber++;
    
    // The rest is inside a try/catch block so we can clean up on errors:
    
    try {
        // In test mode we publish so that subscription tests don't need an
        // aggregator.  Otherwise we push at the aggregator.
        
        if (m_testing) {
             pSocket = new zmq::socket_t(
                TclMessageUtilities::m_zmqContext,  ZMQ_PUB
            );
            pSocket->bind(uri.c_str());           
        } else {
            pSocket = new zmq::socket_t(
                TclMessageUtilities::m_zmqContext, ZMQ_PUSH 
            );
            pSocket->connect(uri.c_str());
        }
        pApiObject  = new CStatusDefinitions::StateChange(*pSocket, app);
        pWrapper    = new TCLStateChangeMessage(
            interp, cmdName.str().c_str(), pApiObject, pSocket
        );
    }
    catch(...) {
        if (pWrapper) {   // deletes its components:
            delete pWrapper;
        } else {
            delete pApiObject;
            delete pSocket;
        }
        throw;                           // We're not processing the exception.
    }
    m_registry[cmdName.str()] = pWrapper;
    interp.setResult(cmdName.str());
}
/**
 * destroy
 *   Destroys an existing object mapping.  In addition to the command name, and
 *   subcommand, the script must supply the commad name of the object
 *   being destroyed.
 *
 *  @param interp - Interpreter executing the command.
 *  @param objv   - The words of the command.
 */
void
CTCLStateChangeMessage::destroy(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    requireExactly(objv, 3, "destroy needs the name of the command");
    std::string cmd(objv[2]);
    
    auto p = m_registry.find(cmd);
    if (p != m_registry.end()) {
        delete p->second;            // Destroy resources and
        m_registry.erase(p);         // the dict. entry.
    } else {
        throw std::invalid_argument("No such status message command to destroy");
    }
    
}
/*-----------------------------------------------------------------------------
 *  Implementation of the nested (wrapper) class.
 */

/**
 * TCLStateChangeMessage - constructor
 *   Wraps a CStatusDefinitions::StateChange object in a Tcl command.
 *
 * @param interp - interpreter on which the Tcl command is registered.
 * @param command - Command names string.
 * @param pObject - Object being wrapped.
 * @param pSock   - ZMQ socket that the object uses for message transport.
 */
CTCLStateChangeMessage::TCLStateChangeMessage::TCLStateChangeMessage(
    CTCLInterpreter& interp, const char* command,
    CStatusDefinitions::StateChange* pObject, zmq::socket_t* pSock
) :
    CTCLObjectProcessor(interp, command, true),
    m_pObject(pObject),
    m_pSocket(pSock)
{}

/**
 * TCLStateChangeMessage destructor
 *    Destroys both the wrapped object and the socket.
 */
CTCLStateChangeMessage::TCLStateChangeMessage::~TCLStateChangeMessage()
{
    delete m_pObject;
    delete m_pSocket;
}
/**
 * TclStateChangeMessage - operator()
 *     Gains control when a script wants to execute a method of the wrapped
 *     class.   The subcommand is the method name. Currently the only
 *     method supported is logChange.  That must have two state names.
 *     A state that is being left and a state that is being entered.
 *     Please note that at present, applications are on their honor to ensure
 *     that these states are legal and sensible.  Presumably they get them
 *     as a result of a state transition request, which will only give legal
 *     and reasonable next states.
 *
 *   @param interp - interpreter that is running the command.
 *   @param objv   - command line words.
 *   @return int   - TCL_OK on success, or TCL_ERROR on failure.
 *   @note This method only dispataches to a command handling method.  This
 *         allows our code to scale should additional methods be added to the
 *         wrapped object.  Our method also provides a top level try/catch
 *         block that centralizes error handling.
 */
int
CTCLStateChangeMessage::TCLStateChangeMessage::operator()(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    bindAll(interp, objv);
    try {
        requireAtLeast(objv, 2, "A subcommand is required");
        
        std::string subcommand(objv[1]);
        if (subcommand == "logChange") {
            logChange(interp, objv);
        } else {
            throw std::invalid_argument("Invalid subcommand.");
        }
    }
    catch (std::exception & e) {
        interp.setResult(e.what());
        return TCL_ERROR;
    }
    catch (CException & e) {
        interp.setResult(e.ReasonText());
        return TCL_ERROR;
    }
    catch(std::string msg) {
        interp.setResult(msg);
        return TCL_ERROR;
    }
    catch (const char* msg) {
        interp.setResult(msg);
        return TCL_ERROR;
    }
    catch (...) {
        interp.setResult("Unanticipated exception C++ type thrown");
        return TCL_ERROR;
    }
    
    return TCL_OK;
}
/**
 *  TclStateChangeMessage - logStateChange
 *
 *     Request the wrapped object to log a state change.  The command line
 *     must have a from and to state.
 *
 *   @param interp - interpreter that is executing the command.
 *   @param objv   - The command words as wrapped Tcl_Obj*s.
 *   
 */
void
CTCLStateChangeMessage::TCLStateChangeMessage::logChange(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    requireExactly(objv, 4, "logChange requires a from and a to state");
    std::string from = objv[2];
    std::string to   = objv[3];
    
    m_pObject->logChange(from, to);
}
