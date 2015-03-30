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
# @file   CVarMgrRmdirCommand.cpp
# @brief  Implement the rmdir command.
# @author <fox@nscl.msu.edu>
*/
#include "CVarMgrRmdirCommand.h"
#include "CVarMgrOpenCommand.h"
#include "CVarMgrApi.h"

#include <TCLInterpreter.h>
#include <TCLObject.h>
#include <stdio.h>
#include <stdexcept>

/**
 * constructor
 *
 * @param interp  - interpreter on which the command will be registered.
 * @param pCommand- String that triggers the command.
 */
CVarMgrRmdirCommand::CVarMgrRmdirCommand(CTCLInterpreter& interp, const char* pCommand) :
    CTCLObjectProcessor(interp, pCommand, true)
{}

/**
 * destructor
 */
CVarMgrRmdirCommand::~CVarMgrRmdirCommand() {}

/**
 * operator()
 *    Actually does the rmdir operation:
 *    -   Require a handle and path.
 *    -   Extract the handle and translate it to an Api object.
 *    -   Perform the rmdi on the API object.
 *
 * @param interp    - Interpreter performing the command.
 * @param objv      - Tcl objects that make up the command words.
 * @return int - TCL_OK for success, TCL_ERROR for failure.
 */
int
CVarMgrRmdirCommand::operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    char usage[100];
    bindAll(interp, objv);
    sprintf(usage, "Usage:\n   %s  handle path", std::string(objv[0]).c_str());
    
    try {
        requireExactly(objv, 3, usage);
        
        std::string handle = objv[1];
        std::string path   = objv[2];
        
        CVarMgrApi* pApi = CVarMgrOpenCommand::handleToApi(handle.c_str());
        if (!pApi) {
            throw std::runtime_error("Invalid handle");
        }
        pApi->rmdir(path.c_str());
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

