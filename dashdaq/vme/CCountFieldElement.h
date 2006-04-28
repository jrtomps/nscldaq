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

#ifndef __CCOUNTFIELDELEMENT_H
#define __CCOUNTFIELDELEMENT_H

#ifndef __CSIMULATORELEMENT_H
#include "CSimulatorElement.h"
#endif



#ifndef __CRT_STDINT_H
#include <stdint.h>
#ifndef __CRT_STDINT_H
#define __CRT_STDINT_H
#endif
#endif

/*!
   Set a new count fiel description in the list.
   The count field description determines which bits are important
   for the initial read done by a CCountFieldRead operation.
*/
class CCountFieldElement : public CSimulatorElement
{
private:
  uint8_t  m_shift;	      	// Number of bits to shift.
  uint32_t m_mask; 		// Bit mask

public:
  CCountFieldElement(uint8_t shift, uint32_t mask);
  virtual ~CCountFieldElement();

  virtual void* operator()(CVMEPio& pio, CSimulatedVMEList& program, 
			   void* outBuffer);

};


#endif
