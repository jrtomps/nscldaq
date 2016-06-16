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
# @file   CStateClientApi.cpp
# @brief  Implementation of the state client api
# @author <fox@nscl.msu.edu>
*/

#include "CStateClientApi.h"
#include "CStateTransitionMonitor.h"
#include <CVarMgrApi.h>
#include <URL.h>

#include <stdlib.h>
#include <unistd.h>

static const std::string DefaultSubscriptionService("vardb-changes");

/**
 * constructor
 *
 * @param reqURI      - URI for the request port, must be tcp.
 * @param subURI      - URI for the subscriptions, must be tcp.
 * @param programName - Name for which we are monitoring, must exist.
 * @note Binding to the program is a one-time affair, at constructino time.
 *       no attempt is going to be made to follow changes in 'ReadoutParentDir'
 *       The API, instead does a cd to the correct program directory.
 * @throw CStateClientApi::Exception in the event of errors.
 */
CStateClientApi::CStateClientApi(
    const char* reqURI, const char* subURI, const char* programName
) : CStateManager(reqURI, subURI),
    m_lastState("unknown"),
    m_programName(programName)
{
    cacheProgramDirectory();
    m_standalone = stringToBool(getProgramVar("standalone"));
    if (m_standalone) {
        m_lastState = getProgramVar("State");
    } else {
        m_lastState = getGlobalState();
    }
}

/**
 * destructor
 */
CStateClientApi::~CStateClientApi()
{
    
}
/**
 * setState
 *    Change the program's state.
 *
 *  @param newState - the new state to set.
 */
void
CStateClientApi::setState(std::string newState)
{
    std::string stateVar = getProgramVarPath("State");
    getApi()->set(stateVar.c_str(), newState.c_str());
    m_lastState = newState;
}

/**
 * inring
 *   @return std::string - Value of the input ring variable (global state).
 */
std::string
CStateClientApi::inring()
{
    
    return getProgramVar("inring");   // getProgramVar diddle mutex.
}

/**
 * outring
 *  @return std::string - value of the output ring
 */
std::string
CStateClientApi::outring()
{
    return getProgramVar("outring");
}

/**
 * isEnabled
 *    @return bool  - true if the program is enabled else false.
 */
bool
CStateClientApi::isEnabled()
{
    return (getProgramVar("enable") == "true") ? true : false;
}


/**
 * waitTransition
 *   Wait for a transition to be requested by a change in the global
 *   state variable.
 *
 * @param[out] newState - state after the transition completes.
 * @param timeout - # milliseconds to wait for timeout (-1 means forever).
 *
 * @return bool - false if no transition, true if there was one.
 */
bool
CStateClientApi::waitTransition(std::string& newState, int timeout)
{
    // First process messages without timeout (could be a pending change)
    
    TransitionInfo t(*this);
    processMessages(waitTransitionMessageHandler, &t, 0);
    // If there's no change pending then process with timeout.

    
    if (!t.s_transitioned) {
        processMessages(waitTransitionMessageHandler, &t, timeout);
    }
    // If there was a transiton, report it to the caller's newState
    // regardless, t.s_transitioned is the return value.
  
    if (t.s_transitioned) {
        newState = t.s_newState;
    }
    return t.s_transitioned;
    
}
/**
 *  Typically called from the waitTransitionMessageHandler if it detects
 *  standalone stae changed.
 */
void
CStateClientApi::updateStandalone(bool newValue)
{
    m_standalone = newValue;
}

/*---------------------------------------------------------------------------*/
/* Private utilities.                                                        */

/**
 * getProgramDirectory
 *    Return our program directory.
 * @return std::string
 */
void
CStateClientApi::cacheProgramDirectory()
{
    std::string parent = getApi()->get("/RunState/ReadoutParentDir");
    if (parent == "") parent = "/RunState";
    
    parent += "/";
    parent += m_programName;
    m_programDirectory =  parent;
}
std::string
CStateClientApi::getProgramDirectory()
{
    return m_programDirectory;
}
/**
 * getProgramVar
 *    Get a program variable.
 *
 *   @param varname - the variable name
 *   @return std::string - the value of the var.
 */
std::string
CStateClientApi::getProgramVar(const char* varname)
{
    std::string fullVarname = getProgramVarPath(varname);
    std::string value = getApi()->get(fullVarname.c_str());
    return  value;
}


/**
 * getProgramVarPath
 *   @param varname program variable name.
 *   @return std::string full path to that variable.
 */
std::string
CStateClientApi::getProgramVarPath(const char* varname)
{
    std::string fullVarname = getProgramDirectory();
    fullVarname += "/";
    fullVarname += varname;
    
    return fullVarname;
}
/**
 *  waitTransitionMessageHandler
 *     Called by processMessage (base class) for each
 *     notification message received while processing messages.
 *     Can update the standalone status of the program.
 *
 *  @param mgr - Reference to the state manager (our base class object).
 *  @param Notification - Notification message to process.
 *  @param cd  - Callback data, actually a pointer to a TransitionInfo struct.
 *  @note the TranstionInfo struct is filled in if there are state transtion
 *               requests.
 */
void
CStateClientApi::waitTransitionMessageHandler(
    CStateManager& mgr, CStateTransitionMonitor::Notification Notification,
        void* cd
)
{
    TransitionInfo *pT = reinterpret_cast<TransitionInfo*>(cd);
    CStateClientApi& o(pT->s_obj);
    
    // We're only intersted in:
    //   GlobalStateChange
    //   ProgramStateChange -- if we are in standalone and it's our program.
    //   VarChanged for our standalone variable.
    //    if in standalone mode we're interested in local state change requests
    //    as those drive us:
    //
    switch (Notification.s_type) {
        case CStateTransitionMonitor::GlobalStateChange:
            if (!o.m_standalone) {
                // Cast cd to a TransitionInfo struct and fill it in:
                
                
                pT->s_transitioned = true;
                pT->s_newState     = Notification.s_state;
            }
            break;
        case CStateTransitionMonitor::VarChanged:
            {
                // Figure out if this is our standalone variable:
                
                std::string path = o.getProgramVarPath("standalone");
                if (path == Notification.s_state) {
                    o.updateStandalone(stringToBool(Notification.s_program));
                }
            }
            break;
        case CStateTransitionMonitor::ProgramStateChange:
            // If standalone and our program this is a transition too:
        
            if (o.m_standalone && (Notification.s_program == o.m_programName)) {
                pT->s_transitioned = true;
                pT->s_newState     = Notification.s_state;
                o.m_lastState      = Notification.s_state;  // cache the state.
                
            }
            break;
        default:
            break;
    }
}
/**
 * stringToBool
 *    Transform a variable value string for a boolean variable into a bool
 *    value.
 *
 *    @param value - the value to convert.
 *    @return bool.
 *    @throw CStateClientApi::CException - the value is invalid.
 */
bool
CStateClientApi::stringToBool(std::string value)
{
    if (value == "true") {
        return true;
    } else if (value == "false")  {
        return false;
    } else {
        throw CException("Invalid boolean string.");
    }
}