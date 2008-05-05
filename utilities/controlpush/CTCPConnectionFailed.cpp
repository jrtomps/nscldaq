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


static const char* Copyright= "(C) Copyright Michigan State University 2002, All rights reserved";// Author:
//   Ron Fox
//   NSCL
//   Michigan State University
//   East Lansing, MI 48824-1321
//   mailto:fox@nscl.msu.edu
//

#include <config.h>
#include "CTCPConnectionFailed.h"

using namespace std;


// Constructors and related functions:

/*!
  'Normal constructor' intended to be used to instantiate an object
  prior to throwing it as an exception.

  \param host - Name of the host which to which the connection was attempted
  \param service - Textualized service to which the connection was attempted
  \param pDoing  - Context information describing what CSocket was doing when
                   the exceptional condition was detected.
 */
CTCPConnectionFailed::CTCPConnectionFailed(const string& host,
					    const string& service,
					   const char*   pDoing) :
  CErrnoException(pDoing),
  m_Host(host),
  m_Service(service)
{
}
/*!
  Copy Constructor. Used by the compiler to create temporaries and by
  the throw statement to create a copy of the actual exception object to ensure
  that the exception remains in scope while it travels up the call stack 
  searching for a handler.

  \param rhs - The reference object which is being copy constructed.
  */
CTCPConnectionFailed::CTCPConnectionFailed(const CTCPConnectionFailed& rhs) :
  CErrnoException(rhs),
  m_Host(rhs.m_Host),
  m_Service(rhs.m_Service)
{
}
/*!
  Assignment.  The functionality is the same as a copy constructor, however
  the target is a fully constructed object.  We protect, therefore against
  assignment to *this, and since this is not a constructor, we cannot use
  initializers to assign our members.
  \param rhs - The object we are being assigned to.
  */
CTCPConnectionFailed&
CTCPConnectionFailed::operator=(const CTCPConnectionFailed& rhs)
{
  if(this != &rhs) {
    CErrnoException::operator=(rhs);
    m_Host    = rhs.m_Host;
    m_Service = rhs.m_Service;
  }
  return *this;
}
/*!
  Equality compare... This is essentially a member by member compare.
  \param rhs - the object to which we are being compared.
  */
int
CTCPConnectionFailed::operator==(const CTCPConnectionFailed& rhs)
{
  return (CErrnoException::operator==(rhs)      &&
	  (m_Host    == rhs.m_Host)             &&
	  (m_Service == rhs.m_Service));
}
/*!
  Returns a textual string describing the message.
  This is going to be something like:
  Failed to connect to host: m_Host on service port m_Service, 
  CErrnoException::ReasonText().
  */
const char*
CTCPConnectionFailed::ReasonText() const
{
  m_ReasonText  = "Failed to connect to host ";
  m_ReasonText += m_Host;
  m_ReasonText += " on service port: ";
  m_ReasonText += m_Service;
  m_ReasonText += ": ";
  m_ReasonText += CErrnoException::ReasonText();

  return m_ReasonText.c_str();
}
