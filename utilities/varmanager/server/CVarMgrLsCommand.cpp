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
# @file   CVarMgrLsCommand.cpp
# @brief  Implement the ls command.
# @author <fox@nscl.msu.edu>
*/
#include "CVarMgrLsCommand.h"
#include "CVarMgrOpenCommand.h"
#include "CVarMgrApi.h"
#include <TCLInterpreter.h>
#include <TCLObjectProcessor.h>


/**
 * constructor
 *
 *  @param interp    - Interpreter on which the command is registered.
 *  @param pCommand  - Command string that activates the command.
 */
CVarMgrLsCommand::CVarMgrLsCommand(CTCLInterpreter& interp, const char* pCommand) :
    CTCLObjectProcessor(interp, pCommand, true)
{}

/**
 * destructor
 */
CVarMgrLsCommand::~CVarMgrLsCommand() {}

/**
 * operator()
 *
 * @param interp  - the interpreter executing the command.
 * @param objv    - Vector of objects that make up the command words.
 * @return int - TCL_OK for  success, TCL_ERROR for failure.
 */
int
CVarMgrLsCommand::operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    char usage[100];
    bindAll(interp, objv);
    
    sprintf(usage, "Usage\n   %s handle ?path?", std::string(objv[0]).c_str());
    
    try {
        requireAtLeast(objv, 2, usage);
        requireAtMost(objv, 3, usage);
        
        std::string handle = objv[1];
        std::string path   = "";
        
        if (objv.size() == 3) {
             path = std::string(objv[2]);
        }
        
        CVarMgrApi* pApi = CVarMgrOpenCommand::handleToApi(handle.c_str());
        if (!pApi) {
            throw std::runtime_error("Invalid handle");
        }
        
        std::vector<std::string> dirs = pApi->ls(path.c_str());
        
        // marshall the vector into a Tcl list:
        
        CTCLObject result;
        result.Bind(interp);
        for (int i = 0; i < dirs.size(); i++) {
            CTCLObject directory;
            directory.Bind(interp);
            directory = dirs[i];
            result += directory;
        }
        
        interp.setResult(result);
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