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
#include <CInvalidConfigurationValue.h>

/*!
   Construct the exception:
   @param parameterName   - Name of the legal parameter.
   @param parameterValue  - Illegal value for that parameter.
   @pDoing                - Context string
   @manager               - Name of driver manager involved.
   @instance              - Name of driver instance involved.

*/
CInvalidConfigurationValue::CInvalidConfigurationValue(std::string parameterName,
						       std::string parameterValue,
						       const char* pDoing,
						       std::string manager = std::string(""),
						       std::string instance = std::string("")) :
  CSourceException(pDoing, manager, instance),
  m_name(parameterName),
  m_value(parameterValue)
{}

/*!
   Copy construction:

*/
CInvalidConfigurationValue::CInvalidConfigurationValue(const CInvalidConfigurationValue& rhs) :
  CSourceException(rhs),
  m_name(rhs.m_name),
  m_value(rhs.m_value)
{
}

/*!
   Object assignment:
*/
CInvalidConfigurationValue& 
CInvalidConfigurationValue::operator=(const CInvalidConfigurationValue& rhs)
{
  if (this != &rhs) {
    CSourceException::operator=(rhs);
    m_name  = rhs.m_name;
    m_value = rhs.m_value;

  }
  return *this;
}

/*!
   Equality test:
*/
int 
CInvalidConfigurationValue::operator==(const CInvalidConfigurationValue& rhs) const
{
  return (CSourceException::operator==(rhs)                 &&
	  (m_name   == rhs.m_name)                           &&
	  (m_value  == rhs.m_value));
}
/*!
  Inverse of equality test:
*/
int
CInvalidConfigurationValue::operator!=(const CInvalidConfigurationValue& rhs) const
{
  return !(*this == rhs);
}

/*!
    @return std::string
    @retval Name of the parameter that is involved
*/
std::string 
CInvalidConfigurationValue::getParameterName() const
{
  return m_name;
}
/*!
  @return std::string
  @retval proposed value for the parameter that failed validation.
*/

std::string 
CInvalidConfigurationValue::getProposedValue() const
{
  return m_name;
}

/*!
   Return the reason code that identifies this exception
   \return int
   \retval (int)INVALID_CONFIG_PARAM_VALUE
*/
int
CInvalidConfigurationValue:: ReasonCode() const
{
  return static_cast<int>(CBuilderConstant::INVALID_CONFIG_PARAM_VALUE);
}
/*!
   Return a human readable string that describes the error.
*/
 const char* 
CInvalidConfigurationValue::ReasonText() const
 {
   m_reason   =  "The value '";
   m_reason  += m_value;
   m_reason  += "' was proposed for configuration parameter '";
   m_reason  += m_name;
   m_reason  += " but failed validation checks\n";
   m_reason  += messageTrailer();

   return m_reason.c_str();
 }
