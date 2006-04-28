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
#include "CHitRegisterRead.h"
#include "CVMEPio.h"
#include "CSimulatedVMEList.h"

/*!
   Construction is just saving he parameters in member data for
   use when we are invoked.
*/
CHitRegisterRead::CHitRegisterRead(unsigned short modifier,
				   unsigned long  address) :
  m_modifier(modifier),
  m_address(address)
{}

/*!
   Destruction is just supplied to provide an unbroken chain of virtual
   destructors up the class hierarchy.
*/
CHitRegisterRead::~CHitRegisterRead()
{
}

/*!
  Read the hit data and store it in the list simulator.
*/
void*
CHitRegisterRead::operator()(CVMEPio& pio, 
			     CSimulatedVMEList& program, 
			     void* outBuffer)
{
  unsigned short hitData = pio.read16(m_modifier, m_address);
  unsigned short* p      = (unsigned short*)outBuffer;
  *p++ = hitData;
  program.setConditionMask(hitData);

  return p;

}
