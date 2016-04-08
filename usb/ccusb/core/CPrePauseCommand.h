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
# @file   CPrePauseCommand.h
# @brief  Prepare the system for a pause.
# @author <fox@nscl.msu.edu>
*/

#ifndef CPREPAUSECOMMAND_H
#define CPREPAUSECOMMAND_H

#include "TCLObjectProcessor.h"

class CTCLInterpreter;
class CTCLObject;

/**
 * @class CPrePauseCommand
 *     Class to execute the pre-pause operation.  For now this operation
 *     just invokes the high level controller's  performStopOperations method.
*/
class CPrePauseCommand : public CTCLObjectProcessor
{
public:
    CPrePauseCommand(CTCLInterpreter& interp);
    virtual ~CPrePauseCommand();
    
    int operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void perform();
    
};
#endif


