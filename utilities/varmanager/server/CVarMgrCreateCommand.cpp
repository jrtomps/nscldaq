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
# @file   CVarMgrCreateCommand.cpp
# @brief  Implementation of the variable manager create command.
# @author <fox@nscl.msu.edu>
*/
#include "CVarMgrCreateCommand.h"
#include <TCLInterpreter.h>
#include <TCLObject.h>
#include <CVariableDb.h>
#include <stdexcept>
#include <stdio.h>


/**
 * Construction
 *   @param interp - interpreter the command is registered on.
 *   @param command - command word that invokes the object.
 */
CVarMgrCreateCommand::CVarMgrCreateCommand(CTCLInterpreter& interp, const char* command) :
    CTCLObjectProcessor(interp, command, true)
{}

/**
 * destructor.
 */
CVarMgrCreateCommand::~CVarMgrCreateCommand()
{}

/**
 * operator()
 *   Invoked to execute the command.
 *   - Ensure there's exactly 2 parameters.
 *   - Extract the file parameter.
 *   - Create the databsae.
 *
 * @note exceptions are used to manage error reporting.
 *
 * @param interp - Interpreter that is executing the command.
 * @param objv   - Vector of command words.
 * @return int   - TCL_OK if all went well or TCL_ERROR If not.
 */
int
CVarMgrCreateCommand::operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    
    char usage[200];
    bindAll(interp, objv);
    
    sprintf(usage, "Usage\n  %s filename", std::string(objv[0]).c_str());
    
    try {
        requireExactly(objv, 2, usage);
        
        std::string filepath = objv[1];
        CVariableDb::create(filepath.c_str());
        
    }
    catch (std::string msg) {
        interp.setResult(msg);
        return TCL_ERROR;
    }
    catch (std::exception& e) {
        interp.setResult(e.what());
        return TCL_ERROR;
    }
    
    return TCL_OK;
}

