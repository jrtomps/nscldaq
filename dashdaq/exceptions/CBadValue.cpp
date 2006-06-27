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
#include <CBadValue.h>


#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif

CBadValue::CBadValue(const char* legal, const char* got, const char* wasDoing) :
  CException(wasDoing),
  m_allowed(legal),
  m_actual(got)
{}

CBadValue::CBadValue(const CBadValue& rhs) :
  CException(rhs),
  m_allowed(rhs.m_allowed),
  m_actual(rhs.m_actual)
{}

CBadValue::~CBadValue() {}

CBadValue&
CBadValue::operator=(const CBadValue& rhs)
{
  if (this != &rhs) {
    CException::operator=(rhs);
    m_allowed = rhs.m_allowed;
    m_actual  = rhs.m_actual;
  }
  return *this;
}

int 
CBadValue::operator==(const CBadValue& rhs) const
{
  return (CException::operator==(rhs)                &&
	  (m_allowed == rhs.m_allowed)               &&
	  (m_actual  == rhs.m_actual));
}
int
CBadValue::operator!=(const CBadValue& rhs) const
{
  return !(*this == rhs);
}

const char*
CBadValue::ReasonText() const
{
  m_message = "Invalid value : ";
  m_message += m_actual;
  m_message += " valid values are: ";
  m_message += m_allowed;
  m_message += " while : ";
  m_message += WasDoing();
  return m_message.c_str();
}

int 
CBadValue::ReasonCode() const
{
  return 0;
}
