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
# @file   CVarDbCloseCommand.cpp
# @brief  Implements the close command for the variable database tcl bindings.
# @author <fox@nscl.msu.edu>
*/

#include "CVarDbCloseCommand.h"
#include "CVarDbOpenCommand.h"
#include "CVariableDb.h"   
#include <TCLInterpreter.h>
#include <TCLObject.h>
#include <exception>
#include <string>


/**
 * constructor
 *    Create the command.
 *
 *   @param interp - reference to the interpreter on which we are registered.
 *   @param command - the string that will invoke operator() normally ::vardb::close
 */
CVarDbCloseCommand::CVarDbCloseCommand(CTCLInterpreter& interp, const char* command) :
    CTCLObjectProcessor(interp, command, true)
{}

/**
 * destructor
 */
CVarDbCloseCommand::~CVarDbCloseCommand() {}


/**
 * operator()
 *   Called when the command is invoked.
 *   - Ensure we have sufficient command parameter.
 *   - Ensure the handle is valid.
 *   - close the handle.
 *
 * @param interp - references the interpreter executing the command.
 * @param objv   - Words that make up the command.
 * @return int   - TCL_OK on succes, TCL_ERROR on failure.
 */
int
CVarDbCloseCommand::operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    bindAll(interp, objv);
    
    // error handling are exceptions that are turned in to results and TCL_ERROR returns:
    
    try {
        requireExactly(objv,2 );
        std::string handle = objv[1];
        CVarDbOpenCommand::HandleState state = CVarDbOpenCommand::translateHandle(handle);
        if (!state.s_db) {
            throw std::string("Invalid variable database handle");
        }
        CVarDbOpenCommand::close(handle);
    }
    catch(std::exception& e) {
        interp.setResult(e.what());
        return TCL_ERROR;
    }
    catch (std::string s) {
        interp.setResult(s);
        return TCL_ERROR;
    }
    return TCL_OK;
}