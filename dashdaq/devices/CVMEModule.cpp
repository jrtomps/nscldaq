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
#include "CVMEModule.h"
#include <CVMEInterface.h>

///////////////////////////////////////////////////////////////////////////////
//////////////////////// Constructors and canonicals //////////////////////////
///////////////////////////////////////////////////////////////////////////////

/*!
   Constructors and assignments just fill in the appropriate fields.
   \param interface : CVMEInterface& 
      Reference to a VME interface that talks to the crate the module is plugged into
   \param base      : uint32_t
      Base address of the module.

*/
CVMEModule::CVMEModule(CVMEInterface& interface, uint32_t base) :
  m_pInterface(&interface),
  m_base(base)
{}


CVMEModule::CVMEModule(const CVMEModule& rhs) :
  m_pInterface(rhs.m_pInterface),
  m_base(rhs.m_base)
{}


CVMEModule::~CVMEModule()
{}


CVMEModule&
CVMEModule::operator=(const CVMEModule& rhs) 
{
  if (this != &rhs) {
    m_pInterface = rhs.m_pInterface;
    m_base       = rhs.m_base;
  }
  return *this;
}

int
CVMEModule::operator==(const CVMEModule& rhs) const
{
  return ((m_pInterface == rhs.m_pInterface)        &
	  (m_base       == rhs.m_base));

}
int
CVMEModule::operator!=(const CVMEModule& rhs) const
{
  return !(*this == rhs);
}

//////////////////////////////////////////////////////////////////////////////
///////////////////// Selectors  needed by derived classes //////////////////
/////////////////////////////////////////////////////////////////////////////

CVMEInterface&
CVMEModule::getInterface()
{
  return *m_pInterface;
}

uint32_t
CVMEModule::getBase() const
{
  return m_base;
}
