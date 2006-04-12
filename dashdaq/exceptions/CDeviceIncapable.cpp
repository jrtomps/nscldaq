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
#include "CDeviceIncapable.h"

#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif

/*!
   Constructs an exception that can be thrown when someone
   attempts an operation with a device that is not supported by the
   device.
   \param attempted : string [in]
       The operation that was being attempted.
   \param wasdoing : string[in]
       What was being done when the operation was being attempted 
       (sort of a traceback).
   \param capabilities : string [in] defaults ""
       optional parameter that defines the capabilities the device
       does possess.
*/
CDeviceIncapable::CDeviceIncapable(string attempted, string wasdoing, 
				   string capabilities) :
  CException(wasdoing),
  m_attempted(attempted),
  m_capabilities(capabilities)
{}

/*!
   Copy constructor.
*/
CDeviceIncapable::CDeviceIncapable(const CDeviceIncapable& rhs) :
  CException(rhs),
  m_attempted(rhs.m_attempted),
  m_capabilities(m_capabilities)
{}
/*!
  Destructor 
*/
CDeviceIncapable::~CDeviceIncapable()
{
}

/*!
   Assignment
*/
CDeviceIncapable&
CDeviceIncapable::operator=(const CDeviceIncapable& rhs)
{
  if (&rhs != this) {
    CException::operator=(rhs);
    m_attempted    = rhs.m_attempted;
    m_capabilities = rhs.m_capabilities;
  }
  return *this;
}
/*!
   Comparison for equality
*/
int
CDeviceIncapable::operator==(const CDeviceIncapable& rhs)
{
  return (CException::operator==(rhs)                    &&
	  (m_attempted     == rhs.m_attempted)           &&
	  (m_capabilities  == rhs.m_capabilities));
}
/*!
   Comparison for inequality.
*/
int
CDeviceIncapable::operator!=(const CDeviceIncapable& rhs)
{
  return !(*this == rhs);
}

/*!
    Returns human readable reasons for the error.
    The value depends on whether or not the capabilities string is blank
*/
const char*
CDeviceIncapable::ReasonText() const
{
  m_reason    = string("Device not capable of attempted operation '");
  m_reason   += m_attempted;
  m_reason   += string("'");
  if (m_capabilities != string("")) {
    m_reason += string(" Device capabilities: ");
    m_reason += m_capabilities;
  }
  return m_reason.c_str();
}

/*!
   Reason code is always 0
*/
int
CDeviceIncapable::ReasonCode() const
{
  return 0;
}
