
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
 * @file CPreBeginCommand.h
 * @brief Command to perform initialization actions prior to run start.
 */
#ifndef CPREBEGINCOMMAND_H
#define CPREBEGINCOMMAND_H

#include <TCLObbjectProcessor.h>

class CTCLInterpreter;
class CTCLObject;


/**
 * @class CPreBeginCommand
 *     Supplies the prebegin command. This command is used to initialize the
 *     DAQ hardware prior to a begin run.  This means:
 *     -  Reading the configurations.
 *     -  Ensuring the stacks fit in the VMUSB stack memory.
 *     -  Setting the controller into its default mode.
 *     -  Running module initialization methods.
 *     -  Loading stacks.
 *     -  Setting stack triggers.
 */
class CPreBeginCommand : public CTCLObjectProcessor
{
public:
    CPreBeginCommand(CTCLInterpreter& interp);
    virtual ~CPreBeginCommand();1
    int operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void perform();
};

#endif