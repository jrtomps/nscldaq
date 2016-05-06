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
# @file   CVarMgrVarCommand.h
# @brief  var compatibility command for VarMgr Tcl api.
# @author <fox@nscl.msu.edu>
*/

#ifndef CVARMGRVARCOMMAND_H
#define CVARMGRVARCOMMAND_H

#include <TCLObjectProcessor.h>
class CTCLInterpreter;
class CTCLObject;
class CVarMgrApi;


/**
 * @class CVarMgrVarCommand
 *    Provide the var command.  This is a command ensemble that has the following
 *    subcommands:
 *
 *    - create - creates a new variable:
 *
 *  \verbatim
 *
 *  varmgr::var create handle path type ?initial-value
 *
 * \endverbatim
 */

class CVarMgrVarCommand : public CTCLObjectProcessor
{
public:
    CVarMgrVarCommand(CTCLInterpreter& interp, const char* pCommand);
    virtual ~CVarMgrVarCommand();
    
    int operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    
protected:
    void create(CVarMgrApi* pApi, std::vector<CTCLObject>& objv);
    std::string get(CVarMgrApi* pApi, std::vector<CTCLObject>& objv);
    void set(CVarMgrApi* pApi, std::vector<CTCLObject>& objv);
    void ls(CTCLInterpreter& interp, CVarMgrApi* pApi, std::vector<CTCLObject>& objv);
    void rmvar (CVarMgrApi* pApi, std::vector<CTCLObject>& objv);
};


#endif