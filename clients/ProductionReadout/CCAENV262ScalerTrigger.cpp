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

#include "CCAENV262ScalerTrigger.h"
#include <CaenIO.h>

/*!
    Construction consists of creating a new module that will
    be used for the trigger.
    \param base  : unsigned long
        base address of the module in VME space.  This is set on the module
	switches.
    \param vmeCrate : unsigned long = 0
       Number of the VME crate in which the module is installed.
*/
CCAENV262ScalerTrigger::CCAENV262ScalerTrigger(unsigned long base,
					       unsigned long vmeCrate) :
  m_pIO(new CCaenIO(base, vmeCrate))
{
  
}
/*!
   Destruction requires the deletion of the module.
*/
CCAENV262ScalerTrigger::~CCAENV262ScalerTrigger()
{
  delete m_pIO;
}
  
/*!
    Check for triggers.
    \return bool
    \retval true  - Device triggered.
    \retval false - No trigger detected.
*/
bool
CCAENV262ScalerTrigger::operator()()
{
  if (m_pIO->ReadInput(0)) {
    m_pIO->PulseOutput(2);	// Allow user to lift the latched input.
    return true;
  }
  else {
    return false;
  }
}
/*!
   Initialize by requesting the external latch to clear
*/
void
CCAENV262ScalerTrigger::Initialize()
{
  m_pIO->PulseOutput(2);
}
