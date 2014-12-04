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
# @file   CVarDbLsCommand.cpp
# @brief  Implement ls functionality.
# @author <fox@nscl.msu.edu>
*/

#include "CVarDbLsCommand.h"
#include <TCLInterpreter.h>
#include <TCLInterpreterObject.h>

#include "CVarDbOpenCommand.h"
#include "CVarDirTree.h"

#include <exception>
#include <string>

/**
 * constructor
 *    Construction registers the object to respond to a specific command in a
 *    specific interpreter.
 *
 *   @param interp  - The interpreter the object/command is being registered on.
 *   @param command - The command string the object responds to.
 */
CVarDbLsCommand::CVarDbLsCommand(CTCLInterpreter& interp, const char* command) :
    CTCLObjectProcessor(interp, command, true)
{}

/**
 * destructor
 */
CVarDbLsCommand::~CVarDbLsCommand() {}


/**
 * operator()
 *   Responds to the command:
 *   -   There must be at most 3 parameters.
 *   -   The handle must be valid.
 *   -   If there is no path parameter, the cwd is listed.  Otherwise, the
 *       directory described in that path parameter is listed.
 *  @param interp - interpreter executing the command.
 *  @param objv   - Words that make up the command.
 *  @return int   - TCL_OK on correct operation, TCL_ERROR otherwise.
 */
int
CVarDbLsCommand::operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    bindAll(interp, objv);
    try {
        requireAtLeast(objv, 2);
        requireAtMost(objv, 3);
        
        std::string handle = objv[1];
        CVarDbOpenCommand::HandleState state = CVarDbOpenCommand::translateHandle(handle);
        if (!state.s_db) {
            throw std::string("Invalid or closed database file handle");
        }
        CVarDirTree& dir(*state.s_cd);
        
        std::vector<CVarDirTree::DirInfo> dirs;
        
        if (objv.size() == 2) {
            dirs = dir.ls();
        } else {
            std::string path = objv[2];
            std::string cd   = dir.wdPath();
            
            dir.cd(path.c_str());         // This might throw but then cd is not changed
            dirs = dir.ls();
            dir.cd(cd.c_str());           // Should not throw as we were just there.
            
        }
        // Marshall the listing into a Tcl list:
        
        CTCLObject result;
        result.Bind(interp);
        
        for (int i =0; i < dirs.size(); i++) {
            result += dirs[i].s_name;
        }
        interp.setResult(result);
        
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
