/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2009.

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
#include <CNoSuchConfigurationParameter.h>


/*!
    Construction:
    @param paramName  - The parameter that is not known to the object the code tried to configure.
    @param pDoing     - Context string.
    @param manager    - Name of manager for the object that was being configured.
    @param instance   - Name of the instance that was being configured.

*/
CNoSuchConfigurationParameter::CNoSuchConfigurationParameter(std::string paramName, const char* pDoing, 
							     std::string manager = std::string(""),
							     std::string instance= std::string("")) : 
  CSourceException(pDoing, manager, instance),
  m_parameterName(paramName)
{}

/*!
   Copy construction:
*/

CNoSuchConfigurationParameter::CNoSuchConfigurationParameter(const CNoSuchConfigurationParameter& rhs) :
  CSOurceException(rhs),
  m_parameterName(rhs.m_parameterName)
{}

/*!
   Assignment:
*/
CNoSuchConfigurationParameter&
CNoSuchConfigurationParameter::operator=(const CNoSuchConfigurationParameter& rhs)
{
  if (this != &rhs) {
    CSourceException::operator=(rhs);
    m_parameterName = rhs.m_parameterName;
  }
  return *this;
}

/*!
  Equality comparison:
*/
int 
CNoSuchConfigurationParameter::operator==(const CNoSuchConfigurationParameter& rhs) const
{
  return (CSourceException::operator==(rhs)                   &&
	  (m_parameterName == rhs.m_parameterName));
}
int 
CNoSuchConfigurationParameter::operator!=(const CNoSuchConfigurationParameter& rhs) const
{
  return !(*this == rhs);
}


/*!
   Return the name of the parameter the user tried to use:
*/
std::string 
CNoSuchConfigurationParameter::getParameterName() const
{
  return m_parameterName;
}

/*!
   Provide the reason for the exception as a code:
   \return int
   \retval (int)INVALID_CONFIG_PARAM_NAME
*/
int 
CNoSuchConfigurationParameter::ReasonCode() const
{
  return static_cast<int>(CBuilderConstant::INVALID_CONFIG_PARAM_NAME);

}
/*!
    Provides a human readable description of the error:
*/
const char* 
CNoSuchConfigurationParameter::ReasonText() const
{
  m_reasonText     = "The configuration parameter ";
  m_reasonText    += m_parameterName;
  m_reasonText    += " is not a valid configuration parameter\n";
  m_reasonText    += messageTrailer();

  return m_reasonText.c_str();
}
