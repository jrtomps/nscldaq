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
#include <CNoSuchDevice.h>
#include <stdio.h>

#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif

CNoSuchDevice::CNoSuchDevice(const char* name, int number, const char* doing) :
  CException(doing),
  m_deviceName(name),
  m_deviceNumber(number)
{}

CNoSuchDevice::CNoSuchDevice(const CNoSuchDevice& rhs) :
  CException(rhs),
  m_deviceName(rhs.m_deviceName),
  m_deviceNumber(rhs.m_deviceNumber)
{}

CNoSuchDevice::~CNoSuchDevice()
{}

CNoSuchDevice&
CNoSuchDevice::operator=(const CNoSuchDevice& rhs)
{
  if(this != &rhs) {
    CException::operator=(rhs);
    m_deviceName = rhs.m_deviceName;
    m_deviceNumber = rhs.m_deviceNumber;
  }
  return *this;
		
}
int
CNoSuchDevice::operator==(const CNoSuchDevice& rhs) const
{
  return ((m_deviceName == rhs.m_deviceName)     &&
	  (m_deviceNumber == rhs.m_deviceNumber) &&
	  (CException::operator==(rhs)));
}
int
CNoSuchDevice::operator!=(const CNoSuchDevice& rhs) const
{
  return !(*this == rhs);
}

int
CNoSuchDevice::ReasonCode() const
{
  return m_deviceNumber;
}

const char*
CNoSuchDevice::ReasonText() const
{
  char message [1000];
  sprintf(message,"No such %s device %d while: %s",
	  m_deviceName.c_str(), m_deviceNumber, WasDoing());
  string m_Message = message;
  return m_Message.c_str();
}
