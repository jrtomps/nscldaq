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

#ifndef __CRUNSTATECOMMAND_H
#define __CRUNSTATECOMMAND_H

#ifndef TCLOBJECTPROCESSOR_H
#include <TCLObjectProcessor.h>
#endif

#ifndef __STL_VECTOR
#include <vector>
#ifndef __STL_VECTOR
#define __STL_VECTOR
#endif
#endif

#ifndef __STL_STRING
#include <string>
#ifndef __STL_STRING
#define __STL_STRING
#endif
#endif


class CTCLInterpreter;
class CTCLObject;
class TclServer;
/*!
    Implements the runstate command.  This class is subclassed from 
    CTclObjectProcessor  The runstate command is the command that
    simply returns a string specifying the run state.
*/
class CRunStateCommand : public CTCLObjectProcessor
{
  // local member data:
private:
  TclServer&  m_Server;
  
  // Canonical functions:

public:
  CRunStateCommand(CTCLInterpreter& interp,
		 TclServer&       server);
  virtual ~CRunStateCommand();

private:
  CRunStateCommand(const CRunStateCommand& rhs);
  CRunStateCommand& operator=(const CRunStateCommand& rhs);
  int operator==(const CRunStateCommand& rhs) const;
  int operator!=(const CRunStateCommand& rhs) const;
public:

  // Virtual function overrides and implementations.
protected:
  virtual int operator()(CTCLInterpreter& interp,
			 std::vector<CTCLObject>& objv);

};


#endif
