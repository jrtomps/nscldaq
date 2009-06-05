#ifndef __CINVALIDCONFIGURATIONVALUE_H
#define __CINVALIDCONFIGURATIONVALUE_H

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


#ifndef __CSOURCEEXCEPTION_H
#include "CSourceException.h"
#endif

#ifndef __STL_STRING
#include <string>
#ifndef __STL_STRING
#define __STL_STRING
#endif
#endif


/*!
  This exception is thrown when the client software attempts to provide an existing
  configuration parameter a value that does not meet the constraints of that parameter.
*/
class CInvalidConfigurationValue : public CSourceException
{
private:
  std::string         m_name;
  std::string         m_value;
  mutable std::string m_reason
public:
  // constructors and other canonicals.

  CInvalidConfigurationValue(std::string parameterName,
			     std::string parameterValue,
			     const char* pDoing,
			     std::string manager = std::string(""),
			     std::string instance = std::string(""));
  CInvalidConfigurationValue(const CInvalidConfigurationValue& rhs);
  
  CInvalidConfigurationValue& operator=(const CInvalidConfigurationValue& rhs);
  int operator==(const CInvalidConfigurationValue& rhs) const;
  int operator!=(const CInvalidConfigurationValue& rhs) const;

  // seletors:

  std::string getParameterName() const;
  std::string getProposedValue() const;

  // Functions that are pure virtual in CException:

  virtual int ReasonCode() const;
  virtual const char* ReasonText() const;

			     
};


#endif
