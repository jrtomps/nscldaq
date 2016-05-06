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
# @brief  Implement the prepause command.
# @author <fox@nscl.msu.edu>
*/
#include "CPrePauseCommand.h"
#include <TCLInterpreter.h>
#include <TCLObject.h>
#include "CRunState.h"
#include <Exception.h>
#include <stdexcept>
#include "CVMUSBHighLevelController.h"
#include <Globals.h>


/**
 *  constructor
 *
 *  @param interp - Interpreter on which the command will be registered.
 */
CPrePauseCommand::CPrePauseCommand(CTCLInterpreter& interp) :
    CTCLObjectProcessor(interp, "prepause", true)
{}

/**
 * destructor
 */
CPrePauseCommand::~CPrePauseCommand() {}

/**
 * operator()
 *    Execute the actual command.  If the state is Active we can prepause the
 *    run.  The actual actions are factored out into perform so that begin
 *    can leverage them as well (leverage - fancy word meaning invoke).
 *
 *  @param interp - The interpreter executing the command.
 *  @param objv   - The command words -- including the command keywords too.
 *  @return int - TCL_OK - success.  TCL_ERROR - failed.
 *  @note If we return TCL_ERROR, the interpreter result is set to an error msg.
 */
int
CPrePauseCommand::operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    bindAll(interp, objv);
    try {
        requireExactly(objv, 1, "prepause takes no command parameters");
        
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
    catch (...) {
        interp.setResult("prepause - unexpected exception type");
        return TCL_ERROR;
    }
    return TCL_OK;
    
}
/**
 * perform
 *    Perform the prepause actions.   This is pulled out so that begin can also
 *    use this code.
 */
void
CPrePauseCommand::perform()
{
    // require that we be active:
    
    CRunState* pS = CRunState::getInstance();
    CRunState::RunState state = pS->getState();
    
    if (state != CRunState::Active) {
        throw std::logic_error("prepause requires the state to be active");
    } else {
        ::Globals::pHLController->performStopOperations();
        
        pS->setState(CRunState::Pausing);
    }
}


