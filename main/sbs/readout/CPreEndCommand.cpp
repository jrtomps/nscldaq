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
# @brief  Perform all operations required prior to ending a run.
# @author <fox@nscl.msu.edu>
*/
#include "CPreEndCommand.h"
#include <TCLInterpreter.h>
#include <TCLObject.h>
#include <RunState.h>
#include <CRunControlPackage.h>
#include <Exception.h>
#include <stdexcept>

/**
 * construtor
 *   @param interp  - interpreter on which our command is registered.
 */
CPreEndCommand::CPreEndCommand(CTCLInterpreter& interp) :
    CTCLPackagedObjectProcessor(interp, "preend")
{
    
}

/**
 * destructor
 */
CPreEndCommand::~CPreEndCommand() {}

/**
 * operator()
 *    Perform the command.
 *
 * @param interp - interpreter executing the command.
 * @param objv   - The command words includig the command name.
 * @return int   - TCL_OK if the command succeeded, TCL_ERROR if not.
 *
 *  @note - In the event the command fails the interpreter result is set to a
 *          human readable error message.
 */
int
CPreEndCommand::operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    bindAll(interp, objv);
    
    try {
        requireExactly(objv, 1, "preend only accepts one parameter");
        CTCLObjectPackage*   pPack       = getPackage();
        CRunControlPackage&  rRunControl = reinterpret_cast<CRunControlPackage&>(*pPack);
        
        rRunControl.preEnd();
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
        interp.setResult("Unanticipated exception caught by 'preend' command");
        return TCL_ERROR;
    }
    
    return TCL_OK;
}