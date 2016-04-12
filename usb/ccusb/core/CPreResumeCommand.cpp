/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2005.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

/**
*   @file       CPreRsumeCommand.cpp
*   @brief      Implementation of the preresume command.
*/
#include "CPreResumeCommand.h"
#include <TCLInterpreter.h>
#include <TCLObject.h>
#include <Exception.h>
#include <stdexcept>
#include "CCCUSBHighLevelController.h"
#include "CRunState.h"
#include <Globals.h>


/**
 * constructor
 *    @param interp  - interpreter on which the command will be registered.
 */
CPreResumeCommand::CPreResumeCommand(CTCLInterpreter& interp) :
    CTCLObjectProcessor(interp, "preresume", true)
    {}
    
/**
 * destructor
 */
CPreResumeCommand::~CPreResumeCommand() {}

/**
 * operator()
 *    Ensures the command is properly formatted and then invokes perform() which
 *    does the real work.  perform is separated so that CResumeRun can also
 *    invoke it the resume command is issued without a prior preresume.
 *
 *  @param interp   - interpreter that's executing the command.
 *  @param objv     - The list of command words (including command keyword).
 *  @return int j   - TCL_OK if successful or TCL_ERROR on failure.
 *  @note if TCL_ERROR is returned the interpreter result is set with a human
 *        readable error message string.
 */
int
CPreResumeCommand::operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    bindAll(interp, objv);
    
    try {
        requireExactly(objv, 1, "preresume does not take any command parameters");
        perform();
    }
    catch (std::string msg) {
        interp.setResult(msg);
        return TCL_ERROR;
    }
    catch (const char* msg) {
        interp.setResult(msg);
        return TCL_ERROR;        
    }
    catch (CException& e) {
        interp.setResult(e.ReasonText());
        return TCL_ERROR;
    }
    catch (std::exception& e) {
        interp.setResult(e.what());
        return TCL_ERROR;        
    }
    catch (...) {
        interp.setResult("prebegin - unexpected exception type");
        return TCL_ERROR;
    }
    return TCL_ERROR;
}
/**
 * perform
 *    - Ensure the state is correct.
 *    - Perform the pre-resume operations.
 *    - set the state to resuming
 *
 * @note - at this time there are no pre-resume actions.
 *         we don't want to do the Start operations as these might include
 *         a clock synchronization that we don't want to do oni a resume.
 *     TODO:   Add, if there's interest, a hook for resume operations.
 */
void
CPreResumeCommand::perform()
{
    // Note that at this time it's legal to do anything to the
    // CCUSB/controller as the system is out of DAQ mode and the acquisition
    // thread has relinquished control over the CCUSB.
    
    CRunState* pState = CRunState::getInstance();
    if (pState->getState() == CRunState::Paused) {
        pState->setState(CRunState::Resuming);
    } else {
        throw std::logic_error("Attempted to pre-resume a run that was not paused");
    }
}
