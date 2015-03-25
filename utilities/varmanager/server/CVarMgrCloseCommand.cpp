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
# @file   CVarMgrCloseCommand.cpp
# @brief  Implement the Tcl bindings for the variable manager close operation.
# @author <fox@nscl.msu.edu>
*/

#include "CVarMgrCloseCommand.h"
#include "CVarMgrOpenCommand.h"

#include <TCLInterpreter.h>
#include <TCLObject.h>
#include <stdio.h>


/**
 * constructor
 *    Register the new command.
 *  @param interp   - interpreter on which the command is registered.
 *  @param pCommand - Command word that executes the command.
 */
CVarMgrCloseCommand::CVarMgrCloseCommand(
    CTCLInterpreter& interp, const char* pCommand
) : CTCLObjectProcessor(interp, pCommand, true)
{}

/**
 * destructor
 */
CVarMgrCloseCommand::~CVarMgrCloseCommand()
{}

/**
 * operator()
 *    Executes the command:
 *    -  The handle parameter is required and extracted from the command.
 *    -  The handle is destroyed via the CVarMgrOpen::close() operation.
 *       That throws any needed exceptions.
 *
 * @param interp - interpreter running the command.
 * @param objv   - Objects that make up the command words.
 */
int
CVarMgrCloseCommand::operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    bindAll(interp, objv);
    char usage[100];
    sprintf(usage, "Usage\n  %s handle", std::string(objv[0]).c_str());
    try {
        requireExactly(objv, 2, usage);
        std::string handle = objv[1];
        CVarMgrOpenCommand::close(handle.c_str());
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
