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
# @file   CStateManagerInstanceCommand.h
# @brief  Represents an instance of a state manager as a command ensemble.
# @author <fox@nscl.msu.edu>
*/
#ifndef CSTATEMANAGERINSTANCECOMMAND_H
#define CSTATEMANAGERINSTANCECOMMAND_H
#include <TCLObjectProcessor.h>
#include <CStateTransitionMonitor.h>


class CTCLInterpreter;
class CTCLObject;
class CStateManager;
class CStateProgram;

#include <map>

/**
 * @class CStateManagerInstanceCommand
 *    Represents a command ensemble that provides the same
 *    functionality as a CStateManager object.  The mapping
 *    of ensemble subcommands is pretty much 1:1 with the public
 *    methods of CStateManager.  The internal document:
 *
 *    https://swdev-redmine.nscl.msu.edu/projects/sfnscldaq/wiki/State_Controller_API#Tcl-Bindings
 *
 *   provides a description of this ensemble.
 */
class CTCLStateManagerInstanceCommand : public CTCLObjectProcessor
{
private:
    typedef struct _CallbackInfo {
        CTCLInterpreter*   s_pInterp;
        std::string        s_scriptBase;
    } CallbackInfo, *pCallbackInfo;
private:
    CStateManager* m_pApi;
    CStateProgram* m_pPrograms;
    
    // canonicals
public:
    CTCLStateManagerInstanceCommand(
        CTCLInterpreter& interp, std::string name,
        std::string requrl, std::string suburl
    );
    CTCLStateManagerInstanceCommand(
        CTCLInterpreter& interp, std::string name,
        std::string dburl
    );
    virtual ~CTCLStateManagerInstanceCommand();
    
public:
    int operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    
    // Methods that provide subcommands in the ensemble.
protected:
    void programParentDir(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void addProgram(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void getProgram(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void modifyProgram(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void enableProgram(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void disableProgram(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void isProgramEnabled(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void setStandalone(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void setNoStandalone(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void isStandalone(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void listPrograms(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void listEnabledPrograms(
        CTCLInterpreter& interp, std::vector<CTCLObject>& objv
    );
    void listStandalonePrograms(
        CTCLInterpreter& interp, std::vector<CTCLObject>& objv
    );
    void listInactivePrograms(
        CTCLInterpreter& interp, std::vector<CTCLObject>& objv
    );
    void listActivePrograms(
        CTCLInterpreter& interp, std::vector<CTCLObject>& objv
    );
    void deleteProgram(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void setGlobalState(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void getGlobalState(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void getParticipantStates(
        CTCLInterpreter& interp, std::vector<CTCLObject>& objv
    );
    void title(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void timeout(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void recording(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void runNumber(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void waitTransition(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void processMessages(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void isActive(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void setProgramState(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void getProgramState(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    
    // Utility methods.
private:
    std::map<std::string, std::string> dictToMap(
        CTCLInterpreter& interp, CTCLObject& dict
    );
    void vectorToResult(CTCLInterpreter& interp, std::vector<std::string> vec);
    static void DictPutString(
        Tcl_Interp* pInterp, Tcl_Obj* pDict, std::string key, std::string value
    );
    static void dispatchTransitionScript(
        CStateManager& manager, std::string program, std::string state,
        void* cd
    );
    static void dispatchMessageScript(
        CStateManager& mgr, CStateTransitionMonitor::Notification msg,
        void* cd
    );
};


#endif