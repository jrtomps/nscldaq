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
#include "CControlModule.h"
#include <CRunState.h>
#include <CControlQueues.h>

using namespace std;

////////////////////////////////////////////////////////////////////////////////////
//
// Constructors and canonicals.
//

/*!
   Constructing a module is essentially a no-op.
   An external entity will need to invoke Attach to attach a 
   CItemConfiguration to this object. That will in turn
   invoke the onAttach function.  We can't do that in the
   constructor (much as I'd like to), since in constructors,
   virtual functions are all confined to the textual class, rather than the
   run-time class, and therefore, the wrong onAttach will get called.

*/
CControlModule::CControlModule(string name) :
  m_name(name)
{}



CControlModule::~CControlModule()
{}

CControlModule::CControlModule(const CControlModule& rhs) :
  CConfigurableObject(rhs)
{}


CControlModule&
CControlModule::operator=(const CControlModule& rhs)
{
  return dynamic_cast<CControlModule&>(CConfigurableObject::operator=(rhs));
}


int
CControlModule::operator==(const CControlModule& rhs) const
{
  return CConfigurableObject::operator==(rhs);
}

int
CControlModule::operator!=(const CControlModule& rhs) const
{
  return !(*this == rhs);
}
/////////////////////////////////////////////////////////////////////////////////
//
//  Virtual functions:

/*! 
   Initialize is called to obtain access to the hardware and
   do any one-time device setup.
   It is optional.  The default implementation here is a no-op.
*/
void
CControlModule::Initialize()
{}


   
/*!
  \return std::string
  \retval name of module.
*/
string
CControlModule::getName() const
{
  return m_name;
}
