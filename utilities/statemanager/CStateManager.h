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
# @file   CStateManager.h
# @brief  Manage state of programs and global state.
# @author <fox@nscl.msu.edu>
*/

#ifndef CSTATEMANAGER_H
#define CSTATEMANAGER_H

#include <utility>
#include <string>
#include <vector>
#include <map>

#include "CStateTransitionMonitor.h"
#include "CStateProgram.h"

class CVarMgrApi;


/**
 * @class CStateManager
 * 
 * This class pulls together the various bits and pieces that make up state
 * management via the variable database.  It is a bit of a corncob anti-pattern
 * for the convenience of its clients as it provides the following moderately
 * independent facilities:
 *
 *  * Program management      - definition, deletion, control over programs.
 *  * Global State control    - Provides the ability to modify the global state.
 *  * State transition monitoring - Provides the ability to ensure that
 *  *                           global state transitions trigger the correct
 *  *                           local state transitions and, where there are
 *  *                           '-ing' intermediate states, that the system
 *  *                           stabilizes to the final state within the
 *  *                           required timeout (e.g. Global NotReady -> Global Readying -> Individual
 *  *                           Readying -> Individual Ready -> Global Ready)
 *  * Stand alone state management - Drives the state of programs that have been
 *                              marked as standalone.
 *                              
 *  What this not in the scope of this class is program startup (boot) and exit
 *  (failure) detection.  That is normally done by the boot manager program
 *  which has its own set of additional support classes for that.
 */

class CStateManager
{
private:    
    // Object data:
    
    CStateTransitionMonitor*  m_pMonitor;
    std::string               m_reqURI;
    std::string               m_subURI;
    std::map<std::string, std::string> m_finalStates;
    CStateProgram*            m_pPrograms;
    
    // Public data types:
    
public:
    typedef CStateProgram::ProgramDefinition
        ProgramDefinition, *pProgramDefinition;
    
    typedef void (*TransitionCallback)(
        CStateManager& mgr, std::string program, std::string state, void* cd
    );
    typedef void (*BacklogCallback)(
        CStateManager& mgr, CStateTransitionMonitor::Notification Notification,
        void* cd
    );
public:
    CStateManager(const char* requestUri, const char* subscriptionUri);
    virtual ~CStateManager();
    
    // Managing programs:
public:
    std::string       getProgramParentDir();
    void              setProgramParentDir(const char* path);
    void              addProgram(const char* name, const pProgramDefinition def);
    ProgramDefinition getProgramDefinition(const char* name);
    void              modifyProgram(const char* name, const pProgramDefinition def);
    void              enableProgram(const char* name);
    void              disableProgram(const char* name);
    bool              isProgramEnabled(const char* name);
    void              setProgramStandalone(const char* name);
    void              setProgramNoStandalone(const char* name);
    bool              isProgramStandalone(const char* name);
    std::vector<std::string> listPrograms();
    std::vector<std::string> listEnabledPrograms();
    std::vector<std::string> listStandalonePrograms();
    std::vector<std::string> listInactivePrograms();
    std::vector<std::string> listActivePrograms();
    void               deleteProgram(const char* name);
    void               setEditorPosition(const char* name, int x, int y);
    int                getEditorXPosition(const char* name);
    int                getEditorYPosition(const char* name);
    CStateProgram*     getProgramApi() {return m_pPrograms;}
    
    
    // Global state control:
    
    void setGlobalState(const char* newState);
    std::string getGlobalState();
    std::vector<std::pair<std::string, std::string> > getParticipantStates();
    
    std::string title();
    void title(const char* newTitle);
    
    unsigned timeout();
    void timeout(unsigned newValue);
    
    bool recording();
    void recording(bool state);
    
    unsigned runNumber();
    void runNumber(unsigned newValue);
    
    
    
    // State transition monitoring
    
    void waitTransition(TransitionCallback cb = 0, void* clientData = 0);
    void processMessages(BacklogCallback cb = 0, void* clientData = 0);
    
    // Managing stand alone program state:
    
    bool isActive(const char* name);
    void setProgramState(const char* name, const char* state);
    std::string getProgramState(const char* name);
    
    // Utilities:
private:    
    std::string  getProgramDirectoryPath(const char* name);
    std::string  getVarpath(const char* program, const char* name);
    void         setProgramVar(
        const char* program, const char* var, const char* value
    );
    std::string getProgramVar(const char* program, const char* var);
    bool        getProgramBool(const char* program, const char* var);
    
    
};

#endif