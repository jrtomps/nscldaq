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
# @file   CVarMgrLsCommand.h
# @brief  Provide ls operation for varmgr Tcl api bindings.
# @author <fox@nscl.msu.edu>
*/

#ifndef CVARMGRLSCOMMAND_H
#define CVARMGRLSCOMMAND_H

#include <TCLObjectProcessor.h>
class CTCLInterpreter;
class CTCLObject;

/**
 * @class CVarMgrLsCommand
 *
 *    Provide the varmgr api ls command.  This returns a list of the subdirectories
 *    that are living in some directory path:
 *
 * \verbatim
 *    varmr::ls handle ?path?
 * \endverbatim
 *
 *   - If path is not supplied, the cwd is listed.
 *   - If path is supplied and absolute, that path is listed.
 *   - If path is supplied and relative the actual listed path is calculated
 *     relative to the cwd.
 */
class CVarMgrLsCommand : public CTCLObjectProcessor
{
public:
    CVarMgrLsCommand(CTCLInterpreter& interp, const char* pCommand);
    virtual ~CVarMgrLsCommand();
    
    int operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
};

#endif
