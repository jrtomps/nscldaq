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

#ifndef __CHITREGISTERREAD_H
#define __CHITREGISTERREAD_H

#ifndef __CSIMULATORELEMENT_H
#include "CSimulatorElement.h"
#endif

/*
   Reads a 16 bit word from an arbitrary location
   in VME space (arbitrary Amod too), and stores it
   as the conditional hit pattern.  The hit pattern is 
   used to conditionalize the family of CConditionalRead
   simulator elements.
*/
class CHitRegisterRead : public CSimulatorElement
{
private:
  unsigned short    m_modifier;
  unsigned long     m_address;
public:
  CHitRegisterRead(unsigned short modifier, unsigned long address);
  virtual ~CHitRegisterRead();

  virtual void* operator()(CVMEPio& pio, 
			   CSimulatedVMEList& program, 
			   void* outBuffer);
};

#endif
