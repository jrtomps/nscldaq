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
#include "CRunControlPackage.h"
#include "RunState.h"
#include <stdexcept>
#include <Exception.h>

/**
 *  constructor
 *     @param interp - interpreter on which the command is going to be registered.
 */
CPrePauseCommand::CPrePauseCommand(CTCLInterpreter& interp) :
   CTCLPackagedObjectProcessor(interp, "prepause")
   {}
   
/**
 * destructor
 */
CPrePauseCommand::~CPrePauseCommand()
{}

/**
 * operator()
 *    executes the command.
 *  @param interp - the interpreter executing the command.
 *  @param objv   - The command words (including the command itself).
 *  @return int - TCL_OK on success and TCL_ERROR on failure.
 *  @note when TCL_ERROR is returned, the interpreter result is a human readable
 *         error message.
 */
int
CPrePauseCommand::operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    bindAll(interp, objv);
    try {
        requireExactly(objv, 1, "prepause takes no command parameters");
        perform();
    }
    catch(std::string msg) {
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
    
    return TCL_OK;
    
}
/**
 * perform
 *    Does the actual work
 *    -   Get the package
 *    -   invoke its prepause operation
 *    -   Set the state to pausing.
 *
 *   @throw CStateException (by way of the package's prepause) to indicate that
 *          the run is not in a valid state to issue prepause.
 */
void
CPrePauseCommand::perform()
{
   
    
    CTCLObjectPackage* pRawPackage = getPackage();
    CRunControlPackage& rPackage   = reinterpret_cast<CRunControlPackage&>(*pRawPackage);
    
    rPackage.prePause();
    
}