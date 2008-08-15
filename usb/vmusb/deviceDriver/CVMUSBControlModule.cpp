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
#include "CVMUSBControlModule.h"
#include <CVMUSB.h>
#include <assert.h>

using  std::string;


CVMUSB* CVMUSBControlModule::m_pController(0);

/*!
  Constructors just chain to the base class as there's no object local data.
*/

CVMUSBControlModule::CVMUSBControlModule(string name) :
  CControlModule(name)
{}

CVMUSBControlModule::CVMUSBControlModule(const CVMUSBControlModule& rhs) :
  CControlModule(rhs)
{}

CVMUSBControlModule&
CVMUSBControlModule::operator=(const CVMUSBControlModule& rhs)
{
  CControlModule::operator=(rhs);
  return *this;
}


int
CVMUSBControlModule::operator==(const CVMUSBControlModule& rhs) const
{
  return CControlModule::operator==(rhs);
}
int 
CVMUSBControlModule::operator!=(const CVMUSBControlModule& rhs) const
{
  return !(*this == rhs);
}

/*!
   Set a new value for the controller pointer.
   \param pController - Pointer to the VM-USB controller.
*/
void
CVMUSBControlModule::setController(CVMUSB* pController)
{
  m_pController = pController;
}

///////////////////////////////////////////////////////////////////////////
//
// Facade operations... all of them assert the non-nullness of the
// CVMUSB pointer.
//

/*!
  Initialize the module.  The module is supposed to do anythning needed
  to gain initial access.  In practice this is often not needed,
  so we'll implement a default no-op as well for the target operation.
*/
void
CVMUSBControlModule::Initialize()
{
  assert(m_pController);

  Initialize(*m_pController);
}

/*!
   Update the hardware from any internal image the driver is holding; or vica 
   versa according to the needs/desired of the driver software.
*/
string
CVMUSBControlModule::Update()
{
  assert(m_pController);
  return Update(*m_pController);
}

/*!
   Set a controlled parameter to a new value:
   \param what      - Name of the parameter to set.  Each driver chooses its own set of names.
   \param value     - New value for the parameter.  This is a string so that the driver
                      can choose how to interpret it
   \return std::string
   \retval Some string returned from the driver.  The string must start with the text ERROR
                     if there is a problem.  It should be OK if successful.
	      
*/
string
CVMUSBControlModule::Set(const char* what, 
			 const char* value)
{
  assert(m_pController);
  return Set(*m_pController, what, value);
}
/*!
   Get a value from a control parameter
   \param what - the name of the parameter being queried.
   \return string
   \retval The value.. If the string begins with the text "ERROR"  This is an error.
*/
string
CVMUSBControlModule::Get(const char* what)
{
  assert(m_pController);
  return Get(*m_pController, what);
}

