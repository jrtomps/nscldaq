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

#ifndef __CMODULECOMMAND_H
#define __CMODULECOMMAND_H

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
    Implements the module command.  This class is subclassed from 
    CTclObjectProcessor  The module command is the command that
    each setup script uses to define the set of contrrol modules
    that exist.  At present, only a single type of control module
    is supported: CJTECGDG  Jtec/Wiener Gate and Delay generator.
*/
class CModuleCommand : public CTCLObjectProcessor
{
  // local member data:
private:
  TclServer&  m_Server;
  
  // Canonical functions:

public:
  CModuleCommand(CTCLInterpreter& interp,
		 TclServer&       server);
  virtual ~CModuleCommand();

private:
  CModuleCommand(const CModuleCommand& rhs);
  CModuleCommand& operator=(const CModuleCommand& rhs);
  int operator==(const CModuleCommand& rhs) const;
  int operator!=(const CModuleCommand& rhs) const;
public:

  // Virtual function overrides and implementations.
protected:
  virtual int operator()(CTCLInterpreter& interp,
			 std::vector<CTCLObject>& objv);
private:
  int create(CTCLInterpreter& interp,
	     std::vector<CTCLObject>& objv);
  int configure(CTCLInterpreter& interp,
		std::vector<CTCLObject>& objv);
  int cget(CTCLInterpreter& interp,
	   std::vector<CTCLObject>& objv);
  


};


#endif
