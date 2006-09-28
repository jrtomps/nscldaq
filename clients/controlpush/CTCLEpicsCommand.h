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

#ifndef __CTCLEPICSCOMMAND_H
#define __CTCLEPICSCOMMAND_H

#ifndef __TCLOBJECTPROCESSOR_H
#include <TCLObjectProcessor.h>
#endif

#ifndef __STL_STRING
#include <string>
#ifndef __STL_STRING
#define __STL_STRING
#endif
#endif

#ifndef __STL_VECTOR
#include <vector>
#ifndef __STL_VECTOR
#define __STL_VECTOR
#endif
#endif

class CTCLInterpeter;
class CTCLObject;


/*!
    The epicschannel command provides a mechanisms for creating objects
    of type CTCLChannelCommand... that is for creating a command that
    accesses Epics channels or epics channel data record entries.

    The syntax of this command is trivial:

    epicschannel  name

    where name is the name of both the channel and command for the channel
    The command returns the name string on success.
*/
class CTCLEpicsCommand  : public CTCLObjectProcessor
{
public:
  CTCLEpicsCommand(CTCLInterpreter& interp, STD(string) command = STD(string)("epicschannel"));
  virtual ~CTCLEpicsCommand();
private:
  CTCLEpicsCommand(const CTCLEpicsCommand& rhs);
  CTCLEpicsCommand& operator=(const CTCLEpicsCommand& rhs);
  int operator==(const CTCLEpicsCommand& rhs) const;
  int operator!=(const CTCLEpicsCommand& rhs) const;
public:


  virtual int operator()(CTCLInterpreter& interp,
			 STD(vector)<CTCLObject>& objv);


};


#endif
