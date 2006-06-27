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

#ifndef __CBADVALUE_H
#define __CBADVALUE_H
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
    This is an exception that can be thrown in case an invalid parameter
    is passed to a member function, where invalidity is more complex than
    a simple range error.
*/
class CBadValue : public CException
{
private:
  STD(string) m_allowed;
  STD(string) m_actual;
  mutable STD(string) m_message;

public:
  CBadValue(const char* legal, const char* got, const char* wasdoing);
  CBadValue(const CBadValue& rhs);
  ~CBadValue();

  CBadValue& operator=(const CBadValue& rhs);
  int operator==(const CBadValue& rhs) const;
  int operator!=(const CBadValue& rhs) const;

  virtual const char* ReasonText() const;
  virtual int         ReasonCode() const;
};

#endif
