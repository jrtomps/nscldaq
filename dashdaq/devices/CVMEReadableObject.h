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

#ifndef __CVMEREADABLEOBJECT_H
#define __CVMEREADABLEOBJECT_H


#ifndef __CREADABLEOBJECT_H
#include "CReadableObject.h"
#endif

#ifndef __CVMEMODULE_H
#include "CVMEModule.h"
#endif

// Forward data types:

class CVMEList;

/*!
   This class is the base class for all VME bus resident modules that
   can respond to a trigger (be read out).  Beware, multiple inheritance
   is at work here as this is both a Readable Object and a VME module.
   Since both base classes are not themselves derived from anything
   we should be able to avoid the classical diamond of multiple inheritance
   death.
     
   We will add a method specification of our own,  the ability to add
   our read out to a VME list so that readable objects can be used
   in conjuction with list processing interfaces.  Note that
   for modules that, for some reason, cannot function in this way,
   simply implement addReadoutList to throw an exception e.g.
   CDeviceIncapable would be a dandy choice.

*/
class CVMEReadableObject :  public CReadableObject, public CVMEModule
{
public:
  CVMEReadableObject(CVMEInterface& interface, uint32_t base);
  CVMEReadableObject(const CVMEReadableObject& rhs);
  virtual ~CVMEReadableObject();

  CVMEReadableObject& operator=(const CVMEReadableObject& rhs);
  int operator==(const CVMEReadableObject& rhs) const;
  int operator!=(const CVMEReadableObject& rhs) const;

  // pure virtual members:

  virtual CVMEList& addReadoutToList(CVMEList& list)  = 0;
};


#endif
