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

#ifndef __CCONDITIONALREAD_H
#define __CCONDITINOALREAD_H

#ifndef __CBLOCKREADELEMENT_H
#include "CBlockReadElement.h"
#endif

#ifndef __CVMEPIO_H
#include "CVMEPio.h"
#endif

#ifndef __CSIMULATEDVMELIST_H
#include "CSimulatedVMEList.h"
#endif

#ifndef __STL_VECTOR
#include <vector>
#ifndef __STL_VECTOR
#define __STL_VECTOR
#endif
#endif

#ifndef __CRT_STDINT_H
#include <stdint.h>
#ifndef __CRT_STDINT_H
#define __CRT_STDINT_H
#endif
#endif


/*!
   This is a block read element that is conditionalized by the
   most recently read hit pattern.  Hit patterns are read via the
   CHitRegisterRead element, and stored in the list for later
   use by this element.
   The caller supplies an ordered vector of terms.  Each term is a 16 bit mask.
   The terms are one-by-one bitwise anded with the stored mask.
   If any of these bitwise ands results in a non zero pattern,
   the condition is made, and the read performed.. otherwise not.
*/
template <class T>
class CConditionalRead : public CBlockReadElement<T>
{
private:
  STD(vector)<uint16_t>   m_terms;
public:
  CConditionalRead(unsigned short mode, unsigned long base, size_t count,
		   STD(vector)<uint16_t> terms) :
    CBlockReadElement<T>(mode, base, count),
    m_terms(terms) 
  {}
  virtual ~CConditionalRead() {}

  virtual void* operator()(CVMEPio& pio, 
			   CSimulatedVMEList& program, 
			   void* outBuffer)
  {
    uint16_t  mask = program.getConditionMask();
    int       numTerms = m_terms.size();
    for (int i=0; i < numTerms; i++) {
      if(m_terms[i] & mask) {
	return CBlockReadElement<T>::operator()(pio, program, outBuffer);
      }
    }
    return outBuffer;
  }
  
};



#endif
