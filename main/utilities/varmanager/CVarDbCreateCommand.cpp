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
# @file   CVarDbCreateCommand.cpp
# @brief  Implement the create command for the variable database.
# @author <fox@nscl.msu.edu>
*/

#include "CVarDbCreateCommand.h"
#include <TCLInterpreter.h>
#include <TCLInterpreterObject.h>
#include <exception>
#include <tcl.h>

#include "CVariableDb.h"

/**
 * constructor
 *    Construct the command (registers it on the interpreter):
 *
 *  @param interp - reference to the interpreter.
 *  @param command - The name of the command (usually ::vardb::create)
 */

CVarDbCreateCommand::CVarDbCreateCommand(CTCLInterpreter& interp, const char* command) :
    CTCLObjectProcessor(interp, command, true)
{}
/**
 * destructor
 */
CVarDbCreateCommand::~CVarDbCreateCommand() {}

/**
 * operator()
 *    The command.
 *    - Ensure we have the right number of arguments.
 *    - invoke the CVariableDb::create static method.
 * @param interp - Reference to the interpreter.
 * @param objv   - vector of words that make up the command.
 *
 * @note we'll turn errors into exceptions to unify processing.
 */
int
CVarDbCreateCommand::operator()(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    bindAll(interp, objv);               // Associate an interp with objects.
    
    try {
        requireExactly(objv, 2);
        std::string name = objv[1];
        CVariableDb::create(name.c_str());
    }
    catch (std::string msg) {
        interp.setResult(msg);
        return TCL_ERROR;
    }
    catch (std::exception& e) {
        interp.setResult(e.what());
        return TCL_ERROR;
    }
    catch (...) {
        interp.setResult("Unanticipated exception type");
        return TCL_ERROR;
        
    }
    return TCL_OK;
}