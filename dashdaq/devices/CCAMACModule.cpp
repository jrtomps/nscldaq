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
#include "CCAMACModule.h"
#include <CCAMACCrate.h>

/////////////////////////////////////////////////////////////////////////
///////////////////////////// Canonicals ///////////////////////////////
////////////////////////////////////////////////////////////////////////

/*!
   Constructors just initialize the members:
   \param crate : CCAMACCrate&
        References the crate in which the module is installed.
   \param slot : size_t
       Number of the slot in which the module is installed.
*/
CCAMACModule::CCAMACModule(CCAMACCrate& crate, size_t slot) :
  m_pCrate(&crate),
  m_slot(slot)
{

}
/*!
   Copy construction:
*/
CCAMACModule::CCAMACModule(const CCAMACModule& rhs) :
  m_pCrate(rhs.m_pCrate),
  m_slot(rhs.m_slot)
{}
/*!
   Destructor is provided just to support a chain of destructors that
   work polymorphically if needed.
*/
CCAMACModule::~CCAMACModule()
{}

/*!
   Assignment is just member by member copy.
*/
CCAMACModule&
CCAMACModule::operator=(const CCAMACModule& rhs)
{
  if(this != &rhs) {
    m_pCrate = rhs.m_pCrate;
    m_slot   = rhs.m_slot;
  }
  return *this;
}
/*!
   Comparison is valid if the crate pointers are equal as are the
   slot numbers.
*/
int
CCAMACModule::operator==(const CCAMACModule& rhs) const
{
  return ((m_pCrate   == rhs.m_pCrate)           &&
	  (m_slot     == rhs.m_slot));
}
/*!
   inequality is done in the usual way
*/
int
CCAMACModule::operator!=(const CCAMACModule& rhs) const
{
  return !(*this == rhs);
}

///////////////////////////////////////////////////////////////////////////////
////////////////////////////// selectors //////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

CCAMACCrate&
CCAMACModule::getCrate() 
{
  return *m_pCrate;
}
size_t
CCAMACModule::getSlot() const
{
  return m_slot;
}
