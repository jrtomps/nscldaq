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
# @file   CVarMgrCreateCommand.h
# @brief  Header for varmgr::create command.
# @author <fox@nscl.msu.edu>
*/
#ifndef CVARMGRCREATECOMMAND_H
#define CVARMGRCREATECOMMAND_H
#include <TCLObjectProcessor.h>

class CTCLInterpreter;
class CTCLObject;

/**
 * @class CVarmgrCreateCommand
 *    Actually directly uses the vardb functions to create an empty
 *    database.
 *    Syntax:
 *
 *\verbatim
 *    varmgr::create filename
 * \endverbatim
 */
class CVarMgrCreateCommand : public CTCLObjectProcessor
{
public:
    CVarMgrCreateCommand(CTCLInterpreter& interp, const char* cmdName);
    virtual ~CVarMgrCreateCommand();
    
    int operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
};


#endif
