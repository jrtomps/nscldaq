#ifndef __CINVALIDOBJECTSTATE_H
#define __CINVALIDOBJECTSTATE_H
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

#ifndef __CSOURCEXCEPTION_H
#include "CSourceException.h"
#endif

#ifndef __STL_STRING
#include <string>
#ifndef __STL_STRING
#define __STL_STRING
#endif
#endif




/*!
   An exception that is thrown if an object is in a state that makes a requested
   operation unreasonable or impossible.
*/
class CInvalidObjectState : public CSourceException 
{
private:
  std::string    m_attemptedState;
  std::string    m_currentState;
  mutable std::string m_reasonText;

public:
  // canonicals:

  CInvalidObjectState(std::string attempted,
		      std::string current,
		      const char* pDoing,
		      std::string manager  = std::string(""),
		      std::string instance = std::string(""));

  // Selectors
public:
  std::string attemptedState() const;
  std::string currentState()   const;

  // Interface for CException that we implement:

public:
  virtual int ReasonCode() const;
  virtual const char* ReasonText() const;

};

#endif
