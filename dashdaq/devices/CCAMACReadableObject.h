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

#ifndef __CCAMACREADABLEOBJECT_H
#define __CCAMACREADABLEOBJECT_H

#ifndef __CCAMACMODULE_H
#include "CCAMACModule.h"
#endif

#ifndef __CREADABLEOBJECT_h
#include "CReadableObject.h"
#endif

/*!
    This class establishes a common base class for CAMAC modules that will
    be read out in response to a trigger.  There is multiple inheritance afoot
    here but the neither base class is subclassed from any other class so it
    should be safe.
*/
class CCAMACReadableObject : public CReadableObject, public CCAMACModule
{
  // Need constructors to be able to properly construct the CCAMACModule
  // base class
  //
public:
  CCAMACReadableObject(CCAMACCrate& crate, size_t slot);
  CCAMACReadableObject(const CCAMACReadableObject& rhs);
  virtual ~CCAMACReadableObject();

  CCAMACReadableObject& operator=(const CCAMACReadableObject& rhs);
  int operator==(const CCAMACReadableObject& rhs) const;
  int operator!=(const CCAMACReadableObject& rhs) const;
};


#endif
