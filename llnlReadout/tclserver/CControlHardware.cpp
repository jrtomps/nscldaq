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


/*!
  Default initialize is a no-op.
*/
void 
CControlHardware::Initialize(CVMUSB& vme)
{}

/*------------------------------------- Default implementations of virtual functions -*/

/**
 ** addMonitorList is supposed to add elements to a list of VME operations that will be
 ** periodically performed to monitor device status.  As many/most devices will not
 ** need to do this, the default implementation adds nothing to the list.
 ** @param vmeList - References the vmusb list to append to.  This is passed by reference
 **                  so that it can be modified by this method.
 */
void
CControlHardware::addMonitorList(CVMUSBReadoutList& vmeList)
{
}

/** processMonitorList is called whenever data is available from a monitor list.
 ** the device driver is expected to fish its data out of the buffer and return
 ** a pointer to the next unhandled part of the list.
 ** As many device drivers do not require monitor data, the default 
 ** for this method consumes no data.
 **
 ** @param pData     - Pointer to the first unconsumed chunk of data from the monitor list.
 ** @param remaining - Number of unprocessed bytes in the monitor list.
 ** @return void*
 ** @retval Pointer to the next unconsumed byte in the list after the data we processed.
 */ 
void* 
CControlHardware::processMonitorList(void* pData, size_t remaining)
{
  return pData;
}
