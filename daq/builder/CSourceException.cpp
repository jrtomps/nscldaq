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
#include <CSourceException.h>


using namespace std;


/*!
  Constructs an exception

  @param pDoing   - Context (what was happening) of the exception,
  @param manager  - Name of the input driver manager involved.
  @param instance - Name of the driver instance involved.

  @note The manager and/or instance may be empty strings if this info
        is not known.

*/
CSourceException::CSourceException(const char* pDoing, 
				   std::string manager = string(""),
				   std::string instance = string("")) :
  CException(pDoing),
  m_managerName(manager),
  m_instanceName(instance)
{}

/*!
  Copy construction:

  @param rhs  - The item being copied into this object.
*/
CSourceException::CSourceException(const CSourceException& rhs) :
  CException(rhs),
  m_managerName(rhs.m_managerName),
  m_instanceName(rhs.m_instanceName)
{}


/*!
    Assignment:

*/
CSourceException::CSourceException& operator=(const CSourceException& rhs)
{
  if(&rhs != this) {
    CException::operator=(rhs);
    m_managerName  = rhs.m_managerName;
    m_instanceName = rhs.m_instanceName;
  }
  return *this;
}
/*!
   Equality comparison.
*/
int
CSourceException::operator==(const CSourceException& rhs)
{
  return (CException::operator==(rhs)                    &&
	  (m_managerName  == rhs.m_managerName)          &&
	  (m_instanceName == rhs.m_instanceName));
}
/*!
    Logical inverse of equality:
*/
int
CSourceException::operator==(const CSourceException& rhs)
{
  return !(*this == rhs);
}

/*!
  Return the name of the manager.  This could be a blank string if the
  manager is not known or inappropriate for this exception
*/
std::string 
CSourceException::getManager() const
{
  return m_managerName;
}y
