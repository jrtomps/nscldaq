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
# @file   CVarDbCdCommand.cpp
# @brief  Implements the change directory operation for the vardb Tcl package.
# @author <fox@nscl.msu.edu>
*/

#include "CVarDbCdCommand.h"
#include "CVarDbOpenCommand.h"

#include <TCLInterpreter.h>
#include <TCLObject.h>
#include <CVariableDb.h>
#include <CVarDirTree.h>

#include <exception>
#include <string>

/**
 * constructor
 *   Register the command on an interpreter to invoke our operator()
 *
 *  @param interp  - reference to the interpreter to which the command is registered.
 *  @param command - String to which the command is registered (usually ::vardb::cd).
 */
CVarDbCdCommand::CVarDbCdCommand(CTCLInterpreter& interp, const char* command) :
    CTCLObjectProcessor(interp, command, true)
{}

/**
 * destructor
 */
CVarDbCdCommand::~CVarDbCdCommand()
{}

/**
 * operator()
 *    Invoked when the registered command is used.
 *  @param interp - Interpreter running the command.
 *  @param objv   - Command words.
 *  @return int   - TCL_OK on success,TCL_ERROR on failure.
 *
 * TODO: Actually establish  a CVarDirTree that is associated with a database
 *       connection so that cd's are remembered/used in other operations.
 */
int
CVarDbCdCommand::operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    bindAll(interp, objv);
    try {
        requireExactly(objv, 3);               // Command, handle and directory.
        std::string handle = objv[1];
        std::string newdir = objv[2];
        
        CVarDbOpenCommand::HandleState state = CVarDbOpenCommand::translateHandle(handle);
        if (!state.s_db) {
            throw std::string("Invalid/closed database handle");
        }
        
        CVarDirTree& d(*state.s_cd);
        d.cd(newdir.c_str());
        interp.setResult(d.wdPath());       // Return the new working diretoctory path.

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
        interp.setResult("Unexpected exception caught");
        return TCL_ERROR;    
    }
    return TCL_OK;
}