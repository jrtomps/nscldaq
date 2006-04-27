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

#ifndef __CBLOCKWRITELEMENT_H
#define __CBLOCKWRITELEMENT_H

#ifndef __CSIMLUATORELEMENT_H
#include "CSimulatorElement.h"
#endif

#ifndef __CRT_STDLIB_H
#include <stdlib.h>
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

#ifndef __STL_VECTOR
#include <vector>
#ifndef __STL_VECTOR
#define __STL_VECTOR
#endif
#endif


/*!
   Templated class that performs block reads as an instruction
   step for the list simulator. 
*/

template <class T>
class CBlockWriteElement : public CSimulatorElement
{
private:
  unsigned short   m_modifier;     // Address modifier.
  unsigned long    m_startAddress; // Base address.
  STD(vector)<T>        m_data;	   // data to write.

public:
  CBlockWriteElement(unsigned short modifier, unsigned long start, 
		     STD(vector)<T>& data) :
    m_modifier(modifier),
    m_startAddress(start),
    m_data(data)
  {
  }
  virtual ~CBlockWriteElement() {
  }
  // Not sure if we'll ever use this but we can also set new data:

  void newData(STD(vector)<T>& data) {
    m_data = data;
  }
  // Now the I/O operations:

  virtual void* operator()(CVMEPio& pio, CSimulatedVMEList& program, void* outBuffer)
  {
    unsigned long dest = m_startAddress;

    if (sizeof(T) == sizeof(uint32_t)) {
      for (int i=0; i < m_data.size(); i++) {
	pio.write32(m_modifier, dest, (long)m_data[i]);
	dest += sizeof(T);
      }
    }
    else if (sizeof(T) == sizeof(uint16_t)) {
      for (int i = 0; i < m_data.size(); i++) {
	pio.write16(m_modifier, dest, (short)m_data[i]);
	dest += sizeof(T);
      }
    }
    else if (sizeof(T) == sizeof(uint8_t)) {
      for (int i =0; i < m_data.size(); i++) {
	pio.write8(m_modifier, dest, (char)m_data[i]);
	dest += sizeof(T);
      }  
    }
    else {
      throw CDeviceIncapable("non primitive write",
			     "CBlockWriteElement::operator()",
			     "8, 16, 32 bit writes only");
    }

    return outBuffer;		// No data read.
  }
};


#endif
