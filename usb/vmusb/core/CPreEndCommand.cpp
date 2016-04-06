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
# @brief  Command that implements operations just prior to an end of run.
# @author <fox@nscl.msu.edu>
*/

#include "CPreEndCommand.h"
#include <TCLInterpreter.h>
#include <TCLObject.h>

#include "CRunState.h"
#include "CVMUSBHighLevelController.h"
#include <Exception.h>
#include <stdexcept>
#include <Globals.h>
/**
 * constructor
 *    @param interp  - the interpreter on which the command is being
 *                  registered.
 */
CPreEndCommand::CPreEndCommand(CTCLInterpreter& interp) :
    CTCLObjectProcessor(interp, "preend", true)
    {}
/**
 * destructor
 */
CPreEndCommand::~CPreEndCommand() {}

/**
 * operator()
 *    Executes the prebegin command itself.
 *  @param interp - the interpreter that's running the command.
 *  @param objv   - The command words (including the command itself).
 *  @return int   - TCL_OK - if the command succeeds, TCL_ERROR if not.
 *  @note   If the return value is TCL_ERROR, the interpreter restult is
 *          set to a human readable error message.
 */
int
CPreEndCommand::operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    bindAll(interp, objv);
    
    try {
        requireExactly(objv, 1, "preend command takes no parameters");
        
        // State must be active or paused:
        
        CRunState* pState = CRunState::getInstance();
        CRunState::RunState state = pState->getState();
        
        if ((state == CRunState::Active) || (state == CRunState::Paused)) {
            perform();
        } else {
            throw std::logic_error("preend requires the run to be either paused or active");
        }
    }
    catch (std::string& msg) {
        interp.setResult(msg);
        return TCL_ERROR;
    }
    catch(const char* msg) {
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
        interp.setResult("preend- unanticipated exception caught");
        return TCL_ERROR;
    }
    return TCL_OK;
}
/**
 * perform
 *    Performs the actual pre run actions.  In this way this operation can be invoked from
 *    end if preend has not yet been done.
 */
void
CPreEndCommand::perform()
{
    CVMUSBHighLevelController* p = Globals::pHLController;
    CRunState* pState = CRunState::getInstance();
    
    p->performStopOperations();
    
    pState->setState(CRunState::Stopping);
}