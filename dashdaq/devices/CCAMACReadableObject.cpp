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
#include "CCAMACReadableObject.h"

using namespace std;

/*!
   Constructors just do base class construction:

*/
CCAMACReadableObject::CCAMACReadableObject(CCAMACCrate& crate, size_t slot) :
  CReadableObject(),
  CCAMACModule(crate, slot)
{}

CCAMACReadableObject::CCAMACReadableObject(const CCAMACReadableObject& rhs) :
  CReadableObject(rhs),
  CCAMACModule(rhs)
{}

//! destruction just establishes the virtual nature of destruction.

CCAMACReadableObject::~CCAMACReadableObject()
{}

/*!
  Assignment is again the job of the base class where appropriate:

*/
CCAMACReadableObject&
CCAMACReadableObject::operator=(const CCAMACReadableObject& rhs)
{
  if (this != &rhs) {
    CReadableObject::operator=(rhs);
    CCAMACModule::operator=(rhs);
  }
  return *this;
}
/*! Similarly for equality...inequality as usual is defined in terms of the
     inverse
*/
int
CCAMACReadableObject::operator==(const CCAMACReadableObject& rhs)  const
{
  return (CReadableObject::operator==(rhs)          &&
	  CCAMACModule::operator==(rhs));
}
int
CCAMACReadableObject::operator!=(const CCAMACReadableObject& rhs) const
{
  return !(*this == rhs);
} 
