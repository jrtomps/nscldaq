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
# @file   CVarMgrSubCommand.cpp
# @brief  Implement the command object for subscriptions.
# @author <fox@nscl.msu.edu>
*/

#include "CVarMgrSubCommand.h"
#include <TCLInterpreter.h>
#include <TCLObject.h>
#include "CVarMgrSubscriptions.h"
#include <stdexcept>
#include <stdio.h>


/**
 * constructor
 *
 * @param[in] interp  - interpreter on which the command is being registered.
 * @param[in] command - String that invokes the command.
 * @param[in] subs    - References a CVarMgrSubscriptions type connected to the
 *                      publisher.
 */
CVarMgrSubCommand::CVarMgrSubCommand(
    CTCLInterpreter& interp, const char* command, CVarMgrSubscriptions& subs
)  : CTCLObjectProcessor(interp, command, true),
    m_subscriptions(subs)
{}

/**
 * destructor:
 */
CVarMgrSubCommand::~CVarMgrSubCommand()
{
    Tcl_DeleteFileHandler(m_subscriptions.fd());         // Cancel any callbacks.
    delete &m_subscriptions;
}

/**
 * operator()
 *    Called when the command is activated.
 *    - Ensure we have at least a subcommand.
 *    - Dispatch the subcommand.
 * @param[in]  interp    - Reference to the interpreter executing this.
 * @param[in]  objv      - vector of command words.
 * @return int Tcl_OK on success.
 */
int
CVarMgrSubCommand::operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    bindAll(interp, objv);
    try {
        char usage[200];
        sprintf(usage, "Usage:\n %s subcommand", std::string(objv[0]).c_str());
        requireAtLeast(objv, 2, usage);
        
        std::string subcommand = objv[1];
        if (subcommand == "destroy") {
            destroy(interp, objv);
        } else if (subcommand == "subscribe") {
            subscribe(interp, objv);
        } else if (subcommand == "read") {
            read(interp, objv);
        } else if (subcommand == "wait") {
            wait(interp, objv);
        } else if (subcommand == "unsubscribe") {
            unsubscribe(interp, objv);
        } else if (subcommand == "notify") {
            notify(interp, objv);
        } else {
            throw std::runtime_error("Invalid subcommand");
        }
    }
    catch (std::string msg) {
        interp.setResult(msg);
        return TCL_ERROR;
    }
    catch (std::exception& e) {
        interp.setResult(e.what());
        return TCL_ERROR;
    }
    
    return TCL_OK;
}

/*--------------------------------------------------------------------------
 * subcommand processors:
 */

/**
 * destroy
 *   Self destruct.
   * @param interp - Reference to encapsulated Tcl interpreter object
   * @param objv   - Reference to the vector of encapsulated Tcl_Obj's that make up the
   *                 command word.
   *
   */
void
CVarMgrSubCommand::destroy(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    char usage[200];
    sprintf(usage, "Usage:\n  %s destroy", std::string(objv[0]).c_str());
    requireExactly(objv, 2, usage);
    
    delete this;
}
/**
 * subscribe
 *    Add a subscription
 *
 * @param interp - Reference to encapsulated Tcl interpreter object
 * @param objv   - Reference to the vector of encapsulated Tcl_Obj's that make up the
 *                 command word.  We need a parameter for the path we are subscribing
 *                 to.
*/
void 
CVarMgrSubCommand::subscribe(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    char usage[200];
    sprintf(usage, "Usage:\n %s subscribe path", std::string(objv[0]).c_str());
    
    requireExactly(objv, 3, usage);
    
    std::string path = objv[2];
    m_subscriptions.subscribe(path.c_str());
  
}
/**
 * read
 *     Reads a notification (blocks until one arrives) and returns
 *     a dict containing the result.s
 *
 * @param interp - Reference to encapsulated Tcl interpreter object
 * @param objv   - Reference to the vector of encapsulated Tcl_Obj's that make up the
*/
void
 CVarMgrSubCommand::read(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
 {
    char usage[200];
    sprintf(usage, "Usage:\n  %s read", std::string(objv[0]).c_str());
    
    requireExactly(objv, 2, usage);
    
    CVarMgrSubscriptions::Message msg = m_subscriptions.read();
    Tcl_Obj* result = Tcl_NewDictObj();
    
    Tcl_Obj* key    = Tcl_NewStringObj("path", -1);
    Tcl_Obj* value  = Tcl_NewStringObj(msg.s_path.c_str(), -1);
    Tcl_DictObjPut(interp.getInterpreter(), result, key, value);
    
    key   = Tcl_NewStringObj("op", -1);
    value = Tcl_NewStringObj(msg.s_operation.c_str(), -1);
    Tcl_DictObjPut(interp.getInterpreter(), result, key, value);
    
    key   = Tcl_NewStringObj("data", -1);
    value = Tcl_NewStringObj(msg.s_data.c_str(), -1);
    Tcl_DictObjPut(interp.getInterpreter(), result, key, value);
    
    interp.setResult(result);
 }
 /**
  * wait
  *    Waits for a message with an optional timeout.
  *    
  * @param interp - Reference to encapsulated Tcl interpreter object
  * @param objv   - Reference to the vector of encapsulated Tcl_Obj's that make up the
  *                 command word.
  */
void
CVarMgrSubCommand::wait(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    // Timeout is optional.. defaults to -1.
    
    int timeout(-1);
    char usage[200];
    sprintf(usage, "Usage:\n  %s wait ?timeout?", std::string(objv[0]).c_str());
    
    requireAtLeast(objv, 2, usage);
    requireAtMost(objv, 3, usage);
    
    if (objv.size() == 3) {
        timeout = objv[2];
    }
    
    bool haveData = m_subscriptions.waitmsg(timeout);
    CTCLObject result; result.Bind(interp);
    
    result = haveData ? 1 : 0;
    interp.setResult(result);
 
}
/**
 * unsubscribe
 *   Remove a subscription
 *
 * @param interp - Reference to encapsulated Tcl interpreter object
 * @param objv   - Reference to the vector of encapsulated Tcl_Obj's that make up the
 *                 command word.
 *
*/
void
CVarMgrSubCommand::unsubscribe(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    char usage[200];
    sprintf(usage, "Usage\n  %s unsubscribe path", std::string(objv[0]).c_str());
    
    requireExactly(objv, 3, usage);
    
    std::string path = objv[2];
    
    m_subscriptions.unsubscribe(path.c_str());
 
}

/** notify
 *     Establishes a script to be called when there's published data.
 *
 * @param interp - Reference to encapsulated Tcl interpreter object
 * @param objv   - Reference to the vector of encapsulated Tcl_Obj's that make up the
 *                 command word.  The script parameter is handled as follows:
 *                 -   If an empty string the file handlers are cancelled.
 *                 -   If a nonempty string the file handler is established on the current string.
*/
void
CVarMgrSubCommand::notify(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    requireExactly(objv, 3);
    
    std::string script = objv[2];
    
    // Cancel:
    
    if (script == "" && (m_script != "")) {
        Tcl_DeleteFileHandler(m_subscriptions.fd());
    } else {
        if (m_script == "") {
            Tcl_CreateFileHandler(m_subscriptions.fd(), TCL_READABLE, readable, this);
        }
    }
    m_script = script;
}
 
 /**
  * readable
  *    Called when the file descriptor is readable and there's a non-empty
  *    handler script
  * @param cd - Actually a pointer to the object (this is a static member).
  * @param mask - Mask of events.
  * @note it is possible for us to get called when the socket is not actually
  *       readable (according to the 0mq docs), therefore we also check the
  *       actual readability before dispatching.
  */
 void
 CVarMgrSubCommand::readable(ClientData cd, int mask)
 {
    CVarMgrSubCommand* pThis = reinterpret_cast<CVarMgrSubCommand*>(cd);
    
    if(pThis->m_subscriptions.readable()) {
        CTCLInterpreter* pInterp = pThis->getInterpreter();
        pInterp->GlobalEval(pThis->m_script);
    }
 }
 