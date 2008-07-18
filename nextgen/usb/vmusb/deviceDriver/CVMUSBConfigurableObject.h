#ifndef __CVMUSBCONFIGURABLEOBJECT_H
#define __CVMUSBCONFIGURABLEOBJECT_h

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

#ifndef  __CCONFIGURABLEOBJECT_H
#include <CConfigurableObject.h>
#endif


class CVMUSB;
class CVMUSBReadoutList;

/*!
   Abstract base class that extends the interface required of 
   configurable objects to what we need from VM-USB configurable objects.
   Specifically we will be adding the member functions that 
   are needed to initialize read out modules and create the stack
   fragment needed to read a module.
*/
class CVMUSBConfigurableObject : public CConfigurableObject
{
  // No construtors/canonicals.  That's only required by derived,
  // concrete classes.

  // The following are in addtion to void OnAttach() which is mandated by the
  // base class, and called just after a brand new configuration item has been
  // attached to us.

  virtual void Initialize(CVMUSB& controller) = 0; //!< init module in accordance with config.
  virtual void addReadoutList(CVMUSBReadoutList& list) = 0; //!< add readout list fragment to list.
  virtual CConfigurableObject* clone() const  = 0;
  
};


#endif
