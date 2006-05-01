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

#ifndef __CSIMULATEDVMELIST_H
#define __CSIMULATEDVMELIST_H

#ifndef __CVMELIST_H
#include "CVMEList.h"
#endif

#ifndef __CRT_STDINT_H
#include <stdint.h>
#ifndef __CRT_STDINT_H
#define __CRT_STDINT_H
#endif
#endif

#ifndef __STL_VECTOR
#include <vector>
#ifndef __STL_VECTOR
#define __STL_VECTOR
#endif
#endif


class CVMEPio;
class CSimulatorElement;

/*!
    This class provides a simulated immediate list based on
    underlying PIO semantics.  The intent is that you can
    use this in systems without list processors either for
    near compatibility or for testing list execution.
*/
class CSimulatedVMEList : public CVMEList
{
  // data members:

  uint8_t    m_countShift;	// Position extracted count bit field.
  uint32_t   m_countMask;       // Mask to extract  count bit field.
  
  uint16_t   m_pattern;		// Pattern mask.

  STD(vector)<CSimulatorElement*>  m_program; // The list to simulate.
  CVMEPio*                         m_pPio;    // The PIO object that executes the VME ops.
  
  // constructors and canonicals:

public:
  CSimulatedVMEList(CVMEPio& pio);
  virtual ~CSimulatedVMEList();

  // We're not going to allow copy like operations.. not because the 'program'
  // can't be copied but because the pio object may not support it.

private:
  CSimulatedVMEList(const CSimulatedVMEList& rhs);
  CSimulatedVMEList operator=(const CSimulatedVMEList& rhs);
  int operator==(const CSimulatedVMEList& rhs) const;
  int operator!=(const CSimulatedVMEList& rhs) const;
public:

  // overrides of virtual methods.

  virtual size_t listCount();
  virtual size_t triggerCount();

  // Single reads:

  virtual void addRead32(unsigned short mode, unsigned long address);
  virtual void addRead16(unsigned short mode, unsigned long address);
  virtual void addRead8(unsigned short mode, unsigned long address);

  // Single writes:

  virtual void addWrite32(unsigned short mode, unsigned long address,
			  long data);
  virtual void addWrite16(unsigned short mode, unsigned long address,
			  long data);
  virtual void addWrite8(unsigned short mode, unsigned long address,
			 long data);

  // Block reads:

  virtual void addBlockRead32(unsigned short mode, unsigned long address, 
			      size_t count);
 
  virtual void addBlockRead16(unsigned short mode, unsigned long address, 
			      size_t count);
  virtual void addBlockRead8(unsigned short mode, unsigned long address, 
			      size_t count);

  // Block writes.

  virtual void addBlockWrite32(unsigned short mode, unsigned long address,
			       STD(vector)<uint32_t> data);
  virtual void addBlockWrite16(unsigned short mode, unsigned long address,
			       STD(vector)<uint16_t> data);
  virtual void addBlockWrite8(unsigned short mode, unsigned long address,
			      STD(vector)<uint8_t> data);
  
  // count field operations

  virtual void defineCountField(uint8_t rightShift, uint32_t mask);
  virtual void addCountFieldRead32(unsigned short modifier, unsigned long base);
  virtual void addCountFieldRead16(unsigned short modifier, unsigned long base);
  virtual void addCountFieldRead8(unsigned short modifier, unsigned long base);


  // Hit pattern conditionals:

  virtual void addHitRegisterRead(unsigned short modifier, unsigned long address);
  virtual void addConditionalRead32(STD(vector)<uint16_t> terms,
				    unsigned short modifier, unsigned long address);

  virtual void addConditionalRead16(STD(vector)<uint16_t> terms,
				    unsigned short modifier, unsigned long address);
  virtual void addConditionalRead8(STD(vector)<uint16_t> terms,
				   unsigned short modifier, unsigned long address);
  virtual void addConditionalBlockRead32(STD(vector)<uint16_t> terms,
					 unsigned short modifier, 
					 unsigned long address, size_t count);
  virtual void addConditionalBlockRead16(STD(vector)<uint16_t> terms,
					 unsigned short modifier,
					 unsigned long address, size_t count);
  virtual void addConditionalBlockRead8(STD(vector)<uint16_t> terms,
					unsigned short modifier,
					unsigned long address, size_t count);


  // Triggered list functions.  Simulated lists don't support triggered operation,
  // therefore these will all throw a CDeviceIncapable exception.
  
  virtual void specifyTrigger(unsigned int listNumber, unsigned int triggerNumber);
  virtual void arm(unsigned int listNumber);
  virtual void readTriggeredListData(unsigned int listNumber, 
			     void* buffer, size_t bufferSize);


  // Execute the list:

  virtual size_t execute(void* pReadData, size_t size);

  // Extended methods of the class that support list execution:

  void     setCountExtractionParameters(uint8_t shiftCount, uint32_t mask);
  uint8_t  getCountRightShift() const;
  uint32_t getCountMask() const;

  void     setConditionMask(uint16_t pattern);
  uint16_t getConditionMask() const;

};
#endif
