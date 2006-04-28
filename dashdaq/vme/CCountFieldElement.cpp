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
#include "CCountFieldElement.h"
#include "CSimulatedVMEList.h"

#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif

/*!
   Constructing the element is just a matter of saving the
   shift count and mask so they can be applied when the element is
   invoked:
*/
CCountFieldElement::CCountFieldElement(uint8_t shift, uint32_t mask) :
  m_shift(shift),
  m_mask(mask)
{
  
}
/*!
   Destruction is a no-op but supplied to provide an unbroken chain of
   virtual destructors to the base class.
*/
CCountFieldElement::~CCountFieldElement()
{
}

/*!
   The invocation of the element just stores the
   field in the simulator list:
*/
void*
CCountFieldElement::operator()(CVMEPio& pio, CSimulatedVMEList& program, 
			       void* outBuffer)
{
  program.setCountExtractionParameters(m_shift, m_mask);
  return outBuffer;
}
