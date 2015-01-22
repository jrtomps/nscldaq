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

#ifndef __CGETCOMMAND_H
#define __CGETCOMMAND_H

#ifndef __TCLOBJECTPROCESSOR_H
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

class TclServer;
class CTCLObject;
class CTCLInterpreter;
class CVMUSB;

/*!
   CGetCommand implements the Get command this command
   retrieves the value of a control point.
   The single form is:

   Get name what

   Where:
   - name is the name of a control object
   - what is the name of a control point within the object.
*/

class CGetCommand : public CTCLObjectProcessor
{
  TclServer&   m_Server;	// Tcl server that is running us.
  CVMUSB&      m_Vme;
public:
  CGetCommand(CTCLInterpreter&   interp,
	      TclServer&         server,
	      CVMUSB&            vme);
  virtual ~CGetCommand();
private:
  CGetCommand(const CGetCommand& rhs);
  CGetCommand& operator=(const CGetCommand& rhs);
  int operator==(const CGetCommand& rhs) const;
  int operator!=(const CGetCommand& rhs) const;
public:

  // Command entry point:

protected:
  int operator()(CTCLInterpreter& interp,
		 std::vector<CTCLObject>& objv);


};



#endif
