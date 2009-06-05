#ifndef __CNOSUCHCONFIGURATIONPARAMETER_H
#define __CNOSUCHCONFIGURATIONPARAMETER_H

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
   This class reports an attempt to use an invalid configuration parameter for a data source.
   See also CInvalidConfigurationValue which reports validation failures for a configuration
*/
class CNoSuchConfigurationParameter : public CSourceException
{
private:
  std:::string        m_parameterName;
  mutable std::string m_reasonText;

public:
  // Canonicals:

  CNoSuchConfigurationParameter(std::string paramName, const char* pDoing, 
				std::string manager = std::string(""),
				std::string instance= std::string(""));
  CNoSuchConfigurationParameter(const CNoSuchConfigurationParameter& rhs);
  CNoSuchConfigurationParameter& operator=(const CNoSuchConfigurationParameter& rhs);
  int operator==(const CNoSuchConfigurationParameter& rhs) const;
  int operator!=(const CNoSuchConfigurationParameter& rhs) const;

  // Selectors:

  std::string getParameterName() const;


  // Implementing the CException pure virtuals:

  virtual int ReasonCode() const;
  virtual const char* ReasonText() const;
};


#endif
