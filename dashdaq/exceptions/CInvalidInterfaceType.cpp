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
#include "CInvalidInterfaceType.h"

#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif


/*!
   Construct the exception, we save our own description and pass the
   action on to the base class.
   \param description : string [in]
       Description line that caused the error.
   \param action : string [in]
       Action being performed when the exception was thrown.

*/
CInvalidInterfaceType::CInvalidInterfaceType(string description, string action) :
  CException(action),
  m_description(description)
{
}

/*!
  Copy construction, copy construct the base class and
  copy the description from the object being cloned.
*/
CInvalidInterfaceType::CInvalidInterfaceType(const CInvalidInterfaceType& rhs) :
  CException(rhs),
  m_description(rhs.m_description)
{}

/*!
   Destructor is null, but provided to complete the chain of 
   virtual destructors back to the base class.
*/
CInvalidInterfaceType::~CInvalidInterfaceType()
{}

/*!
    Assignment.. base class assignment and assign descriptions.
*/
CInvalidInterfaceType&
CInvalidInterfaceType::operator=(const CInvalidInterfaceType& rhs)
{
  if (this != &rhs) {
    CException::operator=(rhs);
    m_description = rhs.m_description;
  }
  return *this;
}
/*!
    Equality compare, compare base class and description strings.
*/
int
CInvalidInterfaceType::operator==(const CInvalidInterfaceType& rhs)
{
  return ((CException::operator==(rhs)           &&
	   m_description == rhs.m_description));
}
/*!
   Inequality is just negation of equality:
*/
int
CInvalidInterfaceType::operator!=(const CInvalidInterfaceType& rhs)
{
  return !(*this == rhs);
}


/*!
  Return the reason for the exception, this is just the description:
*/
const char*
CInvalidInterfaceType::ReasonText() const
{
  return m_description.c_str();
}

/*!
   Reason code is always 0.
*/
int
CInvalidInterfaceType::ReasonCode() const
{
  return 0;
}
