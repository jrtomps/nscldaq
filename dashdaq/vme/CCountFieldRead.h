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

#ifndef __COUNTFIELDREAD_H
#define __CCOUNTFIELDREAD_H

#ifndef __CSIMULATORELEMENT_H
#include "CSimulatorElement.h"
#endif

#ifndef __CBLOCKREADELEMENT_H
#include "CBlockReadElement.h"
#endif

#ifndef __CVMEPIO_H
#include "CVMEPio.h"
#endif

#ifndef __CSIMULATEDVMELIST_H
#include "CSimulatedVMEList.h"
#endif

#ifndef __CDEVICEINCAPABLE_H
#include <CDeviceINcapable.h>
#endif

#ifndef __CRT_STDINT_H
#include <stdint.h>
#ifndef __CRT_STDINT_H
#define __CRT_STDINT_H
#endif
#endif

/*!
   This class provides block read with a transfer count that is determined
   by an initial read.  At present, the initial read must be the same widt
   as the block transfer width.
      The internal function of this element is to 
      'manually' perform the individual read, then to instantiate a 
      CBLockReadElement to do the appropriate block read after determining
      the correct transfer count.
*/
template <class T>
class CCountFieldRead : public CSimulatorElement
{
private:
  unsigned short     m_modifier; // Address modifier for the reads.
  unsigned long      m_base;	 // Address of the first read.
public:
  CCountFieldRead(unsigned short modifier, unsigned long base) :
    m_modifier(modifier),
    m_base(base)
  {}
  virtual ~CCountFieldRead() {}

  virtual void* operator()(CVMEPio& pio, 
			   CSimulatedVMEList& program, 
			   void* outBuffer)
  {
    T* dest = (T*)outBuffer;
    uint32_t transferCount;	// Will hold the transfer count for common code

    
    // To get the transfer count requires we do the appropriately
    // sized initial read...

    if (sizeof(T) == sizeof(uint32_t)) {
      transferCount = (uint32_t)(pio.read32(m_modifier, m_base));
    }
    else if (sizeof(T) == sizeof(uint16_t)) {
      transferCount = (uint32_t)(pio.read16(m_modifier, m_base));
    }
    else if (sizeof(T) == sizeof(uint8_t)) {
      transferCount = (uint32_t)(pio.read8(m_modifier, m_base));
    }
    else {
      throw CDeviceIncapable("non primitive transfer count read",
			     "CCountFieldRead::operator()",
			     "8, 16, 32 bit reads only");
    }

    // This code is common and independent of the type:

    *dest++ = (T)transferCount;
    transferCount = 
      (transferCount >> program.getCountRightShift()) & program.getCountMask();

    CBlockReadElement<T> rd(m_modifier, m_base+sizeof(T), transferCount);
    return rd(pio, program, dest);

  }
};

#endif
