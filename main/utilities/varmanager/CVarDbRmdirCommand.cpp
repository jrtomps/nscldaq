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
# @file   CVarDbRmdirCommand.cpp
# @brief  Implementation of the vardb rmdir command for Tcl applications.
# @author <fox@nscl.msu.edu>
*/

#include "CVarDbRmdirCommand.h"
#include <TCLInterpreter.h>
#include <TCLObject.h>

#include "CVarDirTree.h"
#include "CVariableDb.h"
#include "CVarDbOpenCommand.h"

#include <string>
#include <exception>

/**
 * constructor
 *    Register the command.  Base class constructor takes care of the heavy
 *    lifting.
 * @param interp  - Interpreter on which the command will be registered.
 * @param command - Command word that will invoke the command.
 */
CVarDbRmdirCommand::CVarDbRmdirCommand(CTCLInterpreter& interp, const char* command) :
    CTCLObjectProcessor(interp, command, true)
{}

/**
 * destructor
 */
CVarDbRmdirCommand::~CVarDbRmdirCommand() {}


/**
 * operator()
 *   Dispatched to by the command.  We must:
 *   - Ensure that we have the right number of parameters (handle and path)
 *   - Translate the handle to get the dirtree object associated with it.
 *   - execute the appropriate rmdir operation.
 *
 *  @note we use exception processing to do error management.  All exceptions
 *        get turned into Tcl result strings and TCL_ERROR returns.
 *
 * @param interp - Reference to the interpreter running the command.
 * @param objv   - vector of Tcl objects that make up the command words
 *                (after substitutions naturally).
 * @return int TCL_OK on success and TCL_ERROR on failure.
 *
 * @note On success the command doesn't have a result.  On error the result
 *       is an error message from the caught exception.
 */
int
CVarDbRmdirCommand::operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    bindAll(interp, objv);
    
    try {
        requireExactly(objv, 3);
        std::string handle = objv[1];
        std::string path   = objv[2];
        
        CVarDbOpenCommand::HandleState state = CVarDbOpenCommand::translateHandle(handle);
        if (!state.s_db) {
            throw std::string("Invalid or closd variable database handle (rmdor)");
        }
        
        state.s_cd->rmdir(path.c_str());
        
    }
    catch (std::string msg) {
        interp.setResult(msg);
        return TCL_ERROR;
    }
    catch (std::exception& e) {
        interp.setResult(e.what());
        return TCL_ERROR;
    }
    catch(...) {
        interp.setResult("Unanticipated exception type caught (rmdir)");
        return TCL_ERROR;
    }
    
    return TCL_OK;
}
