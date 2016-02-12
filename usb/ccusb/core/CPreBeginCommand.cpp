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
# @file   CPreBeginCommand.cpp
# @brief  Implements the prebegin command.  
# @author <fox@nscl.msu.edu>
*/

#include "CPreBeginCommand.h"
#include "TCLInterpreter.h"
#include "TCLObject.h"
#include "CCCUSBHighLevelController.h"
#include "Globals.h"
#include <stdexcept>
#include <CRunState.h>


/**
 * constructor
 *    Registers 'prebegin' as a Tcl command.
 *
 *  @param intrerp - interpreter in which the command is getting registered.
 */
CPreBeginCommand::CPreBeginCommand(CTCLInterpreter& interp) :
    CTCLObjectProcessor(interp, "prebegin", true)
{}

/**
 * destructor
 */
CPreBeginCommand::~CPreBeginCommand() {}

/**
 * operator()
 *    Gains control when the prebegin command is encountered by the interpreter.
 *    -   Ensures there are no command line parameters.
 *    -    Validates the state transition.
 *    -    Performs the prebegin initialization.
 *    -    performs the state transition from Idle -> Starting
 *
 * @param interp - interpreter executing the command.
 * @param objv   - Objects containing the command line words.
 * @return int   - TCL_OK - command succeeded, TCL_ERROR - command failed.
 *                 on failure the interpreter result contains an error message.
 */
int
CPreBeginCommand::operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    bindAll(interp, objv);
    try {
        requireExactly(objv, 1, "prebegin does not have parameters");
        
        // State must be idle:
        
        CRunState* pStateMachine = CRunState::getInstance();
        if (pStateMachine->getState() != CRunState::Idle) {
            throw std::logic_error("prebegin - state must be idle but is not.");
        }
        
        performPreBeginInitialization();
        
        
        pStateMachine->setState(CRunState::Starting);
        
    }
    catch (std::exception& e) {
        interp.setResult(e.what());
        return TCL_ERROR;
    }
    catch (std::string s) {
        interp.setResult(s);
        return TCL_ERROR;
    }
    catch (const char* s) {
        interp.setResult(s);
        return TCL_ERROR;
    }
    catch (...) {
        interp.setResult("Unexpected exception type in prebegin command");
        return TCL_ERROR;
    }
    
    return TCL_OK;
}
/**
 * performPreBeginIntialization
 *   -  read the configuration.
 *   - perform CCUSB initialization.
 *   -  perform DAQdevice initialization.
 *   - load stacks.
 */
void
CPreBeginCommand::performPreBeginInitialization()
{
    Globals::pController->readConfiguration(
        Globals::configurationFilename.c_str()
    );
    // Ensure the stack size is ok:
    
    if (!Globals::pController->checkStackSize()) {
        throw std::string("Stacks won't fit in stack memory.");
    }
    
    Globals::pController->initializeController();
    Globals::pController->initializeModules();    
    Globals::pController->loadStacks();
    ::Globals::pController->performStartOperations();
}