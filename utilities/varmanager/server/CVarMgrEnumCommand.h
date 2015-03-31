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
# @file   CVarMgrEnumCommand.h
# @brief  Enumeration command for Tcl bindings to varmgr api.
# @author <fox@nscl.msu.edu>
*/

#ifndef CVARMGRENUMCOMMAND_H
#define CVARMGRENUMCOMMAND_H
#include <TCLObjectProcessor.h>

class CTCLInterpreter;
class CTCLObject;

/**
 * @class CVarMgrEnumCommand
 *     Implement the enum command for the VarMgr Api:
 *
 * \verbatim
 *   enum handle type-name enumeration-value-list
 * \endverbatim
 */
class CVarMgrEnumCommand : public CTCLObjectProcessor
{
public:
    CVarMgrEnumCommand(CTCLInterpreter& interp, const char* pCommand);
    virtual ~CVarMgrEnumCommand();
    
    int operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
};

#endif
