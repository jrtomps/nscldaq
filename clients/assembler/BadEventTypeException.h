#ifndef __BADEVENTTYPEEXCEPTION_H
#define __BADEVENTTYPEEXCEPTION_H
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
   Exception class for event type errors:

*/
class BadEventTypeException : public CException
{
private:
  std::string   m_requiredType;
  unsigned int           m_providedType;
  mutable std::string m_ReasonText;

public:
  BadEventTypeException(unsigned int provided,
			std::string required,
			std::string doing);

  virtual const char* ReasonText() const;
  virtual Int_t ReasonCode() const;

private:
  std::string eventTypeString() const;

};



#endif
