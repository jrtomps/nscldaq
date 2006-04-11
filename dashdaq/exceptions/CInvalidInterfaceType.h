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

/*!
    Defines the CInvalidInterfaceType exception class. This is
    typically thrown when one tries to create a VME interface
    and add it to the VME subsystem via a description line and
    the interface type does not exist.
*/
#ifndef __CINVALIDINTERFACETYPE_H
#define __CINVALIDINTERFACETYPE_H

#ifndef __EXCEPTION_H
#include <Exception.h>		/* Base class */
#endif


#ifndef __STL_STRING
#include <string>
#ifndef __STL_STRING
#define __STL_STRING
#endif
#endif

class CInvalidInterfaceType : public CException
{
private:
  STD(string)    m_description;
public:
  CInvalidInterfaceType(STD(string) description, 
			STD(string) action);
  CInvalidInterfaceType(const CInvalidInterfaceType& rhs);
  virtual ~CInvalidInterfaceType();
  
  CInvalidInterfaceType& operator=(const CInvalidInterfaceType& rhs);
  int operator==(const CInvalidInterfaceType& rhs);
  int operator!=(const CInvalidInterfaceType& rhs);

  // 

public:
  virtual const char* ReasonText() const;
  virtual int         ReasonCode() const;

  
};


#endif
