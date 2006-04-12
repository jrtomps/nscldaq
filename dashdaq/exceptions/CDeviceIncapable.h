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

#ifndef __CDEVICEINCAPABLE_H
#define __CDEVICEINCAPABLE_H

#ifndef __EXCEPTION_H
#include "Exception.h"
#endif

#ifndef __STL_STRING
#include <string>
#ifndef __STL_STRING
#define __STL_STRING
#endif
#endif


/*!
    Report exceptions that state the device is not capable
    of doing what it was asked to do.
*/
class CDeviceIncapable : public CException
{
  STD(string)  m_attempted;
  STD(string)  m_capabilities;
  mutable STD(string)  m_reason; /* Need this for scope issues. */
public:
  CDeviceIncapable(STD(string) attempted,
		   STD(string) wasdoing,
		   STD(string) capabilities = STD(string)(""));
  CDeviceIncapable(const CDeviceIncapable& rhs);
  virtual ~CDeviceIncapable();

  CDeviceIncapable& operator=(const CDeviceIncapable& rhs);
  int operator==(const CDeviceIncapable& rhs);
  int operator!=(const CDeviceIncapable& rhs);

  virtual const char* ReasonText() const;
  virtual int ReasonCode() const;

};

#endif
