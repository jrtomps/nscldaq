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
# @file   CVarDbGetwdCommand.cpp
# @brief  Implement the getwd operation.
# @author <fox@nscl.msu.edu>
*/

#include "CVarDbGetwdCommand.h"
#include <TCLInterpreter.h>
#include <TCLObject.h>
#include "CVarDbOpenCommand.h"
#include "CVarDirTree.h"

#include <exception>
#include <string>

/**
 * constructor
 *
 * @param interp - the interpreter the command is being registered on.
 * @param command - Command string that invokes the command (usually ::vardb::getwd).
 */
CVarDbGetwdCommand::CVarDbGetwdCommand(CTCLInterpreter& interp, const char* command) :
    CTCLObjectProcessor(interp, command, true)
{}

/**
 * destructor
 */
CVarDbGetwdCommand::~CVarDbGetwdCommand() {}

/**
 * operator()
 *    Actual command processor
 *    - Ensure there are exactly 2 command words.
 *    - Get the handle state (complain if there isn't one).
 *    - Get the current working directory from the cd part of the state.
 * @param interp - Interpreter running the command.
 * @param objv   - Vector of command words.
 * @return int   - TCL_OK on success, TCL_ERROR on failure
 */
int
CVarDbGetwdCommand::operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    bindAll(interp, objv);
    try {
        requireExactly(objv, 2);
        std:: string handle = objv[1];
        CVarDbOpenCommand::HandleState s = CVarDbOpenCommand::translateHandle(handle);
        if (!s.s_db) {
            throw std::string("Invalid or closed variable database handle");
        }
        interp.setResult(s.s_cd->wdPath());
    }
    catch (std::exception& e) {
        interp.setResult(e.what());
        return TCL_ERROR;
    }
    catch (std::string msg) {
        interp.setResult(msg);
        return TCL_ERROR;
    }
    catch (...) {
        interp.setResult("Unexpected exception type caught");
        return TCL_ERROR;
    }
    return TCL_OK;

}
