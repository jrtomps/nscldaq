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
# @file   CPreEndCommand.cpp
# @brief  Implement the pre end command operation.
# @author <fox@nscl.msu.edu>
*/

#include "CPreEndCommand.h"
#include <TCLInterpreter.h>
#include <TCLObject.h>
#include "CCCUSBHighLevelController.h"
#include <Exception.h>
#include "Globals.h"
#include <stdexcept>
#include "CRunState.h"


/**
 * constructor
 *   @param interp - the interpreter on which the command is being registered.
 */
CPreEndCommand::CPreEndCommand(CTCLInterpreter& interp) :
    CTCLObjectProcessor(interp, "preend")
{}

/**
 * destructor
 */
CPreEndCommand::~CPreEndCommand() {}

/**
 * operator()
 *    Called when the command is exeucting.
 *
 *  @param interp - the interpreter that is executing the command.
 *  @param objv   - Words that make up the command (including the command word itself).
 *  @return int   - TCL_OK -on success, TCL_ERROR on failure.
 *  @note If the return value is TCL_ERROR, the interpreter result is a human
 *        readable error message.
 */
int
CPreEndCommand::operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    bindAll(interp, objv);
    
    try {
        requireExactly(objv, 1, "preend command does not have any parameters");
        
        // This can only be done when the state is Active or paused:
        
        CRunState* pState = CRunState::getInstance();
        CRunState::RunState state = pState->getState();
        
        if ((state == CRunState::Active) || (state == CRunState::Paused)) {
            perform();
        } else {
            throw std::logic_error(
                "preend - Invalid run state must be active or paused."
            );
        }
       
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
    catch (CException& e) {
        interp.setResult(e.ReasonText());
        return TCL_ERROR;
    }
    catch (...) {
        interp.setResult("preend - unexpected exception type caught");
        return TCL_ERROR;
    }
    
    return TCL_OK;
}
/**
 * perform
 *    Does the actual operations.  This is separated so that it can also
 *    be called from the end command...if the state is right for that.
 */
void
CPreEndCommand::perform()
{
    CCCUSBHighLevelController* pC = ::Globals::pController;
    pC->performStopOperations();
    
    // The state must now be ending.
    
    CRunState* pState = CRunState::getInstance();
    pState->setState(CRunState::Ending);
}