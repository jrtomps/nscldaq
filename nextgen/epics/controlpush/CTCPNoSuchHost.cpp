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
// Copyright 
//   NSCL All rights reserved.
//

// Headers:

#include <config.h>
#include <CTCPNoSuchHost.h>
#include <netdb.h>

using namespace std;


/*!
  'Normal' Constructor for the exception class. Initializes the base class
  and sets up the initial object state.

  \param hostname Name of the host which could not be looked up.
  \param Doing    String which describes what the application was attempting
                  to do when the exceptional condition was thrown.  This
                  string is encoded verbatim into the ReasonText.

External Inputs:
- h_errno  The (thread) global containing the error number describing why
           the last host database function called failed.
 */
CTCPNoSuchHost::CTCPNoSuchHost(const string& hostname, const string& Doing) :
  CException(Doing),
  m_hErrno(h_errno),
  m_Host(hostname)
{
}

/*!
  Copy constructor. Used to build temporaries for e.g. call by value or
  return by value functional semantics.

  \param rhs - The original object from which the copy is constructed.

  */
CTCPNoSuchHost::CTCPNoSuchHost(const CTCPNoSuchHost& rhs) : 
  CException(rhs),
  m_hErrno(rhs.m_hErrno),
  m_Host(rhs.m_Host)
{
}
/*!
  Assignment operator

  \param rhs - the right hand side of the assignment.
 */
CTCPNoSuchHost&
CTCPNoSuchHost::operator=(const CTCPNoSuchHost& rhs)
{
  if(this != &rhs) {		// Need to assign
  }
  return *this;			// Allow chaining.
}

/*!
  Comparison operator.

  \param rhs - the right hand side of the comparison (Who *this is potentially
               equal to).
 */
int
CTCPNoSuchHost::operator==(const CTCPNoSuchHost& rhs)
{
  return (CException::operator==(rhs) &&
	  (m_hErrno == rhs.m_hErrno)  &&
	  (m_Host   == rhs.m_Host));
}

// Operations on the class:

/*!
  Returns a textual string indicating why the exception was thrown.
  This string is suitable for diaplay to a user.  The string is of the
  form:
     "Host Lookup failed for [m_Host] due to: [hstrerror(m_hErrno)] 
     while [WasDoing()]"
 */
const char*
CTCPNoSuchHost::ReasonText() const
{
  m_Reason  = "Host Lookup failed for ";
  m_Reason += m_Host;
  m_Reason += " due to: ";
  m_Reason += hstrerror(m_hErrno);
  m_Reason += " while ";
  m_Reason += WasDoing();

  return m_Reason.c_str();
}
/*!
  Returns the exception specific reason code.  In this case, m_hErrno.
  */
Int_t
CTCPNoSuchHost::ReasonCode() const
{
  return m_hErrno;
}
