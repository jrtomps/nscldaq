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

#ifndef __CDUPLICATEDEVICE_H
#define __CDUPLICATEDEVICE_H
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
   This class is an exception that reports duplicate devices.
   Messages are produced of the form:
   
A duplicate device : $what was produced while $doing.

error codes are always 0.
*/
class CDuplicateDevice : public CException
{
private:
  STD(string)         m_device;
  mutable STD(string) m_Message; // Needed for scoping reasons by ReasonText().
public:
  CDuplicateDevice(const char* device,const char* doing);
  CDuplicateDevice(const CDuplicateDevice& rhs);
  virtual ~CDuplicateDevice();

  CDuplicateDevice& operator=(const CDuplicateDevice& rhs);
  int operator==(const CDuplicateDevice& rhs) const;
  int operator!=(const CDuplicateDevice& rhs) const;

  virtual const char* ReasonText() const;
  virtual int ReasonCode() const;
};


#endif
