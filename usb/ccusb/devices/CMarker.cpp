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

/**
 * @file CMarker.cpp
 * @brief Implementation of the marker driver : insert a constant uint16_t in the buffer.
 * @author Ron Fox <fox@nscl.msu.edu>
 */

#include "CMarker.h"

#include "CReadoutModule.h"
#include <CCCUSB.h>
#include <CCCUSBReadoutList.h>

#include <tcl.h>

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include <string>
#include <set>

#include <iostream>

/**
 * Local consts
 */
static CConfigurableObject::limit Zero(0);
static CConfigurableObject::limit MaxUint16(0xffff);

static CConfigurableObject::Limits valueLimits(Zero, MaxUint16);

/*-----------------------------------------------------------------------------
 * Implementations of canonicals- note m_pConfiguration is a base class protected
 * member.
 */

/**
 * default constructor:
 */
CMarker::CMarker()

{
  m_pConfiguration = 0;		// Can't use an initializer for a base class.
}
/**
 * copy constructor
 *
 * @param rhs - The object that will be cloned into this.
 */
CMarker::CMarker(const CMarker& rhs) 
{
  m_pConfiguration = 0;                    // Someone else manages storage.
  if(rhs.m_pConfiguration) {
    m_pConfiguration = new CReadoutModule(*(rhs.m_pConfiguration));
  }
}
/**
 * destuctor
 */
CMarker::~CMarker()
{
  // Comoone else manages the m_pConfiguration storage.
}

/**
 * Assignments should probably be illegal:
 *
 * @param rhs - The item being assigned to this.
 * @return *this
 */
CMarker&
CMarker::operator=(const CMarker& rhs)
{
  if (&rhs != this) {
    m_pConfiguration = 0;
    if (rhs.m_pConfiguration) {
      m_pConfiguration = new CReadoutModule(*(rhs.m_pConfiguration));
    }
  }
  return *this;
}

/*-----------------------------------------------------------------------
 * Implementing the driver interface:
 */

/**
 * onAttach
 *   The configuration is attached to this object and options
 *   are added to it.
 *
 * @param config - References the configuration
 */
void
CMarker::onAttach(CReadoutModule& configuration)
{
  m_pConfiguration = &configuration;
  configuration.addParameter("-value", CConfigurableObject::isInteger, &valueLimits);


}
/**
 * Initialize:
 *   No-op as there's no hardware to initialize.
 */
void
CMarker::Initialize(CCCUSB& controller) {}

/**
 * addReadoutList
 *   Add the marker element to the readoutlist.
 *
 * @param list - the readout list for this stack.
 */
void
CMarker::addReadoutList(CCCUSBReadoutList& list)
{
  uint16_t value = getIntegerParameter("-value");
  list.addMarker(value);
}
/**
 * clone
 *   Virtual constructor.
 *
 * @return copy of this.
 */
CReadoutHardware*
CMarker::clone() const
{
    return new CMarker(*this);
}

