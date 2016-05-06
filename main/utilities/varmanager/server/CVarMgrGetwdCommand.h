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
# @file   CVarMgrGetwdCommand.h
# @brief  Variable manager Tcl interface to getwd operation.
# @author <fox@nscl.msu.edu>
*/

#ifndef CVARMGRGETWDCOMMAND_H
#define CVARMGRGETWDCOMMAND_H

#include <TCLObjectProcessor.h>
class CTCLInterpreter;
class CTCLObject;


/**
 * @class CVarMgrGetwdCommand
 *
 *    Provide the getwd functionality for the Varmgr api to tcl clients.
 *
 *  \verbatim
 *     set wd [::varmgr::getwd $handle]
 *  \endverbatim
 */
class CVarMgrGetwdCommand : public CTCLObjectProcessor
{
public:
    CVarMgrGetwdCommand(CTCLInterpreter& interp, const char* pCommand);
    virtual ~CVarMgrGetwdCommand();
    
    int operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
};

#endif