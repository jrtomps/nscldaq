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
 * @file CPreBeginCommand.cpp
 * @brief prebegin command implementation.
 */

#include "CPreBeginCommand.h"
#include <TCLObjectProcessor.h>
#include <TCLObject.h>
#include <Exception.h>

#include "CConfiguration.h"
#include <Globals.h>
#include "CVMUSBHighLevelController.h"
#include "CRunState.h"

#include <stdexcept>

/**
 * constructor
 *   @param interp - interpreter the command will be registered under.
 */
CPreBeginCommand::CPreBeginCommand(CTCLInterpreter& interp) :
    CTCLObjectProcessor(interp, "prebegin", true) {}
    
/**
 * destructor
 */
CPreBeginCommand::~CPreBeginCommand() {}

/**
 *  operator() - executes the actual command.
 *
 *  @param interp - interpreter executing the command.
 *  @param objv   - command words.
 *  @return int   - TCL_OK - command succeeded.  TCL_ERROR - command failed
 */
int
CPreBeginCommand::operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    bindAll(interp, objv);
    
    try {
       requireExactly(1, objv, "prebegin does not allow any parameters");
       perform();
    }
    catch (const char* msg) {
        interp.setResult(msg);
        return TCL_ERROR;
    }
    catch (std::string msg) {
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
    
    
}
/**
 * perform
 *     Perform the operations.  This is factored from the command processing so
 *     begin can call it if the state is idle rather than starting.
 */
void
CPreBeginCommand::perform()
{
    CRunState* pState = CRunState::getInstance();
    // Ensure the run state is idle:
       
    if (pState->getState() != CRunState::idle) {
     throw std::logic_error("prebegin - run state is incorrect must be 'idle'.");
    }
    
    // Read the configuration -- check stacksize
       
    Globals::pHLController->readConfiguration(Globals::configurationFilename);
    if(!checkStackSize()) {
     throw std::system_error("prebegin - the stack sizes requested by the configuration won't fit in the VMUSB");
    }
    // Initialize the controller and the hardware.
    
    Globals::pHLController->flushBuffers();
    Globals::pHLController->initializeController();
    Globals::pHLController->initializeModules();
    
    // Load and enable the stacks:
    //
    Globals::pHLController->loadStacks();
    Globals::pHLController->enableStacks();
    
    // Perform run start operations:
    
    Globals::pHLController->performStartOperations();

    pState->setState(CRunState::Starting);
    
}