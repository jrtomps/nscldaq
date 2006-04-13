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
#include "CSBSVmeException.h"

#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif

/*!
   Construct an exception.
   \param error : bt_error_t [in]
      Error returned by the bt_xxx function.
   \param wasDoing : std::string [in]
      Some context about what was going on when the error was detected.
*/
CSBSVmeException::CSBSVmeException(bt_error_t error,
				   string wasDoing) :
  CException(wasDoing),
  m_errorCode(error)
{}
/*!
   Copy construction. The errorString does not need to be copied as that's
   generated each ReasonText() call.
*/
CSBSVmeException::CSBSVmeException(const CSBSVmeException& rhs) :
  CException(rhs),
  m_errorCode(rhs.m_errorCode)
{
}
/*!  Destruction is a noop.
 */
CSBSVmeException::~CSBSVmeException()
{}

/*!
    Assignment, like copy construction, no need to 
    keep the error string.
*/
CSBSVmeException&
CSBSVmeException::operator=(const CSBSVmeException& rhs)
{
  if (&rhs != this) {
    CException::operator=(rhs);
    m_errorCode = rhs.m_errorCode;
  }
  return *this;
}
/*!
   Equality compare.. only compare the error code and base class.
*/
int
CSBSVmeException::operator==(const CSBSVmeException& rhs)
{
  return (CException::operator==(rhs)         &&
	  (m_errorCode == rhs.m_errorCode));
}
/*!
  Inequality is just the negation of equality.
*/
int
CSBSVmeException::operator!=(const CSBSVmeException& rhs)
{
  return !(*this == rhs);
}

/*!
     ReasonText - generate the reason for the failure.
     this will be the output of bt_strerror().
*/
const char*
CSBSVmeException::ReasonText() const
{
  char reasonBuffer[1000];
  bt_strerror((bt_desc_t)NULL, m_errorCode,
	      "", reasonBuffer, sizeof(reasonBuffer));

  // Can't return reason buffer as that will go out of scope so:

  m_errorString = reasonBuffer;
  return m_errorString.c_str();
}
/*!
    Return the reason code for the error.
*/
int
CSBSVmeException::ReasonCode() const
{
  return m_errorCode;
}
