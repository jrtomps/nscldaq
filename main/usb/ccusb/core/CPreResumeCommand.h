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
*   @file    CPreResumeCommand.h
*   @brief   Operations that prepare for a resume run.
*/
#ifndef CPRERESUMECOMMAND_H
#define CPRERESUMECOMMAND_H
#include <TCLObjectProcessor.h>

class CTCLInterpreter;
class CTCLObject;

/**
 * @class CPreResumeRun
 *    Operations that prepare to reusume a run.
 */
class CPreResumeCommand : public CTCLObjectProcessor
{
public:
    CPreResumeCommand(CTCLInterpreter& interp);
    virtual ~CPreResumeCommand();
    int operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void perform();
};

#endif