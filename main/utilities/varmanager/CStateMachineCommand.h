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
# @file   CStateMachineCommand.h
# @brief  Tcl bindings to statemachine.
# @author <fox@nscl.msu.edu>
*/

#ifndef __CSTATEMACHINECOMMAND_H
#define __CSTATEMACHINECOMMAND_H
#include <TCLObjectProcessor.h>

class CTCLInterpreter;
class CTCLObject;

/**
 * @class CStateMachineCommand
 *    Tcl bindings to allow the creation of state machines.
 */
class CStateMachineCommand : public CTCLObjectProcessor
{
public:
    CStateMachineCommand(CTCLInterpreter& interp, const char* pName);
    virtual ~CStateMachineCommand();
    
    int operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
};


#endif