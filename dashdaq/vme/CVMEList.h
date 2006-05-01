
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

#ifndef __CVMELIST_H
#define __CVMELIST_H

#ifndef __CRT_STDLIB_H
#include <stdlib.h>		// For size+t
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

#ifndef __STL_VECTOR
#include <vector>
#ifndef __STL_VECTOR
#define __STL_VECTOR
#endif
#endif

/*!
   Abstract base class for list processors.  A noted subclass is
   CSimulatedVMEList which builds on PIO access to the VME crate
   to provide a simulated list environment in which one may
   be able to test some list operations.
*/
class CVMEList 
{
public:

  // Canonical methods Since the class is so purely abstract we implement
  // inline in order to do away with the need for a .cpp file that would
  // be mostly empty air.

  CVMEList() {}
  CVMEList(const CVMEList& rhs) {}
  virtual ~CVMEList() {}

  int operator==(const CVMEList& rhs) const {return 1; }
  int operator!=(const CVMEList& rhs) const {return !(*this==rhs);}



  // Pure virtual methods:

  virtual size_t listCount()    = 0;
  virtual size_t triggerCount() = 0;
  
  // Add single shot reads:

  virtual void addRead32(unsigned short mode, unsigned long address) = 0;
  virtual void addRead16(unsigned short mode, unsigned long address) = 0;
  virtual void addRead8(unsigned short mode, unsigned long address)  = 0;

  // Add single shot writes:

  virtual void addWrite32(unsigned short mode, unsigned long address,
			  long data) = 0;
  virtual void addWrite16(unsigned short mode, unsigned long address,
			  long data) = 0;
  virtual void addWrite8(unsigned short mode, unsigned long address,
			 long data)  = 0;

  // Block read operations.

  virtual void addBlockRead32(unsigned short mode, unsigned long address, 
			      size_t count) = 0;
 
  virtual void addBlockRead16(unsigned short mode, unsigned long address, 
			      size_t count) = 0;
  virtual void addBlockRead8(unsigned short mode, unsigned long address, 
			      size_t count) = 0;

  // Block write operations.  These are overloaded so that the data
  // can be supplied either as a vector or as a raw buffer.

  virtual void addBlockWrite32(unsigned short mode, unsigned long address,
			       STD(vector)<uint32_t> data) = 0;
  virtual void addBlockWrite32(unsigned short mode, unsigned long address,
			       void* data, size_t count);

  virtual void addBlockWrite16(unsigned short mode, unsigned long address,
			       STD(vector)<uint16_t> data) = 0;
  virtual void addBlockWrite16(unsigned short mode, unsigned long address,
			       void* data, size_t count);

  virtual void addBlockWrite8(unsigned short mode, unsigned long address,
			      STD(vector)<uint8_t> data) = 0;
  virtual void addBlockWrite8(unsigned short mode, unsigned long address,
			       void* data, size_t count);

  // Count field operations.

  virtual void defineCountField(uint8_t rightShift, uint32_t mask) = 0;
  virtual void addCountFieldRead32(unsigned short modifier, unsigned long base) = 0;
  virtual void addCountFieldRead16(unsigned short modifier, unsigned long base) = 0;
  virtual void addCountFieldRead8(unsigned short modifier, unsigned long base)  = 0;

  // Hit pattern conditionals:

  virtual void addHitRegisterRead(unsigned short modifier, unsigned long address) = 0;
  virtual void addConditionalRead32(STD(vector)<uint16_t> terms,
				    unsigned short modifier, unsigned long address) = 0;
  virtual void addConditionalRead32(uint16_t* terms, unsigned int numTerms,
				    unsigned short modifier, unsigned long address);

  virtual void addConditionalRead16(STD(vector)<uint16_t> terms,
				    unsigned short modifier, unsigned long address) = 0;
  virtual void addConditionalRead16(uint16_t* terms, unsigned int numTerms,
				    unsigned short modifier, unsigned long address);

  virtual void addConditionalRead8(STD(vector)<uint16_t> terms,
				   unsigned short modifier, unsigned long address) = 0;
  virtual void addConditionalRead8(uint16_t* terms, unsigned int numTerms,
				   unsigned short modifier, unsigned long address);

  virtual void addConditionalBlockRead32(STD(vector)<uint16_t> terms,
					 unsigned short modifier, 
					 unsigned long address, size_t count) = 0;
  virtual void addConditionalBlockRead32(uint16_t* terms, unsigned int numTerms,
					 unsigned short modifier,
					 unsigned long address, size_t count);

  virtual void addConditionalBlockRead16(STD(vector)<uint16_t> terms,
					 unsigned short modifier,
					 unsigned long address, size_t count) = 0;
  virtual void addConditionalBlockRead16(uint16_t* terms, unsigned int numTerms,
					 unsigned short modifier,
					 unsigned long address, size_t count);

  virtual void addConditionalBlockRead8(STD(vector)<uint16_t> terms,
					unsigned short modifier,
					unsigned long address, size_t count) = 0;
  virtual void addConditionalBlockRead8(uint16_t* terms, unsigned int numTerms,
					unsigned short modifier,
					unsigned long address, size_t count);

  // Triggered list functions:

  virtual void specifyTrigger(unsigned int listNumber, unsigned int triggerNumber) = 0;
  virtual void arm(unsigned int listNumber) = 0;
  virtual void readTriggeredListData(unsigned int listNumber, 
			     void* buffer, size_t bufferSize) = 0;
				       

  // Execute the list immediately:

  virtual size_t execute(void* pReadData, size_t size) = 0;
};

#endif
