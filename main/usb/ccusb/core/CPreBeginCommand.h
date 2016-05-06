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
# @file   CPreBeginCommand.h
# @brief  Supplies the pre-begin run initialization command.
# @author <fox@nscl.msu.edu>
*/

#ifndef CPREBEGINCOMMAND_H
#define CPREBEGINCOMMAND_H

#include <TCLObjectProcessor.h>

class CTCLInterpreter;
class CTCLObject;

/**
 * @class CPreBeginCommand
 *    Provides a command that performs all pre-begin initialization. This includes
 *    -  Sizing the stacks.
 *    -  Initializing the hardware.
 *    -  Loading the stacks.
 *   The 'begin' command then will actually start the acquisition thread which
 *   enables stack triggers and puts the CCUSB into autonomous mode.
 *   
 */
class CPreBeginCommand : public CTCLObjectProcessor
{
public:
    CPreBeginCommand(CTCLInterpreter& interp);
    virtual ~CPreBeginCommand();
    
public:
    int operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void performPreBeginInitialization();           // Begin can call this.
};

#endif
