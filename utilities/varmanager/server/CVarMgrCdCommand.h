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
# @file   CVarMgrCdCommand.h
# @brief  Tcl bindings to the varmgr api - cd operation.
# @author <fox@nscl.msu.edu>
*/
#ifndef CVARMGRCDCOMMAND_H
#define CVARMGRCDCOMMAND_H

#include "TCLObjectProcessor.h"

class CTCLInterpreter;
class CTCLObject;

/**
 * @class CVarMgrCdCommand
 *     Provide access to the cd operation of the CVarMgrApi.
 *  \verbatim
 *    varmgr::cd handle directory-path
 *  \endverbatim
 */
class CVarMgrCdCommand : public CTCLObjectProcessor
{
public:
    CVarMgrCdCommand(CTCLInterpreter& interp, const char* pCommand);
    virtual ~CVarMgrCdCommand();
    
    int operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
};

#endif
