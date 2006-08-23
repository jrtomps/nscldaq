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
#include "CControlHardware.h"
#include "CControlModule.h"

using std::string;

#ifndef TRUE
#define TRUE -1
#endif

// The canonical are basically a bunch of no-ops.. Any CControlHardware
// is equivalent to any other..let the derived classes sort out subtle differences.
//

CControlHardware::CControlHardware(string name)
{
  m_pConfig = new CControlModule(name, *this);
}
CControlHardware::CControlHardware(const CControlHardware& rhs)
{
  CControlModule* pModule = rhs.m_pConfig;
  m_pConfig = new CControlModule(pModule->getName(), *this);
}
CControlHardware::~CControlHardware()
{
  delete m_pConfig;
}

CControlHardware&
CControlHardware::operator=(const CControlHardware& rhs)
{
  if(this != &rhs) {
    delete m_pConfig;
    m_pConfig = new CControlModule(rhs.m_pConfig->getName(), *this);
  }
  return *this;
}


int
CControlHardware::operator==(const CControlHardware& rhs) const
{
  return TRUE;
}

int 
CControlHardware::operator!=(const CControlHardware& rhs) const
{
  return !(*this == rhs);	// In case operator== becomes meaningful.
}
