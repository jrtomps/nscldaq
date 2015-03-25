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
# @file   <filename>
# @brief  <brief description>
# @author <fox@nscl.msu.edu>
*/
#include "CVarMgrCdCommand.h"
#include "CVarMgrOpenCommand.h"
#include "CVarMgrApi.h"

#include <TCLInterpreter.h>
#include <TCLObject.h>

/**
 * constructor
 *
 * @param interp     - interpreter on which the command is registered.
 * @param pCommand   - Command word that activates us.
 */
CVarMgrCdCommand::CVarMgrCdCommand(CTCLInterpreter& interp, const char* pCommand) :
    CTCLObjectProcessor(interp, pCommand, true)
{}

/**
 * destructor
 */
CVarMgrCdCommand::~CVarMgrCdCommand() {}

/**
 * operator()
 *   Executes the command:
 *   -  Ensures we have the right number of command parameters.l
 *   -  extracts and translates the handle to an API object.
 *   -  extracts the directory path.
 *   -  uses cd on the API object.
 *
 * @param interp     - interpreter executing the command.
 * @param objv       - Vector of command words.
 * @return int       - TCL_OK if success, TCL_ERROR if not.
 */
int
CVarMgrCdCommand::operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    char usage[100];
    bindAll(interp, objv);
    
    sprintf(
        usage, "Usage\n  %s handle directory-path",
        std::string(objv[0]).c_str()
    );
    
    try {
        requireExactly(objv, 3, usage);
        
        std::string handle = objv[1];
        std::string path   = objv[2];
        
        CVarMgrApi* pApi = CVarMgrOpenCommand::handleToApi(handle.c_str());
        if (!pApi) {
            throw std::runtime_error("Invalid handle");
        }
        pApi->cd(path.c_str());
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