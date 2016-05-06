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
# @file   CVarMgrMkdirCommand.h
# @brief  Provide the mkdir command for Tcl binings to the variable manager api
# @author <fox@nscl.msu.edu>
*/
#ifndef CVARMGRMKDIRCOMMAND_H
#define CVARMGRMKDIRCOMMAND_H

#include <TCLObjectProcessor.h>
class CTCLInterpreter;
class CTCLObject;

/**
 * @class CVarMgrMkdirCommand
 *
 *  Provides the variable manager binding to the mkdir function; creates a new
 *  directory in the database.
 *
 * \verbatim
 *    varmgr::mkdkir handle path
 * \endverbatim
 */
class CVarMgrMkdirCommand : public CTCLObjectProcessor
{
public:
    CVarMgrMkdirCommand(CTCLInterpreter& interp, const char* pCommand);
    virtual ~CVarMgrMkdirCommand();
    
    int operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
};

#endif