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

#ifndef __CNOSUCHDEVICE_H
#define __CNOSUCHDEVICE_H
#ifndef __EXCEPTION_H
#include <Exception.h>
#endif

#ifndef __STD_STRING
#include <string>
#ifndef __STD_STRING
#define __STD_STRING
#endif
#endif
/*!
   Class to report attempts to reference devices that don't exist.
   (exception).  The message associated with this class will
   be of the form:
   
No such $device number $num while : $doing

   The error code will be the device number.
*/
class CNoSuchDevice : public CException
{
private:
  STD(string) m_deviceName;
  int         m_deviceNumber;
  mutable STD(string) m_Message; // Needed for scoping issues in ReasonText()
public:
  CNoSuchDevice(const char* name, int number, const char* doing);
  CNoSuchDevice(const CNoSuchDevice& rhs);
  virtual ~CNoSuchDevice();

  CNoSuchDevice& operator=(const CNoSuchDevice& rhs);
  int operator==(const CNoSuchDevice& rhs) const;
  int operator!=(const CNoSuchDevice& rhs) const;


  virtual const char* ReasonText() const;
  virtual int ReasonCode() const;
};

#endif
