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
# @brief  Perform the prebegin command.
# @author <fox@nscl.msu.edu>
*/
#include "CPreBeginCommand.h"
#include <TCLInterpreter.h>
#include <TCLObject.h>
#include "CRunControlPackage.h"
#include "RunState.h"
#include <StateException.h>
#include <Exception.h>
#include <string>
#include <string.h>
#include <stdlib.h>
#include <tcl.h>
#include <stdexcept>


/**
 * constructor
 *    Register the prebegin command with the interpreter.
 *
 *  @param interp -interpreter in which the command is registered.
 */
CPreBeginCommand::CPreBeginCommand(CTCLInterpreter& interp) :
    CTCLPackagedObjectProcessor(interp, std::string("prebegin"))
{
        
}

/**
 *   destructor.
 */
CPreBeginCommand::~CPreBeginCommand()
{
    
}

/**
 * operator()
 *     Performs the command:
 *     - Ensure we are in the halted state.
 *     - Ask the run control package to pre-begin the experiment.
 *     - Note that this transitions to the starting state.
 *
 *     All error returns run through an exception catch block.
 *
 * @param interp - the interpreter that is running the command
 * @param objv   - command line words.
 */
int
CPreBeginCommand::operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    bindAll(interp, objv);
    try {
        requireExactly(objv, 1, "'prebegin' command does not take any parameters");
        CTCLObjectPackage*   pPack       = getPackage();
        CRunControlPackage&  pRunControl = reinterpret_cast<CRunControlPackage&>(*pPack);
        
        pRunControl.preBegin();               // Experiment will update the state.

    }
    catch (std::exception& e) {
        interp.setResult(e.what());
        return TCL_ERROR;
    }
    catch (std::string msg) {
        interp.setResult(msg);
        return TCL_ERROR;
    }
    catch (const char* msg) {
        interp.setResult(msg);
        return TCL_ERROR;
    }
    catch (CStateException& e) {
        std::string msg = "Run was not in the proper state for 'prebegin': ";
        msg += e.ReasonText();
        interp.setResult(msg);
        return TCL_ERROR;
    }
    catch (CException& e) {
        std::string msg = "Run was not in the proper state for 'prebegin': ";
        msg += e.ReasonText();
        interp.setResult(msg);
        return TCL_ERROR;
    }
    catch (...) {
        interp.setResult("Unanticipated exception type caught be 'prebegin'");
        return TCL_ERROR;
    }
    
    return TCL_OK;
}

