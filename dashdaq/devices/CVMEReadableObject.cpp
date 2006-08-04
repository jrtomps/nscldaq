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
#include "CVMEReadableObject.h"
#include <CVMEInterface.h>


using namespace std;

/*!
    Constructors just invoke the base class constructors.
*/
CVMEReadableObject::CVMEReadableObject(CVMEInterface& interface, uint32_t base) :
  CReadableObject(),
  CVMEModule(interface, base)
{}

CVMEReadableObject::CVMEReadableObject(const CVMEReadableObject& rhs) :
  CReadableObject(rhs),
  CVMEModule(rhs)
{}

CVMEReadableObject::~CVMEReadableObject()
{}



CVMEReadableObject&
CVMEReadableObject::operator=(const CVMEReadableObject& rhs)
{
  if (this != &rhs) {
    CReadableObject::operator=(rhs);
    CVMEModule::operator=(rhs);
  }
  return *this;
}

int
CVMEReadableObject::operator==(const CVMEReadableObject& rhs) const
{
  return (CReadableObject::operator==(rhs)          &&
	  CVMEModule::operator==(rhs));
}
int 
CVMEReadableObject::operator!=(const CVMEReadableObject& rhs) const
{
  return !(*this == rhs);

}
