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

#include <config.h>
#include "CDuplicateDevice.h"

#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif

CDuplicateDevice::CDuplicateDevice(const char* device,
				   const char* doing) :
  CException (doing),
  m_device(device)
{}

CDuplicateDevice::CDuplicateDevice(const CDuplicateDevice& rhs) :
  CException(rhs),
  m_device(rhs.m_device)
{}

CDuplicateDevice::~CDuplicateDevice() {}

CDuplicateDevice& 
CDuplicateDevice::operator=(const CDuplicateDevice& rhs)
{
  if (this != &rhs) {
    CException::operator=(rhs);
    m_device = rhs.m_device;
  }
  return *this;
}

int
CDuplicateDevice::operator==(const CDuplicateDevice& rhs) const
{
  return ((m_device == rhs.m_device)              &&
	  (CException::operator==(rhs)));
}
int
CDuplicateDevice::operator!=(const CDuplicateDevice& rhs) const
{
  return !(*this == rhs);
}


const char*
CDuplicateDevice::ReasonText() const
{
  m_Message = "Attempt to install an existing device: ";
  m_Message += m_device;
  m_Message += " while : ";
  m_Message += getAction();

  return m_Message.c_str();
}


int
CDuplicateDevice::ReasonCode() const
{
  return 0;
}
