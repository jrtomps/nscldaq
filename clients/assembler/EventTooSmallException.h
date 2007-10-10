#ifndef __EVENTTOOSMALLEXCEPTION_H
#define __EVENTTOOSMALLEXCEPTION_H

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
   Exception that can be thrown if a physics event segment is
   smaller than some minimum amount... a bit like a single ended range error.
   ..should probably have used that instead.
*/
class EventTooSmallException : public CException
{
private:
  int    m_actualSize;
  int    m_requestedSize;
  mutable std::string m_reasonText;

public:
  EventTooSmallException(int actualSize,
			 int requiredSize,
			 std::string doing);

  int getActualSize() const;
  int getRequestedSize() const;

  virtual const char* ReasonText() const;
  virtual Int_t ReasonCode() const;

  
};

#endif
