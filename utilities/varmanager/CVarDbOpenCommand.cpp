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
# @file   CVarDbOpenCommand.cpp
# @brief  Implement the vardb open command
# @author <fox@nscl.msu.edu>
*/

#include "CVarDbOpenCommand.h"

#include <TCLInterpreter.h>
#include <TCLObject.h>
#include "CVariableDb.h"
#include "CVarDirTree.h"
#include <stdio.h>
#include <exception>


// Static non-const storage:

std::map<std::string, CVarDbOpenCommand::HandleState> CVarDbOpenCommand::m_handleMap;
int                                 CVarDbOpenCommand::m_handleNo(0);

/*-----------------------------------------------------------------------------
 * static method implementations:
 */

/**
 * translateHandle
 *    Returns the CVariableDb* that corresponds to a handle string.
 *    If there is no match NULL is returned instead.
 *
 *  @param handle       - handle string.
 *  @return HandleState - Struct that has the state of the handle.  If
 *                        the handle does not exist, the database and current
 *                        directory state pointers are null.
 */
CVarDbOpenCommand::HandleState
CVarDbOpenCommand::translateHandle(std::string handle)
{
    std::map<std::string, HandleState>::iterator p = m_handleMap.find(handle);
    if (p == m_handleMap.end()) {
        HandleState null = {0, 0};
        return null;
    } else {
        return p->second;
    }
}
/**
 * close
 *   Close a handle by
 *   -  Destroying the associated state objects.
 *   -  Deleting the entry from the map.
 *
 *   It is up to the caller to validate the handle.  Closing a non-existent handle
 *   is a no-op.
 *
 *   @param handle - Handle string to close.
 *   
 */
 void
 CVarDbOpenCommand::close(std::string handle)
 {
    std::map<std::string, HandleState>::iterator p = m_handleMap.find(handle);
    if (p != m_handleMap.end()) {
        HandleState state = p->second;
        delete state.s_cd;
        delete state.s_db;
        m_handleMap.erase(p);
    }
 }
 
 /**
  * nextHandle
  *    Creates and returns a new unique handle string.
  *
  * @return std::string
  */
 std::string
 CVarDbOpenCommand::nextHandle()
 {
    char handleText[1000];
    sprintf(handleText, "vardb_%d", m_handleNo++);
    return std::string(handleText);
 }
 
 /*----------------------------------------------------------------------------
  * Object level method implementations.
  */
 
 /**
  * constructor
  *    @param interp - reference to the interpreter on which this object is being
  *                    registered.
  *   @param command - String used to invoke the command.
  */
 CVarDbOpenCommand::CVarDbOpenCommand(CTCLInterpreter& interp, const char* command) :
    CTCLObjectProcessor(interp, command, true)
{}

/**
 * destructor
 */
CVarDbOpenCommand::~CVarDbOpenCommand() {}

/**
 * operator()
 *   Invoked when the command is executed.
 *
 *  @param interp References the interpreter running the command.
 *  @param objv   command words.
 *
 *  @note we're using the exception -> return method.
 *
 *  @return int - TCL_OK on succes, TCL_ERROR on failure.
 */
int
CVarDbOpenCommand::operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    bindAll(interp, objv);
    
    try {
        requireExactly(objv, 2);        // Command + file path.
        std::string path = objv[1];
        CVariableDb* pDb = new CVariableDb(path.c_str());
        CVarDirTree* pCd = new CVarDirTree(*pDb);
        
        // If we get here we must succeed:
        
        std::string handle = nextHandle();
        HandleState h = {pDb, pCd};
 
        m_handleMap[handle] = h;
        interp.setResult(handle);
    }
    catch (std::exception& e) {
        interp.setResult(e.what());
        return TCL_ERROR;
    }
    catch(std::string msg) {
        interp.setResult(msg);
        return TCL_ERROR;
    }
    catch (...) {
        interp.setResult("Unexpected exception type caught");
        return TCL_ERROR;
    }
    return TCL_OK;
}
