/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2014.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Authors:
             Ron Fox
             Jeromy Tompkins 
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/


/** 
 * @file CPidToCommand.h
 * @brief Tcl interface to the Os::getProcessCommand API
 */

#ifndef CPIDTOCOMMAND_H
#define CPIDTOCOMMAND_H
#include <TCLObjectProcessor.h>

class CTCLInterpreter;
class CTCLObject;

/**
 * @class CPidToCommand
 *   Defines a Tcl command to turn a PID into the command
 *   it's running.  Note that the PID must refer to a current,
 *   non-zombified process.  The command is returned as a list
 *   of command words.
 */

class CPidToCommand : public CTCLObjectProcessor
{
public:
  CPidToCommand(CTCLInterpreter& interp, const char* pCommand="pidToCommand");
  virtual ~CPidToCommand();

  virtual int operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);

};

#endif
