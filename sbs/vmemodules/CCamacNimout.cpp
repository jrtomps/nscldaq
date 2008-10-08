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

static const char* Copyright = "(C) Copyright Michigan State University 2002, All rights reserved";
//////////////////////CCamacNimout.cpp file////////////////////////////////////

#include <config.h>
#include "CCamacNimout.h"                  

using namespace std;



/*!
   Construct a NIMOUT module located in a particular part of CAMAC space.
   \param b  - Branch in which the module lives (if not yet mapped it will
               be mapped.
   \param c  - Crate in which the module lives.
   \param n  - Slot in which the module lives.

   \exception CRangeError if the branch crate or slot are bad.
   \exception CErrnoException if at some point a system service returned an
              error.
*/
CCamacNimout::CCamacNimout (unsigned int b,
			    unsigned int c,
			    unsigned int n) :
  CCamacModule(b,c,n),
  m_pWrite(0) 
{
  // The bcnaf pointer for data writes are cached:

  m_pWrite = (unsigned short*)MakePointer(16, 0, true);
} 
/*!
   Copy construction. This is invoked when e.g. an object is passed by value
   to a function.  The copy constructor makes a clone of the rhs object.
*/
CCamacNimout::CCamacNimout(const CCamacNimout& rhs)  :
  CCamacModule(rhs),
  m_pWrite(rhs.m_pWrite)
{

}

/*!
   Assignment operation.  This member function supports assignment of
   an object of this class to an object of the same class.
*/
CCamacNimout& CCamacNimout::operator= (const CCamacNimout& aCCamacNimout)
{ 
    if (this != &aCCamacNimout) {
       CCamacModule::operator= (aCCamacNimout);
       m_pWrite = aCCamacNimout.m_pWrite;

    }
    return *this;
}
/*!
  Equality comparison.
  */
int
CCamacNimout::operator==(const CCamacNimout& rhs) const
{
  return ((m_pWrite == rhs.m_pWrite)  &&
	  operator==(rhs));
}

// Functions for class CCamacNimout

/*!
    Writes a mask of data to the NIMOUT.
    All of the bits in the mask will be strobed simultaneously.
    
    \param mask - The mask of bits to strobe.  Note that only
                            bits below bit 12 (from 0) mean anything.

	\param unsigned short mask

*/
void 
CCamacNimout::WriteMask(unsigned short mask)  
{
  *m_pWrite = mask;
}  

/*!
    Strobes a single bit of output.
    \param nBitno - The bit to strobe numbered from 0.
                             Any value of 12 or above results in a no-op.

	\param unsigned int nBitno

*/
void 
CCamacNimout::WriteBit(unsigned int nBitno)  
{
  *m_pWrite = (1 << nBitno);
}
