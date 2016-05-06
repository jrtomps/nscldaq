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
# @file   CPrePauseCommand.cpp
# @brief  Implement the pre-pause command.
# @author <fox@nscl.msu.edu>
*/

#include "CPrePauseCommand.h"
#include <TCLInterpreter.h>
#include <TCLObject.h>

#include "CCCUSBHighLevelController.h"
#include <stdexcept>
#include <Exception.h>
#include "CRunState.h"
#include "Globals.h"

/**
 * constructor
 *    @param interp - interpreter on which the command is being implemented.
 */
CPrePauseCommand::CPrePauseCommand(CTCLInterpreter& interp) :
    CTCLObjectProcessor(interp, "prepause")
    {}
/**
 * destructor
 */
CPrePauseCommand::~CPrePauseCommand() {}

/**
 * operator()
 *    Executes the command.
 *
 *  @param interp - the interpreter executing the command.
 *  @param objv   - The command words (inluding the command keyword).
 *  @return int   - TCL_OK for success, TCL_ERROR for failure.
 *  @note If TCL_ERROR is returned, the interpreter result is an error message.
 */
int
CPrePauseCommand::operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    bindAll(interp, objv);
    try {
        requireExactly(objv, 1, "prebegin takes no command parameters");
        
        // Require that the state be active only.

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
        interp.setResult("prepause - unexpected exception type thrown");
        return TCL_ERROR;
    }
    return TCL_OK;
}
/**
 * perform
 *   Perform the state checks, do the action and set the state to Pausing:
 */
void
CPrePauseCommand::perform()
{
    // State must be active coming in:
    
    CRunState* pR = CRunState::getInstance();
    CRunState::RunState state = pR->getState();
    
    if (state == CRunState::Active) {
        
    } else {
        // wrong state:
        
        throw std::logic_error("Unable to prepause a run because it's not 'Active'.");
    }
    
    ::Globals::pController->performStopOperations();
    
    pR->setState(CRunState::Pausing);
}
