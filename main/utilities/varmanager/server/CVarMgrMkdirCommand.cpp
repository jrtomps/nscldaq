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
# @file   CVarMgrMkdirCommand.cpp
# @brief  Implement the mkdir operation.
# @author <fox@nscl.msu.edu>
*/
#include "CVarMgrMkdirCommand.h"
#include "CVarMgrOpenCommand.h"
#include "CVarMgrApi.h"

#include <TCLInterpreter.h>
#include <TCLObject.h>




/**
 * constructor
 *   @param interp   - interpreter on which the command is registered.
 *   @param pCommand - Command word that executes us.
 */
CVarMgrMkdirCommand::CVarMgrMkdirCommand(
    CTCLInterpreter& interp, const char* pCommand
) : CTCLObjectProcessor(interp, pCommand, true)
{}

/**
 * destructor
 */
CVarMgrMkdirCommand::~CVarMgrMkdirCommand() {}

/**
 * operator()
 *    Execute the command:
 *    - Ensure there are the right number of parameters.
 *    - Extract/translate the handle -> API
 *    - Extract the path parameter
 *    - Execute the api's mkdir method.
 *
 *  @param interp   - interpreter executing the commabnd.
 *  @param objv     - vector of command words.
 *  @return int TCL_OK for success, TCL_ERROR if not.
 */
int
CVarMgrMkdirCommand::operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    char usage[100];
    bindAll(interp, objv);
    
    sprintf(usage, "Usage\n  %s handle dir-path", std::string(objv[0]).c_str());
    
    try {
        requireExactly(objv, 3, usage);
       
        std::string handle = objv[1];
        std::string path   = objv[2];
        
        CVarMgrApi* pApi = CVarMgrOpenCommand::handleToApi(handle.c_str());
        if (!pApi) {
            throw std::runtime_error("Invalid handle");
        }
        
        pApi->mkdir(path.c_str());
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