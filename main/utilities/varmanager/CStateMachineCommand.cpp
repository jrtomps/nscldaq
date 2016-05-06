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
# @file   CStateMachineCommand.cpp
# @brief  implement the statemachine command.
# @author <fox@nscl.msu.edu>
*/

#include "CStateMachineCommand.h"
#include "CStateMachine.h"
#include "CVarDbOpenCommand.h"

#include <TCLInterpreter.h>
#include <TCLInterpreterObject.h>
#include <tcl.h>

#include <exception>


// Local unbound functions.

/**
 * makeMap
 *   Local unbound function to create the transition map from the
 *   dict.  If there is an error, a string exceptionis thrown
 *   This is unbound so that we don't have to include <CStateMachine.h>
 *   in our header.
 */
static CStateMachine::TransitionMap
makeMap(CTCLInterpreter& interp, Tcl_Obj* pMapDict)
{
    Tcl_Interp*                  pInterp = interp.getInterpreter();
    CStateMachine::TransitionMap result;
    Tcl_DictSearch               searchContext;
    Tcl_Obj*                     key;
    Tcl_Obj*                     value;
    int                          done;
    
    if (Tcl_DictObjFirst(
        interp, pMapDict, &searchContext, &key, &value, &done) != TCL_OK
    ) {
        throw std::string("Parameter is not a dictionary");
    }
    
    while (!done) {
        int nChar;
        const char* pKey   = Tcl_GetStringFromObj(key, &nChar);
 
        CTCLObject  valueObj(value);            // List of to states
        valueObj.Bind(interp);
        for (int i = 0; i < valueObj.llength(); i++) {
            CTCLObject toState = valueObj.lindex(i);
            toState.Bind(interp);
            CStateMachine::addTransition(
                result, std::string(pKey), std::string(toState)
            );    
        }
        
        
        Tcl_DictObjNext(&searchContext, &key, &value, &done); 
    }
    
    return result;
}
/**
 * construction.
 *
 * @param interp - reference to the interpreter on which this command is being
 *                 registered.
 * @param pName  - Name of the command.
 */
CStateMachineCommand::CStateMachineCommand(
    CTCLInterpreter& interp, const char* pName
) : CTCLObjectProcessor(interp, pName, true)
{}

/**
 * destrution
 */
CStateMachineCommand::~CStateMachineCommand() {}


/**
 * operator()
 *    Command handler.
 *
 * @param interp - interpreter running the command.
 * @param objv   - command words.
 * @return int   - TCL_OK for success, TCL_ERROR for failure.
 */
int
CStateMachineCommand::operator()(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    bindAll(interp, objv);
    try  {
        requireExactly(objv, 4);

        // Marshall the parameters.
        
        std::string handle   = objv[1];
        std::string typeName = objv[2];
        Tcl_Obj*    pStateMap= objv[3].getObject();
        
        // First parameter must be a handle string.
        
        CVarDbOpenCommand::HandleState hstate =
            CVarDbOpenCommand::translateHandle(handle);
        if (!hstate.s_db) {
            throw std::string("Invalid database handle");
        }
        
        // Create the transition map from the last parameter.
        
        CStateMachine::TransitionMap transitions = makeMap(interp, pStateMap);
        
        // Create the state machine:
        
        CStateMachine::create(*(hstate.s_db), typeName, transitions);
        
    }
    catch (std::string msg) {
        interp.setResult(msg);
        return TCL_ERROR;
    }
    catch (const char* msg) {
        interp.setResult(msg);
        return TCL_ERROR;
    }
    catch (std::exception& e) {
        interp.setResult(e.what());
        return TCL_ERROR;
    }
    catch (...) {
        interp.setResult("Unanticipated exception type caught");
        return TCL_ERROR;
    }
    
    return TCL_OK;
}