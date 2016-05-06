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
# @file   CVarMgrGetwdCommand.cpp
# @brief  Implement the getwd operation.
# @author <fox@nscl.msu.edu>
*/

#include "CVarMgrGetwdCommand.h"
#include "CVarMgrOpenCommand.h"
#include "CVarMgrApi.h"
#include <TCLInterpreter.h>
#include <TCLObject.h>

/**
 * constructor
 *
 * @param interp    - interpreter the command is registered on.
 * @param pCommand  - Command word that activates us.
 */
CVarMgrGetwdCommand::CVarMgrGetwdCommand(CTCLInterpreter& interp, const char* pCommand) :
    CTCLObjectProcessor(interp, pCommand, true)
{}

/**
 * destructor
 */
CVarMgrGetwdCommand::~CVarMgrGetwdCommand() {}

/**
 * operator()
 *    Executes the command:
 *    - Validates the number of parameters
 *    - Extracts the handle and turns it into an api instance.
 *    - performthe getwd operation on the handle and set the command result.
 *
 * @param interp    - Intepreter running the command.
 * @param objv      - Vector of command line words
 * @return int - TCL_OK - if successful, TCL_ERROR if not.
 */
int
CVarMgrGetwdCommand::operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    char usage[100];
    bindAll(interp, objv);
    
    sprintf(usage, "Usage\n  %s handle", std::string(objv[0]).c_str());
    
    try {
        requireExactly(objv, 2, usage);
        std::string handle = objv[1];
        
        CVarMgrApi* pApi = CVarMgrOpenCommand::handleToApi(handle.c_str());
        if (!pApi) {
            throw std::runtime_error("Invalid handle");
        }
        interp.setResult(pApi->getwd());
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