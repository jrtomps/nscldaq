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
#include "CBiRA3251.h"
#include <CCAMACCrate.h>

/*!
   Construct the module:
   \param crate : CCAMACCrate& 
      Reference to the crate in which this module is installed.
   \param slot  : size_t
      Number of the slot in which the module was installed.
*/
CBiRA3251::CBiRA3251(CCAMACCrate& crate, size_t slot) :
  CCAMACModule(crate, slot)
{}

/*!
   Copy constructor we don't have member data so just invoke the parent
   class's copy constructor.
*/
CBiRA3251::CBiRA3251(const CBiRA3251& rhs) :
  CCAMACModule(rhs)
{}
/*!
  Assignment...again, let the base class deal with it.
*/
CBiRA3251&
CBiRA3251::operator=(const CBiRA3251& rhs)
{
  CCAMACModule::operator=(rhs);
  return *this;
}
/*
  Comparison delegate as well:
*/
int
CBiRA3251::operator==(const CBiRA3251& rhs) const
{
  return CCAMACModule::operator==(rhs);
}
int
CBiRA3251::operator!=(const CBiRA3251& rhs) const
{
  return CCAMACModule::operator!=(rhs);
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////  module operations ////////////////////////
//////////////////////////////////////////////////////////////////////////

void
CBiRA3251::write(uint16_t mask)
{
  CCAMACCrate& crate(getCrate());
  size_t       slot = getSlot();

  crate.write16(slot, 16, 0, mask);
}
