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


///////////////////////////////////////////////////////////
//  CCAENV977Trigger.cpp
//  Implementation of the Class CCAENV977Trigger
//  Created on:      07-Jun-2005 04:42:55 PM
//  Original author: Ron Fox
///////////////////////////////////////////////////////////
#include <config.h>
#include "CCAENV977Trigger.h"
#include "CCAENV977.h"

#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif

// Bits in the control register:

const UShort_t CR_PATTERN(CCAENV977::control_Pattern);
const UShort_t CR_GATEMASK(CCAENV977::control_gateMask);
const UShort_t CR_ORMASK(CCAENV977::control_OrMask);


/*!
  Construct a trigger given the location of the module.
*/
CCAENV977Trigger::CCAENV977Trigger(ULong_t lBase, UShort_t nCrate) :
  m_Module(*(new CCAENV977(lBase, nCrate)))
{
  SetupModule();
}


/*!
  Construct a trigger given an existing module.
*/
CCAENV977Trigger::CCAENV977Trigger(CCAENV977& module) :
  m_Module(*(new CCAENV977(module)))
{
  SetupModule();
}
/*!
   Copy Construction.
*/
CCAENV977Trigger::CCAENV977Trigger(const CCAENV977Trigger& rhs) :
  m_Module(*(new CCAENV977(rhs.m_Module)))
{
  SetupModule();
}
/*!
  Destructor must delete the module object
*/
CCAENV977Trigger::~CCAENV977Trigger()
{
  delete &m_Module;
}
/*!
   Assignment
*/
CCAENV977Trigger&
CCAENV977Trigger::operator=(const CCAENV977Trigger& rhs)
{
  if (this != &rhs) {
    m_Module = rhs.m_Module;
  }
  return *this;
}
/*!
   Equality comparison.
*/
int
CCAENV977Trigger::operator==(const CCAENV977Trigger& rhs) const
{
  return (m_Module == rhs.m_Module);
}
/*!
  Inequality test.
*/
int
CCAENV977Trigger::operator!=(const CCAENV977Trigger& rhs) const
{
  return !(*this == rhs);
}


/**
 * True if a trigger is pending.... this is true if any bits in the 
 * single hit register are non zero.  We'll use the single hit read and clear
 * register to determine this so that we don't have to bother with
 * clearing later.  Note that the pattern is stored in m_LastPattern in case
 * the user wants it to determine how to branch out later.
 *
 */
bool
CCAENV977Trigger::operator()()
{
  m_LastPattern = m_Module.singleHitReadAndClear();
  m_Module.inputSet(m_LastPattern & 0x8000); // So top bit can be used for other stuf.
  return (m_LastPattern != 0);
}

/*!
   Return the most recent trigger pattern.  Note that if a trigger
   poll is done with no trigger present, this will be zero.
*/
UShort_t
CCAENV977Trigger::getTriggerPattern() const
{
  return m_LastPattern;
}

// Utility to set the module in Pattern mode.

void
CCAENV977Trigger::SetupModule() 
{
  m_Module.Reset();
  m_Module.controlRegister(CR_PATTERN);
  m_Module.outputMask(0xffff);	// Only output as programmed.
  m_Module.singleHitReadAndClear();
  m_Module.multiHitReadAndClear();
  m_Module.outputClear();

}
