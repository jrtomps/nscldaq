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
# @file   CTCLLogMessage.cpp
# @brief  Implementation of Tcl wrappers for CStatusDefinitions::LogMessage
# @author <fox@nscl.msu.edu>
*/
#include "CTCLLogMessage.h"
#include "CTCLRingStatistics.h"
#include <TCLInterpreter.h>
#include <TCLObject.h>
#include <stdexcept>
#include <Exception.h>
#include <sstream>


unsigned CTCLLogMessage::m_instanceNumber(0);

static std::map<std::string, uint32_t> severityTable = {
    {"INFO", CStatusDefinitions::SeverityLevels::INFO}   ,
    {"DEBUG", CStatusDefinitions::SeverityLevels::DEBUG} ,
    {"WARNING", CStatusDefinitions::SeverityLevels::WARNING},
    {"SEVERE",  CStatusDefinitions::SeverityLevels::SEVERE},
    {"DEFECT",  CStatusDefinitions::SeverityLevels::DEFECT}
};

/**
 * constructor
 *    Construct the creator/destroyer command object.
 *
 *  @param interp  - interpreter on which the command will be registered.
 *  @param command - The command name.
 */

CTCLLogMessage::CTCLLogMessage(CTCLInterpreter& interp, const char* command) :
    CTCLObjectProcessor(interp, command, true)
{}

/**
 * destructor
 *    Destroys the make/destroy command.  This also destroys all instance commands
 *    that were created by this command.
 */
CTCLLogMessage::~CTCLLogMessage()
{
    for (auto p = m_registry.begin(); p != m_registry.end(); p++) {
        delete p->second;
    }
    // The map will delete itself.
}

/**
 *  operator()
 *     Gets control when a script wants to create or destroy a LogMessage
 *     wrapper object.  The base command is an ensemble with two subcommands:
 *     -  create - Creates a new LogMessage wrapper.
 *     -  destroy - Destroys an existing LogMessage wrapper.
 *
 *     What this method does is only:
 *     - Dispatch to the appropriate processing method (named the same as the
 *       subcommand).
 *     - Centralize error handling via a try/catch block.
 *
 *  @param interp - interpreter on which the command is running.
 *  @param objv   - Command word objects.
 *  @return int   - TCL_OK - The command succeeded.  TCL_ERROR - the command failed.
 */
int
CTCLLogMessage::operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    bindAll(interp, objv);
    try {
        requireAtLeast(objv, 2, "Command needs at least a subcommand");
        std::string subcommand(objv[1]);
        
        if (subcommand == "create") {
            create(interp, objv);
        } else if (subcommand == "destroy") {
            destroy(interp, objv);
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
    catch (...) {
        interp.setResult("Unanticipated C++ exception type caught");
        return TCL_ERROR;
    }
    return TCL_OK;
}
/*---------------------------------------------------------------------------
 *  Command executors for the outer class
 */

/**
 * create
 *    Create a new wrapped class.  This requires at a URI and an application
 *    name.
 *
 *  @param interp - interpreter on which the command is running.
 *  @param objv   - Command words.
 */
void
CTCLLogMessage::create(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    // We need exactly two additional parameters (4 total).
    
    requireExactly(objv, 4, "create requires a connection URI and an appname");
    
    std::string uri(objv[2]);
    std::string app (objv[3]);
    
    // Create the socket and the API object:
    
    zmq::socket_t*                   pSocket(0);
    CStatusDefinitions::LogMessage* pApiObject(0);
    TCLLogMessage*                   pWrapperObject(0);
    std::stringstream                commandStream;
    try {
        pSocket = new zmq::socket_t(
            CTCLRingStatistics::m_zmqContext, m_testing ? ZMQ_PUSH : ZMQ_PUB
        );
        pSocket->connect(uri.c_str());
        pApiObject = new CStatusDefinitions::LogMessage(*pSocket, app);

        // Construct the new command name and the wrapper object:
        
        commandStream << "log_" << m_instanceNumber++;
        pWrapperObject =
            new TCLLogMessage(interp, commandStream.str().c_str(), pApiObject, pSocket);
        m_registry[commandStream.str()] = pWrapperObject;
        
    }
    catch (...) {
        if (pWrapperObject) {
            delete pWrapperObject;            // Kills everthing else.
        } else {
            delete pApiObject;
            delete pSocket;
        }
    }
}
/**
 * destroy
 *    Destroys a Tcl wrapper for a CStatusDefinitions::LogMessage object.
 *    The wrapper is referred to by its command name which is a required
 *    command line parameter.
 *
 *  @param interp - the interpreter that is running this command.
 *  @param objv   - The words that make up the command.
 */
void
CTCLLogMessage::destroy(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    requireExactly(objv, 3, "Destroy requires the name of the command to destroy");
    std::string command(objv[2]);
    
    auto p = m_registry.find(command);
    if (p != m_registry.end()) {
        delete p->second;            // Destroy the object.
        m_registry.erase(p);         // remove it from the registry.
    } else {
        throw std::invalid_argument("No Such command in destroy");
    }
}
/*-----------------------------------------------------------------------------
 *  Implementation of the interal instance class: TCLLogMessage:
 */

/**
 * TCLLogMessage - constructor
 *    Construct a new TCLLogMessage - this is a Tcl command ensemble that
 *    wraps a CStatusDefinitions::LogMessage object.  The subcommands
 *    of the ensemble are the methods of the class..
 *
 *  @param interp - interpreter on which the command will be registered.
 *  @param command - Command name string.
 *  @param pObject - The C++ API object to wrap.
 *  @param pSocket - The zmq::socket_t used by pObject for message transport.
 *  @note This object is responsible for disposing of both pObject and pSocket.
 */
CTCLLogMessage::TCLLogMessage::TCLLogMessage(
    CTCLInterpreter& interp, const char* command,
    CStatusDefinitions::LogMessage* pObject, zmq::socket_t* pSocket
) :
    CTCLObjectProcessor(interp, command, true),
    m_pObject(pObject),
    m_pSocket(pSocket)
{}

/**
 * TCLLogMessage - destructor
 *    Disposes of the api object and the socket it used to transport messages
 */
CTCLLogMessage::TCLLogMessage::~TCLLogMessage()
{
    delete m_pObject;
    delete m_pSocket;
}

/**
 * TCLLogMessage  - operator()
 *    Called when a method on the underlying object needs to be invoked.
 *    We just dispatch the subcommand to the appropriate handler function.
 *    At present subcommand are just:
 *    - log - Log a message.
 *    
 * @param interp -  interpreter executing the command.
 * @param objv   - The command words.
 * @return int   - TCL_OK - the operation succeeded.  TCL_ERROR if not.
 */
int
CTCLLogMessage::TCLLogMessage::operator()(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    bindAll(interp, objv);
    try {
        requireAtLeast(objv, 2, "LogMessage wrapper command requires at least a subcommand");
        std::string subcommand = objv[1];
        
        if(subcommand == "Log") {
            log(interp, objv);
        } else {
            throw std::invalid_argument("Invalid subcommand in logmessage wrapper");
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
    catch (...) {
        interp.setResult("Unexpectec C++ exception type caught");
        return TCL_ERROR;
    }
    
    return TCL_OK;
}
/**
 * TclLogMessage - log
 *    Create a log message.  The log message requires a severity code, see
 *    severityTable for the allowed values, and a string that is the log
 *    message.
 *
 *  @param interp - interpreter on which the command is running
 *  @param objv   - The command line words.
 */
void
CTCLLogMessage::TCLLogMessage::log(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    requireExactly(objv, 4, "Log requires a severity level and a message");
    std::string severityString(objv[2]);
    std::string messageText(objv[3]);
    
    // Conver the severity string to a severity level -- throw if the string
    // invalid:
    
    
    auto p = severityTable.find(severityString);
    if (p != severityTable.end()) {
        uint32_t severity = p->second;
        m_pObject->Log(severity, messageText);
    } else {
        throw std::invalid_argument("Invalid messages severity level");
    }
}