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
# @file   CStateManager.cpp
# @brief  Manage state of programs and global state. (implementation)
# @author <fox@nscl.msu.edu>
*/


#include "CStateManager.h"
#include "CStateTransitionMonitor.h"
#include <CVarMgrApi.h>
#include <stdio.h>
#include <stdexcept>
#include <set>
#include <stdlib.h>


/**
 * constructor
 *   @param requestURI  - URI of the REQ port of the server.
 *   @param subscriptionURI - URI of the PUB/SUB port of the server.
 */
CStateManager::CStateManager(const char* requestURI, const char* subscriptionURI) :
    m_pMonitor(0),
    m_reqURI(requestURI),
    m_subURI(subscriptionURI)
{
    m_pMonitor = new CStateTransitionMonitor(m_reqURI.c_str(), m_subURI.c_str());
    
    // For each new global state, m_finalStates provides the
    // final program and global states expected.
    // For now we don't worry about intermediate states from the programs:
    // This presupposes that the final state is reachable from the in-transition
    // state.  Since the final state is at most separated from the initial
    // state by an 'ing' state this is the case.
    
    m_finalStates["0Initial"] = "0Initial";
    m_finalStates["NotReady"] = "NotReady";
    m_finalStates["Readying"] = "Ready";
    m_finalStates["Beginning"] = "Active";
    m_finalStates["Pausing"]  = "Paused";
    m_finalStates["Resuming"] = "Active";
    m_finalStates["Ending"]   = "Ready";
    
}

/**
 * destructor
 */
CStateManager::~CStateManager()
{
    delete m_pMonitor;
}
/*-----------------------------------------------------------------------------
 * Program management
 */

/**
 * getProgramParentDir
 *    Return the directory that is the parent to program directories:
 * @return std::string
 */
std::string
CStateManager::getProgramParentDir()
{
    return m_pMonitor->programParentDir();
}
/**
 * setProgramParentDir
 *    Sets a new program parent dir.  Note that since the state monitor freezes
 *    the parent dir we need to kill/recreate it.
 *
 * @param path - new parent directory.
 * @throw std::runtime_error derived exception if the path does not exist.
 */
void
CStateManager::setProgramParentDir(const char* path)
{
    // Get the API, ensure the dir exists and set the ReadoutParentPath to it
    // if so.
    CVarMgrApi* pApi = m_pMonitor->getApi();
    std::string wd = pApi->getwd();
    try {
        pApi->cd(path);    
    } catch (...) {
        pApi->cd(wd.c_str());
        throw;
    
    }
    pApi->set("/RunState/ReadoutParentDir", path);
    
    delete m_pMonitor;
    m_pMonitor = new CStateTransitionMonitor(m_reqURI.c_str(), m_subURI.c_str());
}
/**
 * addProgram
 *   Add a new program to the system.
 *
 *  @param name - new program name (must be unique).
 *  @param def  - Pointer to the program's definition.
 */
void
CStateManager::addProgram(const char* name, const pProgramDefinition def)
{
    std::string directory = getProgramDirectoryPath(name);

    CVarMgrApi* pApi = m_pMonitor->getApi();
    std::string wd = pApi->getwd();
    try {
        // Make the program directory and cd to it:
        
        pApi->mkdir(directory.c_str());
        pApi->cd(directory.c_str());
        
        // Stock with the contents according to def:
        
        pApi->declare("State", "RunStateMachine");  // Default -- 0Initial.
        pApi->declare("path", "string", def->s_path.c_str());
        pApi->declare("enable", "boolean", def->s_enabled ? "true" : "false");
        pApi->declare("standalone", "boolean", def->s_standalone ? "true" : "false");
        pApi->declare("host", "string", def->s_host.c_str());
        pApi->declare("outring", "string", def->s_outRing.c_str());
        pApi->declare("inring", "string", def->s_inRing.c_str());
        
    }
    catch(...) {
        pApi->cd(wd.c_str());    // Be sure we are back to normal.
        throw;
    }
    pApi->cd(wd.c_str());
}
/**
 * getProgramDefinition
 *    Return a program definition struct for the specified program.
 *    Note that if the program does not exist, an exception is thrown
 *
 *  @param name - program name.
 *  @return CStateManager::ProgramDefinition
 *  @throw std::runtime_error
 */
CStateManager::ProgramDefinition
CStateManager::getProgramDefinition(const char* name)
{
    std::string directory = getProgramDirectoryPath(name);
    
    CVarMgrApi* pApi = m_pMonitor->getApi();
    std::string wd = pApi->getwd();
    ProgramDefinition result;
    
    try {
        pApi->cd(directory.c_str());             // Also tests for exists.
        result.s_enabled    = pApi->get("enable") == "true" ? true : false;
        result.s_standalone = pApi->get("standalone") == "true" ? true : false;
        result.s_path       = pApi->get("path");
        result.s_host       = pApi->get("host");
        result.s_outRing    = pApi->get("outring");
        result.s_inRing     = pApi->get("inring");
    }
    catch (...) {
        pApi->cd(wd.c_str());
        throw;
    }
    
    pApi->cd(wd.c_str());
    
    return result;
}
/**
 *  modifyProgramDefinition
 *     Modify the definition of a program.
 *
 *  @param name  - Name of the program.
 *  @param def   - New definition.
 *  @throw std::runtime_error - if there are problems.
 */
void
CStateManager::modifyProgram(const char* name, const pProgramDefinition def)
{
    CVarMgrApi* pApi = m_pMonitor->getApi();
    std::string directory = getProgramDirectoryPath(name);
    
    std::string wd = pApi->getwd();
    try {
        pApi->cd(directory.c_str());
        
        pApi->set("path",  def->s_path.c_str());
        pApi->set("enable",  def->s_enabled ? "true" : "false");
        pApi->set("standalone",  def->s_standalone ? "true" : "false");
        pApi->set("host",  def->s_host.c_str());
        pApi->set("outring",  def->s_outRing.c_str());
        pApi->set("inring",  def->s_inRing.c_str());
    }
    catch(...) {
        pApi->cd(wd.c_str());
        throw;
    }
    pApi->cd(wd.c_str());
}
/**
 * enableProgram
 *    Enable a program.  Enabling a program that is already enabled is a no-op.
 *
 * @param name - program name.
 */
void
CStateManager::enableProgram(const char* name)
{
    setProgramVar(name, "enable", "true");
}
/**
 * disableProgram
 *   Disable a program.
 *
 * @param name  - the program name
 */
void
CStateManager::disableProgram(const char* name)
{
    setProgramVar(name, "enable", "false");

}
/**
 * setProgramStandalone
 *    Puts a program into standalone mode.
 *
 * @param name - name of the program.
 */
void
CStateManager::setProgramStandalone(const char* name)
{
    setProgramVar(name, "standalone", "true");
}
/**
 * setProgramNoStandalone
 *    Takes a program out of standalone mode.
 *
 *    @param name - name of the program.
 */
void
CStateManager::setProgramNoStandalone(const char* name)
{
    setProgramVar(name, "standalone", "false");
}
/**
 * isProgramEnabled:
 *     True if the specified program is enabled:
 *
 *  @param name - name of the program.
 *  @return bool
 */
bool
CStateManager::isProgramEnabled(const char* name)
{
    return getProgramBool(name, "enable");
}
/**
 * isProgramStandalone
 *
 *   @param name - program name.
 *   @return bool - true if the named program is set standalone.
 */
bool
CStateManager::isProgramStandalone(const char* name)
{
    return getProgramBool(name, "standalone");
}
/**
 * listPrograms
 *    Returns a list of all programs.
 * @return std::vector<string>
 */
std::vector<std::string>
CStateManager::listPrograms()
{
    std::vector<std::string> result;
    
    // We are just going to return the names of the sub-directories
    // in the program directory.  The claim is that users shouldn not
    // put additional stuff there.  If this claim is false then we'll
    // need to check for the variables that make each subdir a program.
    
    std::string dir = getProgramParentDir();
    CVarMgrApi* pApi = m_pMonitor->getApi();
    result           = pApi->ls(dir.c_str());
    
    return result;
}
/**
 * listEnabledPrograms
 *    List programs for which enabled is set.
 * @return std::vector<std::string> names of enabled programs (possibly empty)
 */
std::vector<std::string>
CStateManager::listEnabledPrograms()
{
    std::vector<std::string> result;
    std::vector<std::string> all  = listPrograms();
    
    // Filter that down to the set that are enabled:
    
    for (int i = 0; i < all.size(); i++) {
        if (getProgramBool(all[i].c_str(), "enable")) {
            result.push_back(all[i]);
        }
    }
    
    return result;
}
/**
 * listStandalonePrograms
 *   Return only those programs that have the standalone flag set:
 *
 *  @return std::vector<std::string>
 */
std::vector<std::string>
CStateManager::listStandalonePrograms()
{
    std::vector<std::string> result;
    std::vector<std::string> all  = listPrograms();
    
    // Filter that down to the set that are enabled:
    
    for (int i = 0; i < all.size(); i++) {
        if (getProgramBool(all[i].c_str(), "standalone")) {
            result.push_back(all[i]);
        }
    }
    
    return result;
}
/**
 * listInactivePrograms
 *   Return only those programs that are inactive.
 *   A program is inactive if either it is disabled or standalone.
 *
 * @return std::vector<std::string>
 */
std::vector<std::string>
CStateManager::listInactivePrograms()
{
    std::vector<std::string> result;
    std::vector<std::string> all  = listPrograms();
    
    // Filter that down to the set that are enabled:
    
    for (int i = 0; i < all.size(); i++) {
        if (getProgramBool(all[i].c_str(), "standalone") ||
            (!getProgramBool(all[i].c_str(), "enable"))) {
            result.push_back(all[i]);
        }
    }
    
    return result;
}
/**
 * listActivePrograms
 *    Lists programs that are enabled and not standalone:
 *
 * @return std::vector<std::string>
 */
std::vector<std::string>
CStateManager::listActivePrograms()
{
    std::vector<std::string> result;
    std::vector<std::string> all  = listPrograms();
    
    // Filter that down to the set that are enabled:
    
    for (int i = 0; i < all.size(); i++) {
        if ((!getProgramBool(all[i].c_str(), "standalone")) &&
            (getProgramBool(all[i].c_str(), "enable"))) {
            result.push_back(all[i]);
        }
    }
    
    return result;
}
/**
 * deleteProgram
 *    Delete a specified program.
 * @param name - name of the program to delete.
 */
void
CStateManager::deleteProgram(const char* name)
{
    // Get the name of the directory to delete:
    
    std::string progDir = getProgramDirectoryPath(name);
    
    // Delete all the variables in that directory:
    
    CVarMgrApi* pApi                      = m_pMonitor->getApi();
    std::vector<CVarMgrApi::VarInfo> vars =
        pApi->lsvar(progDir.c_str());
    
    // Step into that directory and delete them:
    
    std::string wd = pApi->getwd();
    try {
        pApi->cd(progDir.c_str());
        for (int i = 0; i < vars.size(); i++) {
            pApi->rmvar(vars[i].s_name.c_str());
        }
    }
    catch(...) {
        pApi->cd(wd.c_str());
        throw;
    }
    pApi->cd(wd.c_str());
    
    // Delete the directory too:
    
    pApi->rmdir(progDir.c_str());
}
/**
 * getParticipantStates
 *    Get the states of programs that should be participating
 *    in state transitions (active programs).
 *
 * @return std::vector<std::string, std::string> > -
 *           .first is the name of a program, .second, the
 *           state.
 */
std::vector<std::pair<std::string, std::string> >
CStateManager::getParticipantStates()
{
    std::vector<std::string> progs = listActivePrograms();
    std::vector<std::pair<std::string, std::string> > result;
    for (int i = 0; i < progs.size(); i++) {
        result.push_back(std::pair<std::string, std::string>(
            progs[i], getProgramVar(progs[i].c_str(), "State")
        ));
    }
    return result;
}

/**
 * title()
 *   @return std::string - the title.
 */
std::string
CStateManager::title()
{
    CVarMgrApi* pApi = m_pMonitor->getApi();
    return pApi->get("/RunState/Title");
}
/**
 * title(const char*)
 *   @param title - new title.
 */
void
CStateManager::title(const char* title)
{
    CVarMgrApi* pApi = m_pMonitor->getApi();
    pApi->set("/RunState/Title", title);
}

/**
 * timeout()
 *  @return unsigned int - current timeout value in seconds
 */
unsigned
CStateManager::timeout()
{
    CVarMgrApi* pApi = m_pMonitor->getApi();
    std::string timeoutString = pApi->get("/RunState/Timeout");
    char* endPtr;
    unsigned long timeout = strtoul(timeoutString.c_str(), &endPtr, 0);
    if (endPtr == timeoutString.c_str()) {
        throw std::runtime_error("Invalid timeout value in database");
    }
    return timeout;
    
}
/**
 * timeout(unsigned)
 *    Set a new timeout value.
 *
 *  @param newTo - new timeout value.
 */
void
CStateManager::timeout(unsigned newValue)
{
    CVarMgrApi* pApi = m_pMonitor->getApi();
    char timeoutString[100];
    sprintf(timeoutString, "%u", newValue);
    pApi->set("/RunState/Timeout", timeoutString);
}

/**
 * recording()
 *    @return bool -recording state
 */
bool
CStateManager::recording()
{
    CVarMgrApi* pApi = m_pMonitor->getApi();
    return pApi->get("/RunState/Recording") == "true";
}

/**
 * recording(bool)
 *
 *  @param state - new state for the recording flag.
 */
void
CStateManager::recording(bool state)
{
    CVarMgrApi* pApi = m_pMonitor->getApi();
    pApi->set("/RunState/Recording", state ? "true" : "false");
}
/**
 * runNumber()
 *  @return unsigned  the current run number.
 */
unsigned
CStateManager::runNumber()
{
    CVarMgrApi* pApi = m_pMonitor->getApi();
    std::string runNumstr = pApi->get("/RunState/RunNumber");
    return strtoul(runNumstr.c_str(), NULL, 0);
}
/**
 * runNumber(unsigned)
 *   @param newValue - New run number to set.
 */
void
CStateManager::runNumber(unsigned newValue)
{
    CVarMgrApi* pApi = m_pMonitor->getApi();
    char runStr[100];
    sprintf(runStr, "%d", newValue);
    
    pApi->set("/RunState/RunNumber", runStr);
}
/*------------------------------------------------------
 * State management/listing
 */

/**
 * setGlobalState
 *    Set a new global state value:
 *
 *  @param newState - new state value
 *  @throw - Must be a valid next state given the current state.
 */
void
CStateManager::setGlobalState(const char* newState)
{
    CVarMgrApi* pApi = m_pMonitor->getApi();
    pApi->set("/RunState/State", newState);
}
/**
 * getGlobalState
 *
 * @return std::string - current global state value
 */
std::string
CStateManager::getGlobalState()
{
    CVarMgrApi* pApi = m_pMonitor->getApi();
    return pApi->get("/RunState/State");
}
/*----------------------------------------------------------------
 * State transition monitoring - ensuring all programs maintain
 * state relations:
 */

/**
 * waitTransition
 *  Given the current global state processes monitor messages
 *  until the timeout occurs or the proper final state in all the
 *  participating programs is reached.
 *
 *  At each state transition processed, the optional user callout
 *  is invoked (this allows application level updates).
 *
 *  @note If all programs are already at the global state,
 *        this is a no-op.
 *  @note if there is a timeout prior to reaching the correct
 *        final state in all programs std::runtime_error
 *        is thrown.
 *
 *  @param cb         - Pointer to callback function, 0 if not used.
 *  @param clientData - Data passed to the callback.
*/

void
CStateManager::waitTransition(TransitionCallback cb, void* clientData)
{
    
    std::string gblState  = getGlobalState();
    std::string nextState = m_finalStates[gblState];   // State we expect next:
    std::vector<std::string> programs = listActivePrograms();
    
    std::set<std::string> stillWaiting;
    for (int i = 0; i < programs.size(); i++) {
        stillWaiting.insert(programs[i]);
    }

    
    // Process state transitions until timeout or the conditions are
    // met.
    
    CVarMgrApi* pApi = m_pMonitor->getApi();
    std::string timeoutString = pApi->get("/RunState/Timeout");
    int timeout;
    sscanf(timeoutString.c_str(), "%d", &timeout);   // Seconds.
    timeout *= 1000;                        // Milliseconds.
    
    std::vector<CStateTransitionMonitor::Notification> notifications;
    notifications = m_pMonitor->getNotifications(-1, 0); // Clear backlog.
   
    do {
        // Process messages:
        
         for (int i = 0; i < notifications.size(); i++) {
            if (
                (notifications[i].s_type ==
                    CStateTransitionMonitor::ProgramStateChange)
            ) {
                // Invoke the callout:
                if (cb) {
                    (*cb)(
                        *this, notifications[i].s_program,
                        notifications[i].s_state, clientData
                    );
                }
                // Remove appropriate programs for the waiting set:
                
                if (notifications[i].s_state == nextState) {
                    stillWaiting.erase(notifications[i].s_program);
                }
            }
        }
        // if stilWaiting is empty the state transition completed:
        
        if(stillWaiting.empty()) {
            if (gblState != nextState) {
                setGlobalState(nextState.c_str());
            }
            return;
        }
 
        // Get the next notifications -- with timeout
        
        notifications.clear();
        notifications = m_pMonitor->getNotifications(-1, timeout);
        
    } while (notifications.size() > 0);  // Timeout if no notifs.
    throw std::runtime_error("State transition timeout");
}
/**
 * processMessages
 *    Drains the publication message backlog invoking a user callback
 *    for each message present (if supplied).
 *
 * @param cb - Callback to invoke.
 * @param cd - Client data passed without interpretation to the callback.
 *
 * @note if cb is null then the message queue is just drained and
 *       any internal processing needed is performd without notifying
 *       the caller.
 */
void
CStateManager::processMessages(BacklogCallback cb, void* cd)
{
    std::vector<CStateTransitionMonitor::Notification> nots =
        m_pMonitor->getNotifications(-1, 0);  // Get all backlogged messages.
        
    for (int i = 0; i < nots.size(); i++) {
        if (cb) {
            (*cb)(*this, nots[i], cd);
        }
        // Local processing goes here or above the if above.
    }
}
/*----------------------------------------------------------------------
 * Individual programs...active/standalone:
 */

/**
 * isActive
 * 
 * @param name -name of a program.
 * @return bool true if the program is enabled and not standalone.
 * 
 */
bool
CStateManager::isActive(const char* name)
{
    bool enabled = getProgramBool(name, "enable");
    bool standalone = getProgramBool(name, "standalone");
    
    return (enabled && (!standalone));
}
/**
 * setProgramState
 *   Set the state of a single program.  Note that this really should
 *   only be done with standalone or disabled programs.  That policy
 *   enforcement is not done here, however to allow a client to
 *   take a program that's exited leaving its state in a bad uh...state
 *   to be able to reset it without changing the active-ness of the program.
 *
 * @param name - name of the program.
 * @param state - Desired state
 */
void
CStateManager::setProgramState(const char* name, const char* state)
{
    setProgramVar(name, "State", state);
}
/**
 * getProgramState
 *
 *   @param name - name of a program.
 *   @return std::string - that program's current state.
 */
std::string
CStateManager::getProgramState(const char* name)
{
    return getProgramVar(name, "State");    
}

/*----------------------------------------------------------------------
 * Private utilities
 */

/**
 * getProgramDirectory
 *
 *  @param name - name of a program.
 *  @return std::string - directory in whih that program lives.
 *  @note - no determination is done to ensure the program/dir exists.
 */
std::string
CStateManager::getProgramDirectoryPath(const char* name)
{
    std::string directory = getProgramParentDir();
    directory           += "/";
    directory           += name;
    
    return directory;
}
/**
 * getVarPath
 *    Returns the path to a variable in a program.
 *
 *  @param program - the program in which to return the path.
 *  @param name    - Name of the value.
 *  @return std::string - full path to the variable.
 */
std::string
CStateManager::getVarpath(const char* program, const char* name)
{
    std::string path = getProgramDirectoryPath(program);
    path += "/";
    path += name;
    
    return path;
}
/**
 * setProgramVar
 *    set a new value for a program variable.
 *
 *  @param program - name of the program.
 *  @param var     - name of the variable.
 *  @param value   - New value for the variable.
 */
void
CStateManager::setProgramVar(
    const char* program, const char* var, const char* value
)
{
    std::string varPath = getVarpath(program, var);
    CVarMgrApi* pApi    = m_pMonitor->getApi();
    
    pApi->set(varPath.c_str(), value);
}

/**
 * getProgramVar
 *    Returns the value of a program variable:
 *
 *  @param program - name of the program.
 *  @param var     - Name of the variable.
 *  @return std::string - value of the variable.
 */
std::string
CStateManager::getProgramVar(const char* program, const char* var)
{
    std::string path = getVarpath(program, var);
    CVarMgrApi* pApi = m_pMonitor->getApi();
    return pApi->get(path.c_str());
}
/**
 * getProgramBool
 *    Returns the value of a program boolean variable:
 *  @param program - name of the program.
 *  @param var     - Name of the variable.
 *  @return bool   - Value.
 */
bool
CStateManager::getProgramBool(const char* program, const char* var)
{
    std::string value = getProgramVar(program, var);
    
    if (value == "true") return true;
    if (value == "false") return false;
    
    // Not a bool value:
    
    
    std::string errorMessage("Variable: ");
    errorMessage += var;
    errorMessage += " for program ";
    errorMessage += program;
    errorMessage += " does not have a boolean value: ";
    errorMessage += value;
    throw std::runtime_error(errorMessage);
}

void* gpTCLApplication(0);