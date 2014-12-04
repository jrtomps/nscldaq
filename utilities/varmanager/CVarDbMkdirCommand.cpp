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
# @file   CVarDbMkdirCommand.cpp
# @brief  Implementation of the vardb mkdir command.
# @author <fox@nscl.msu.edu>
*/

#include "CVarDbMkdirCommand.h"
#include <TCLInterpreter.h>
#include <TCLObject.h>
#include "CVarDbOpenCommand.h"
#include "CVariableDb.h"
#include "CVarDirTree.h"
#include <exception>
#include <string>


/**
 * constructor
 *
 * @param interp - refers to the interpreter on which this command is registered.
 * @param command - Command string.  Usually ::vardb::mkdir
 */
CVarDbMkdirCommand::CVarDbMkdirCommand(CTCLInterpreter& interp, const char* command) :
    CTCLObjectProcessor(interp, command, true)
{}

/**
 * destructor
 */
CVarDbMkdirCommand::~CVarDbMkdirCommand()
{}

/**
 * operator()
 *   The command processor
 *   - Need either 3 or 4 words in the command.
 *   - If 4 words word 1 must be -nopath
 *   - db handle must validate.
 *   - Try to make the directory using a CVarDirTree object.
 * @param interp - Refers to the interpreter that is executing the command.
 * @param objv   - Vector of command words.
 * @return int   - TCL_OK on success, TCL_ERROR on failure.
 *
 * @note we use exception handling to deal with error management.
 */
int
CVarDbMkdirCommand::operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    bindAll(interp, objv);
    try {
        requireAtLeast(objv, 3);
        requireAtMost(objv, 4);
        
        bool    createPath = true;           // If -nopath missing.
        unsigned i         = 1;              // Where the db handle is in the line
        if (objv.size() == 4) {
            std::string sw = objv[1];
            i++;
            if (sw == "-nopath") {
                createPath = false;
            } else {
                throw std::string("Wrong number of parameters or -nopath misspelled");
            }
        }
        // i is the index of the database handle.
        
        std::string dbHandle = objv[i];
        std::string path     = objv[i+1];
        
        CVarDbOpenCommand::HandleState state = CVarDbOpenCommand::translateHandle(dbHandle);
        if (!state.s_db) {
            throw std::string("Invalid database handle");
        }
        CVarDirTree& tree(*state.s_cd);
        tree.mkdir(path.c_str(), createPath);
        
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
        interp.setResult("Unanticipated exception caught");
        return TCL_ERROR;
    }
    
    
    return TCL_OK;
}