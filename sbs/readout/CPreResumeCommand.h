/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2005.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

/**
*   @file   CPfeResumeCommand.h
*   @brief  Command to prepare for resuming a run.
*   
*/
#ifndef CPRERESUMECOMMAND_H
#define  CPRERESUMECOMMAND_H

#include <TCLPackagedObjectProcessor.h>
class CTCLInterpreter;
class CTCLObject;

/**
 * @class CPreResumeCommand
 *      Command execution class that performs the actions needed to prepare to
 *      resume a run.  The run is actually resumed by the resume command
 *      (CResumeCommand).
 */

class CPreResumeCommand : public CTCLPackagedObjectProcessor
{
public:
    CPreResumeCommand(CTCLInterpreter& interp);
    virtual ~CPreResumeCommand();
    
    int operator()(CTCLInterpreter& interp, std::vector<CTCLObject& objv);
    
};

#endif 