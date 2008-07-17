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

#ifndef __CSETCOMMAND_H
#define __CSETCOMMAND_H

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
  CSetCommand implements the Set command.
  This command sets the value of a control point.
  The command has a single form:

  "Set name what value"

   where:
   - name is the name of a control object.
   - what is the name of a control point within that object.
   - value is the new value for that control point.
   
   Consider for example an 8 channel gate and delay generator named
   gdg1  A command like:

   Set gdg1 delay0 10

   Might set the delay for channel 0 to 10  in that module.
*/
class CSetCommand : public CTCLObjectProcessor
{
private:
  TclServer&   m_Server;	// Tcl server that is running us.
public:
  CSetCommand(CTCLInterpreter&   interp,
	      TclServer&         server);
  virtual ~CSetCommand();
private:
  CSetCommand(const CSetCommand& rhs);
  CSetCommand& operator=(const CSetCommand& rhs);
  int operator==(const CSetCommand& rhs) const;
  int operator!=(const CSetCommand& rhs) const;
public:

  // Command entry point:

protected:
  int operator()(CTCLInterpreter& interp,
		 std::vector<CTCLObject>& objv);


};

#endif
