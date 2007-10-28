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
#include "CCAENChain.h"
#include <CReadoutModule.h>
#include <CVMUSB.h>
#include <CVMUSBReadoutList.h>
#include <C785.h>
#include <CConfiguration.h>

/////////////////////////////////////////////////////////////////
/////////////// Canonical class/object implementations /////////
////////////////////////////////////////////////////////////////

/*!
  Construction is largely a no-op as all the action happens
  at attach time.
*/
CCAENChain::CCAENChain() :
  m_pConfiguration(0)
{
}
/*!
  Copy construction requires duplication of the configuration as
  well as the list.
*/
CCAENChain::CCAENChain(const CCAENChain& rhs) :
  m_pConfiguration(0)
{
  if(rhs.m_pConfiguration) {
    m_pConfiguration = new CReadoutModule(*(rhs.m_pConfiguration));
  }
  m_Chain = rhs.m_Chain; 
}
/*! Destroy the module.  This is pretty much a no-op as configurations
    take care of their own destruction:
*/
CCAENChain::~CCAENChain()
{
}

/*!
   Assignment for now is not really good
*/
CCAENChain&
CCAENChain::operator=(const CCAENChain& rhs)
{
  return *this;
}

///////////////////////////////////////////////////////////////////
///////////// overridable object operations ///////////////////////
///////////////////////////////////////////////////////////////////

/*!
   Called to attach the configuration object to us.
   We have to define the following parameters, none of which have defaults:
   -base, -modules.  The -base checker will just be the standard list checker,
   while we will supply a custom checker for the module list, that will
   ensure the module list is a valid list of strings, and that each item
   in the list identifies a module that is a C785 object, as only those
   are allowed to be in a chain.
*/
void
CCAENChain::onAttach(CReadoutModule& configuration)
{
  m_pConfiguration = &configuration;

  m_pConfiguration->addParameter("-base", CConfigurableObject::isInteger,
				 NULL,"0");

  m_pConfiguration->addParameter("-modules", CCAENChain::moduleChecker,
				 NULL, "");
}
