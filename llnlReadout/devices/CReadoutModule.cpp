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
#include "CReadoutModule.h"
#include "CReadoutHardware.h"


#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif


///////////////////////////////////////////////////////////////////////////
//////////////////// Constructors and other Canonicals ////////////////////
///////////////////////////////////////////////////////////////////////////

/*!
    Constructing is just a matter of saving the hardware reference
    and invoking its onAttach member.
    \param hardware : CReadoutHardware& 
        Reference to a readout hardware object that will be 
        copy constructed to form our hardware.  This copy construction
        avoids scope/ownership/deletion issues... we own it and we 
        \em will delete it on destruction.
*/
CReadoutModule::CReadoutModule(const CReadoutHardware& hardware) 
  m_pHardware((new CReadoutHardware(hardware)))
{
  m_pHardware->onAttach(*this);
}
/*!
   Copy construction.. just need to 
   - copy construct a new hardware object.
   - invoke its onAttach member.
*/
CReadoutModule::CReadoutModule(const CReadoutModule& rhs) :
  m_pHardware(new CReadoutHardware(*(rhs.m_pHardware)))
{
  m_pHardware->onAttach(*this);
}
/*!
   Destruction: Just destroy our hardware object
*/
CReadoutModule::~CReadoutModule()
{
  delete m_pHardware;
}
/*!
   Assignment is much like copy construction.
*/
CReadoutModule&
CReadoutModule::operator=(const CReadoutModule& rhs)
{
  if (this != &rhs) {
    delete m_pHardware;
    m_pHardware = new *(rhs.m_pHardware);
  }
  return *this;
}
/*!
   Two CReadoutModule objects are equal if their configurations are
   equal and the hardware objects are equal.
*/
int
CReadoutModule::operator==(const CReadoutModule& rhs) const
{
  return ((CConfigurableObject::operator==(rhs))       &&
	  (*m_pHardware  == *(rhs.m_pHardware)));
}
/*!
   Two ReadoutModule objects are unequal if they are not equal.
   This sounds trite but it is not necessarily the case with c++
*/
int
CReadoutModule::operator!=(const CReadoutModule& rhs) const
{
  return !(*this == rhs);
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////   Selectors   /////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

/*!
   Return a pointer to a dynamcially allocated copy of our hardware.
   The caller is responsible for deleting this when done.
*/
CReaoutHardware*
CReadoutModule::getHardwareCopy() const
{
  return new (*m_pHardware);
}

////////////////////////////////////////////////////////////////////////////
//////////////////// Operations on the object //////////////////////////////
////////////////////////////////////////////////////////////////////////////

/*!
   Initialize the module. This is supposed to make the hardware object
   ready to take data.   All that has to be done is to delegate this work
   to the object itself.  This is part of the facade that the CReadoutModule
   object maintains for its CReadoutModule object.
   \param controller : CVMUSB&
      A VMUSB controller that is connected to the VME bus in which the
      hardware resides.
*/
void
CReadoutModule::Initialize(CVMUSB& controller) 
{
  m_pHardware->Initialize(controller);
}
/*!
   Add the module's event readout requirements to the readout list.
   this is part of the facade that CReadoutModule's maintain for their
   CReadoutHardware objects. 
   \param list : CVMUSBReadoutList&
        Reference to the list object that is being built up to create the
	readout list that will be loaded into the VM USB.
*/
void
CReadoutModule::addReadoutList(CVMUSBReadoutList& list)
{
  m_pHardware->addReadoutList(list);
}
