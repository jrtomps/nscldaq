#ifndef __ASSEMBLERWRONGSTATEEXCEPTION_H
#define __ASSEMBLERWRONGSTATEEXCEPTION_H
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

#ifndef __EXCEPTION_H
#include <Exception.h>
#endif

#ifndef __STL_STRING
#include <string>
#ifndef __STL_STRING
#define __STL_STRING
#endif
#endif


/*!
  Reports exceptions that can be thrown when the assembler input stage
  is in the wrong state while attempting to start/stop asembly.
*/
class AssemblerWrongStateException : public CException
{
public:
  typedef enum _State {
    inactive,
    active
  } State;

private:
  State       m_actualState;
  std::string m_reasonText;

public:
  AssemblerWrongStateException(AssemblerWrongStateException::State state,
			       std::string                         attempted,
			       std::string                         wasDoing);

  virtual const char* ReasonText() const;
  virtual Int_t       ReasonCode() const;

};



#endif
