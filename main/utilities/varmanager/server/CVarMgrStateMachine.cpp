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
# @file   CVarMgrStateMachine.cpp
# @brief  implement the var mgr API's statemachine command.
# @author <fox@nscl.msu.edu>
*/
#include "CVarMgrStateMachine.h"
#include "TCLInterpreter.h"
#include "TCLObject.h"
#include "CVarMgrOpenCommand.h"
#include "CVarMgrApi.h"
#include <tcl.h>

/**
 * constructor
 *
 *  @param interp  - interpreter on which the command will be registered.
 *  @param command - String that initiates command execution.
 */
CVarMgrStateMachine::CVarMgrStateMachine(
    CTCLInterpreter& interp, const char* command
) : CTCLObjectProcessor(interp, command, true) {}

/**
 * destructor
 */
CVarMgrStateMachine::~CVarMgrStateMachine() {}

/**
 * operator()
 *    Executes the statemachine command.  This uses the variable manager
 *    API to create a new statemachine type.
 *
 * @param interp - interpreter executing the command.
 * @param objv   - Vector of bound objects that are the command words.
 */
int
CVarMgrStateMachine::operator()(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    char usage[100];
    bindAll(interp, objv);
    sprintf(usage, "Usage\n   %s handle type-name transition-dict",
            std::string(objv[0]).c_str());
    
    try {
        requireExactly(objv, 4, usage);
        
        std::string handle   = objv[1];
        std::string typeName = objv[2];
        
        // Handle name to API object:
        
        CVarMgrApi* pApi = CVarMgrOpenCommand::handleToApi(handle.c_str());
        if (!pApi) {
            throw std::runtime_error("Invalid Handle");
        }
        
        // Marshall the dict -> the state map.
        
        CVarMgrApi::StateMap transitions;
        Tcl_Interp *pInterp = interp.getInterpreter();
        Tcl_Obj*    pDict   = objv[3].getObject();
        Tcl_Obj*    pKey;
        Tcl_Obj*    pValue;
        int         done(0);
        Tcl_DictSearch searchPtr;
        
        int status = Tcl_DictObjFirst(pInterp, pDict, &searchPtr, &pKey, &pValue, &done);
        if (status != TCL_OK) {
            throw std::runtime_error("State transition map must be a dict");
        }
        
        while (!done) {
            CTCLObject trList(pValue);
            trList.Bind(interp);
            for (int i =0; i < trList.llength(); i++) {
                pApi->addTransition(
                    transitions, Tcl_GetString(pKey), std::string(trList.lindex(i))
                );    
            }
            
            Tcl_DictObjNext(&searchPtr, &pKey, &pValue, &done);
            
        }
        
        // Map better not be empty:
        
        if (transitions.empty()) {
            throw std::runtime_error("Transition map cannot be empty");
        }
        
        // Make the machine
        
        pApi->defineStateMachine(typeName.c_str(), transitions);
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