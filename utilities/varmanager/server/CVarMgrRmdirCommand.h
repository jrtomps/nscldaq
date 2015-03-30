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
# @file   CVarMgrRmdirCommand.h
# @brief  Provide rmdir functionality to the Tcl api bindings.
# @author <fox@nscl.msu.edu>
*/

#ifndef CVARMGRRMDIRCOMMAND_H
#define CVARMGRRMDIRCOMMAND_H

#include <TCLObjectProcessor.h>
class CTCLInterpreter;
class CTCLObject;

/**
 * @class CVarMgrRmdirCommand
 *
 *   Provides the ability to remove directories to the tcl VarMgr API bindings.
 *
 * \verbatim
 *
 *  rmdir $handle path
 * \endverbatim
 *
 * @note the cwd is used as the base for computing the directory removed
 * when path is relative.
 */
class CVarMgrRmdirCommand : public CTCLObjectProcessor
{
public:
    CVarMgrRmdirCommand(CTCLInterpreter& interp, const char* pCommand);
    virtual ~CVarMgrRmdirCommand();
    
    int operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
};

#endif