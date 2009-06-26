#ifndef __CNOSUCHMANAGER_H
#define __CNOSUCHMANAGER_H



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
  
  This exception is thrown when when the software attempts to manipulate a 
driver manager that does not exist.

*/
class CNoSuchManager : public CSourceException
{
  // Member storage:
  
  mutable std::string m_ReasonText;
  // canonicals:

public:
  CNoSuchManager(std::string manager,
		 const char* pDoing);

  // CException interface implemented in this clas.
public:
  virtual int ReasonCode() const;
  virtual const char* ReasonText() const;
}

#endif
f
