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
*   @file   CPreResumeCommand.cpp
*   @brief  Implementation of the preresume command.
*/
#include "CPreResumeCommand.h"
#include <TCLInterpreter.h>
#include <TCLObject.h>

#include <CRunState.h>
#include <CVMUSBHighLevelController.h>
#include <Globals.h>
#include <Exception.h>
#include <stdexcept>


/**
 * constructor
 *   @param interp Interpreter on which the command is going to be registered.
 */

CPreResumeCommand::CPreResumeCommand(CTCLInterpreter& interp) :
    CTCLObjectProcessor(interp, "preresume", true)
    {}

/**
 * destructor
 *    Just chain -> parent class.
 */
CPreResumeCommand::~CPreResumeCommand() {}

/**
 * operator()
 *    Invoked when the command is processed by the interpreter.  Note that
 *    -   There are no command parameters.
 *    -   The actual work is done in the perform() method so that it can be
 *        invoked by resume if necessary.
 *
 *  @param   interp - interpreter executing the command.
 *  @param   objv   - Command words.  Includes the command keyword.
 *  @return int     -  TCL_OK on success and TCL_ERROR on failure.
 *  @note If TCL_ERROR is returnedm the interpreter result will be a
 *        (hopefully) descripttive error message.
 */
int CPreResumeCommand::operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    bindAll(interp, objv);
    
    try {
        requireExactly(objv, 1, "preresume requires no parameters");
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
    return TCL_OK;
}
/**
 * perform
 *    Does the actual action.
 */
void CPreResumeCommand::perform()
{
    // Only do (nothing) if the state is paused:
    
    CRunState* pState = CRunState::getInstance();
    CRunState::RunState = pState->getState();
    
    if (RunState == "paused") {
        pState->setState("resuming");
    } else {
        throw std::logic_error("Pre resuming a non-paused run"):
    }
}