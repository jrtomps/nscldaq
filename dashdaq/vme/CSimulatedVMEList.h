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


class CVMEPio;

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

  // Extended methods of the class that support list execution:

  void     setCountExtractionParameters(uint8_t shiftCount, uint32_t mask);
  uint8_t  getCountRightShift() const;
  uint32_t getCountMask() const;

  void     setConditionMask(uint16_t pattern);
  uint16_t getConditionMask() const;

};
#endif
