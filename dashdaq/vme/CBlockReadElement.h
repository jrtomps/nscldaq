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

#ifndef __CBLOCKREADELEMENT_H
#define __CBLOCKREADELEMENT_H

#ifndef __CSIMULATORELEMENT_H
#include "CSimulatorElement.h"
#endif

#ifndef __CRT_STDLIB_H
#include <stdlib.h>		/* For size_t */
#ifndef __CRT_STDLIB_H
#define __CRT_STDLIB_H
#endif
#endif

#ifndef __CRT_STDINT_H
#include <stdint.h>
#ifndef __CRT_STDINT_H
#define __CRT_STDINT_H
#endif
#endif

#ifndef __VMEPIO_H
#include <CVMEPio.h>
#endif

#ifndef __CSIMULATEDVMELIST_H
#include "CSimulatedVMEList.h"
#endif

#ifndef __CDEVICEINCAPABLE_H
#include <CDeviceIncapable.h>
#endif

/*!
   Simulator instruction step that does a block read from the
   VME bus.
*/
template <class T>
class CBlockReadElement : public CSimulatorElement
{
private:
  unsigned short m_modifier;	// Address modifier.
  unsigned long  m_startAddress;// Base address.
  size_t         m_count;       // Read count.
public:
  CBlockReadElement(unsigned short modifier, unsigned long start, size_t count) :
    m_modifier(modifier),
    m_startAddress(start),
    m_count(count) {}
  virtual ~CBlockReadElement() {}

  // Canonicals can be done via bitwise copy.

  virtual void* operator()(CVMEPio& pio, CSimulatedVMEList& program, void* outBuffer) 
  {
    T*            p = static_cast<T*>(outBuffer);
    unsigned long s = m_startAddress;
    if (sizeof(T) == sizeof(uint32_t)) {
      for(int i =0; i < m_count; i++) {
	*p++ = (T)pio.read32(m_modifier, s);
	s   += sizeof(T);
      }
    }
    else if (sizeof(T) == sizeof(uint16_t)) {
      for (int i =0; i < m_count; i++) {
	*p++ = (T)pio.read16(m_modifier, s);
	 s  += sizeof(T);
      }
    }
    else if (sizeof(T) == sizeof(uint8_t)) {
      for (int i=0; i < m_count; i++) {
	*p++  = (T)pio.read8(m_modifier, s);
 	 s   += sizeof(T);
      }
    } 
    else {
      throw CDeviceIncapable("non primitive read",
			     "CBlockReadElement::operator()",
			     "8, 16, 32 bit reads only");
    }
    return static_cast<void*>(p);
  }
};

#endif
