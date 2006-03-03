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
#include "CCAENV977ScalerTrigger.h"
#include <CCAENV977.h>

#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif

// Local definitions:

static const int myBit(0x8000);

/*!
   Creation requires creation of the underlying module.
   \param base  : unsigned long
       base address of the module (set by switches).
   \param vmeCrate : unsigned int = 0
       Number of the VME crate the module is inserted in.
*/
CCAENV977ScalerTrigger::CCAENV977ScalerTrigger(unsigned long base, 
					       unsigned int vmeCrate) : 
  m_pModule(new CCAENV977(base, vmeCrate))
{
  
}
/*! 
  Destruction means deletion of the module object:
 */
CCAENV977ScalerTrigger::~CCAENV977ScalerTrigger()
{
  delete m_pModule;
}
/*!
    Initialize:
    - Set the module into coincidence register mode.
    - reset the top bit of the input latch.
*/
void
CCAENV977ScalerTrigger::Initialize()
{
  // We'll leave the control register alone except for the 
  // pattern bit which we zero:

  m_pModule->controlRegister(m_pModule->controlRegister() & (CCAENV977::control_gateMask |
							     CCAENV977::control_OrMask));
  // Unmask my bit and  clear it:

  unsigned short mask = ~myBit;
  m_pModule->inputMask(m_pModule->inputMask() & mask);
  m_pModule->inputSet(m_pModule->inputSet() & mask);


}
/*!
  Check the trigger condition.
*/
bool
CCAENV977ScalerTrigger::operator()()
{
  unsigned short mask = m_pModule->singleHitReadAndClear();
  m_pModule->inputSet(mask & (~myBit));
  return ((mask & myBit) != 0);

}
