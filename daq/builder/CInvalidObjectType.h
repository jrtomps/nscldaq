#ifndef __CINVALIDOBJECTTYPE_H
#define __CINVALIDOBJECTTYPE_h
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

// Forward classes:

class CFragmentSource;

/*!
    This class reports an invalid object type.  This is typically thrown by
    a driver manager when it is asked to operate on a driver that it is not
    managing.  Note that this implies an expectation that all driver managers
    will maintain a record of the set of driver instances they are currently managing.
*/
class CInvalidObjectType  : public CSourceException
{
private:
  CFragmentSource*     m_pSource;
  mutable std::string  m_reasonText;

public:
  // Constructors and other canonical functions:

  CInvalidObjectType(CFragmentSource* pSource,
		     const char*      pDoing,
		     std::string      manager  = std::string(""),
		     std::string      instance = std::string(""));
  CInvalidObjectType(const CInvalidObjectType& rhs);
  CInvalidObjectType& operator=(const CInvalidObjectType& rhs);
  int operator==(const CInvalidObjectType& rhs) const;
  int operator!=(const CInvalidObjectType& rhs) const;

  
  // Implementation of the pure virtual interface of the ultimate base class:

  virtual int ReasonCode() const;
  virtual const char* ReasonText() const;

};

#endif
