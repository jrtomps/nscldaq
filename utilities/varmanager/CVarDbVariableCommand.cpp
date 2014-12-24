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
# @file   CVarDbVariableCommand.cpp
# @brief  Implementation of the vardb::variable command.
# @author <fox@nscl.msu.edu>
*/
#include "CVarDbVariableCommand.h"
#include <TCLInterpreter.h>
#include <TCLObject.h>
#include "CVariable.h"
#include "CVarDirTree.h"
#include "CVariableDb.h"

#include <exception>
#include <iostream>


/**
 * constructor
 *   @param interp - Reference to the interpreter on which we will register our
 *                   command.
 *   @param command - Base command name for the ensemble.
 */
CVarDbVariableCommand::CVarDbVariableCommand(
    CTCLInterpreter& interp, const char* pCommand
) : CTCLObjectProcessor(interp, pCommand, true)
{}

/**
 * operator()
 *    Called when the command is executed.
 *    - Bind the interpreter to all objv elements.
 *    - Ensure there's a subcommand and handle.
 *    - Get the handle state.
 *    - Dispatch to the subcommand.
 * @param interp     - Interpreter executing this command.
 * @param objv       - Words that make up the command.
 * @return int       - TCL_OK or TCL_ERROR depending on whether or not the
 *                     command succeeded.
 */
int
CVarDbVariableCommand::operator()(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    bindAll(interp, objv);
    
    try {
        requireAtLeast(objv, 3);         // Need subcommand and handle.
        
        // Pull out the subcommand and handle strings...then get the handle
        // information.
        
        std::string subcommand = objv[1];
        std::string handle     = objv[2];
        CVarDbOpenCommand::HandleState hstate     =
            CVarDbOpenCommand::translateHandle(handle);
        if (!hstate.s_db) {
            throw std::string("Invalid handle");
        }
        
        if (subcommand == "create") {
            create(interp, objv, hstate);
        } else if (subcommand == "destroy") {
            destroy(interp, objv, hstate);
        } else if (subcommand == "set") {
            set(interp, objv, hstate);
        } else if (subcommand == "get") {
            get(interp, objv, hstate);            
        } else if (subcommand == "ls") {
            ls(interp, objv, hstate);
        } else {
            throw std::string("Invalid subcommand");
        }
        
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

/**
 * create
 *   Handler for the create subcommand.
 *   - Require at least a path and a type.
 *   - Require at most a path a type and an initial value.
 *   - Marshall the parameters for a CVariable::create call to create the variable
 *   - Create the variable.
 *   - Destroy the object (variable is non-volatile).
 *
 * @param interp    - Interpreter running the command.
 * @param objv      - Command words.
 * @param hstate    - Handle state (supplies us the db and cwd objects).
 *
 * @throw std::string - If we detect errors.
 * @throw std::exception - if the variable database API detects and error.
 */
void
CVarDbVariableCommand::create(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv,
    CVarDbOpenCommand::HandleState& state
)
{
    requireAtLeast(objv, 5);
    requireAtMost(objv, 6);
    
    
    // Marshall the parameters to the create operation.
    
    std::string path = objv[3];
    std::string type = objv[4];
    std::string initialValue;
 
    
    const char* pValue(0);
    if (objv.size()==6) {
        initialValue = (std::string)(objv[5]);
        pValue = initialValue.c_str();
    }
    
    CVariable* pVar = CVariable::create(
        *state.s_db, *state.s_cd, path.c_str(), type.c_str(), pValue
    );
    delete pVar;
}
/**
 * destroy
 *    Destroy a variable
 *
 *   @param interp    - Interpreter running the command
 *   @param objv      - Command words.
 *   @param state     - Handle state
 *
 * @throw std::string - if we detect an error
 * @throw std::exception - if the API detects an error.
 */
void
CVarDbVariableCommand::destroy(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv,
    CVarDbOpenCommand::HandleState& state
)
{
    requireExactly(objv, 4);
    
    std::string path = objv[3];
    CVariable::destroy(*state.s_db, *state.s_cd, path.c_str());
}
/**
 * set
 *    Set a new value for a variable.
 *    -  Validate number of command words.
 *    -  Create a variable for the path.
 *    -  invoke it's set method
 *
 * @param interp   - Interpreter that is executing the command.
 * @param objv     - Command words.
 * @param state    - Handle state (has db and cwd).
 *
 * @throw std::string - Errors we detect.
 * @throw std::exception - errors the API detects.
 */
void
CVarDbVariableCommand::set(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv,
    CVarDbOpenCommand::HandleState& state
)
{
    requireExactly(objv, 5);
    std::string path = objv[3];
    std::string value= objv[4];
    
    CVariable var(*state.s_db, *state.s_cd, path.c_str());
    var.set(value.c_str());
}

/**
 * get
 *    Get the value of an existing variable.
 *    - Validate the command word count.
 *    - Construct a variable
 *    - Get the value.
 *    - Marshall the value into the interpreter result.
 * @param interp   - Interpreter that is executing the command.
 * @param objv     - Command words.
 * @param state    - Handle state (has db and cwd).
 *
 * @throw std::string - Errors we detect.
 * @throw std::exception - errors the API detects.
 */
void
CVarDbVariableCommand::get(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv,
    CVarDbOpenCommand::HandleState& state
)
{
    requireExactly(objv, 4);
    
    std::string path = objv[3];
    
    CVariable var(*state.s_db, *state.s_cd, path.c_str());
    
    interp.setResult(var.get());
}

/**
 * list
 *    List the variables in a directory.
 *    - If there's a path get it from the command line
 *    - Use the CVariable::list method to get the raw listing.
 *    - Marshall the results into a list of lists.  Each list element
 *      is just the VarInfo elements in the order in which they appear
 *      in the struct.
 * @param interp   - Interpreter that is executing the command.
 * @param objv     - Command words.
 * @param state    - Handle state (has db and cwd).
 *
 * @throw std::string - Errors we detect.
 * @throw std::exception - errors the API detects.
 */
void
CVarDbVariableCommand::ls(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv,
    CVarDbOpenCommand::HandleState& state
)
{
    requireAtMost(objv, 4);
    const char* path = 0;
    std::string sPath;
    if (objv.size() == 4) {
        sPath = std::string(objv[3]);
        path  = sPath.c_str();
    }
    
    std::vector<CVariable::VarInfo> info =
        CVariable::list(state.s_db, *state.s_cd, path);
        
    CTCLObject result;
    result.Bind(interp);
    for(int i =0; i < info.size(); i++) {
        CTCLObject element;
        element.Bind(interp);
        
        element += info[i].s_id;
        element += info[i].s_name;
        element += info[i].s_type;
        element += info[i].s_typeId;
        element += info[i].s_dirId;
        
        result += element;
    }
    
    
    interp.setResult(result);
}