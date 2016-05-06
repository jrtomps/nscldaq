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
*   @file   CPreResumeCommand.h
*   @brief  Class that implements the preresume command.
*/
#ifndef PRERESUMECOMMAND_H
#define PRERESUMECOMMAND_H

#include <TCLObjectProcessor.h>
class CTCLInterpreter;
class CTCLObject;

/**
 * @class  CPreResumeCommand
 *     The preresume command prepares a pause run to be resumed.
 */
class CPreResumeCommand : public CTCLObjectProcessor
{
public:
    CPreResumeCommand(CTCLInterpreter& interp);
    virtual ~CPreResumeCommand();
    
    int operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objb);
    void perform();                            // So Resume can call this.
};

#endif