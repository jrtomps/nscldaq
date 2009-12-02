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

// Header files:

#include <config.h>
#include <CTCPNoSuchService.h>


using namespace std;



// Constructors and related functions:

/*! 
  Standard constructor. Initialize the member variables to values
  determined by the application and environmental parameters.

  \param service Textualized service which could not be found. This could
                 either be a service name (mis-spelled e.g.) or a textualized
                 service number.
  \param Doing   Context information provided by the application to describe
                 what it was trying to do when the failure was detected.

  External inputs:
  - errno (Thread specific) global variable containing the reason getserv
           database functions failed.
 */
CTCPNoSuchService::CTCPNoSuchService(const string& service, 
				     const string& Doing) :
  CErrnoException(Doing),
  m_Service(service)
{}

/*!
   Copy constructor: Creates an object given a reference object.  Used by
   the language to produce temporaries for .e.g. call by value and return
   by value function interactions.

   \param rhs The reference object being copied into *this.
 */
CTCPNoSuchService::CTCPNoSuchService(const CTCPNoSuchService& rhs) :
  CErrnoException(rhs),
  m_Service(rhs.m_Service)
{
  
}
/*!
  Assignment.  Assign *this to a rhs object. Different from copy construction
  in that:
  - We already exist as a validly constructed object.
  - We return a reference to ourselves so that = chaining is supported.
  - We prevent self assignment.

  \param rhs The item to which we are being assigned.
  */
CTCPNoSuchService&
CTCPNoSuchService::operator=(const CTCPNoSuchService& rhs)
{
  if(this == &rhs) {
    CErrnoException::operator=(rhs);
    m_Service = rhs.m_Service;
  }
  return *this;
}
/*!
  Equality compare.. This is a field by field compare of the important elements
  Note that the m_Reason member is unimportant to the compare.

  \param rhs The object *this is being compared to.
  */
int
CTCPNoSuchService::operator==(const CTCPNoSuchService& rhs)
{
  return (CErrnoException::operator==(rhs) &&
	  (m_Service == rhs.m_Service));
}

// Operations on the class.

/*!
  Returns the reason for the exception. This is a string which is built up
  from CErrno::ReasonText and other information we have it is of the form:
    "Unable to translate service [m_Service] becuase: [CErrno::ReasonText()]"
 */
const char*
CTCPNoSuchService::ReasonText() const
{
  m_Reason  = "Unable to translate service ";
  m_Reason += m_Service;
  m_Reason += " because: ";
  m_Reason += CErrnoException::ReasonText();
  return m_Reason.c_str();
}
