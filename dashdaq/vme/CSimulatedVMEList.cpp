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
#include "CSimulatedVMEList.h"
#include "CBlockReadElement.h"
#include "CBlockWriteElement.h"
#include "CCountFieldElement.h"
#include "CCountFieldRead.h"
#include "CHitRegisterRead.h"
#include "CConditionalRead.h"

#include <CDeviceIncapable.h>
#include <RangeError.h>

#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif

/*!
   Construct a simulated list.  
   - The stored data are initialized to 0..which may or may not make sense
     but is as good as any other random value.
   \param pio  : CVMEPio&
       a Programmed I/O object for the VME crate we will
       run the list on.  This provides access to the hardware
       at list execution time.
*/
CSimulatedVMEList::CSimulatedVMEList(CVMEPio& pio) :
  m_countShift(0),
  m_countMask(0),
  m_pattern(0),
  m_pPio(&pio)
{
}
/*!
   Destruction of a simulated list must destroy the program elements.
*/
CSimulatedVMEList::~CSimulatedVMEList()
{
  for(int i = 0; i < m_program.size(); i++) {
    delete m_program[i];
  }
}
/*!
   Return the number of stored lists supported by the list processor.
   For simulated list processor, only immediate execution is supported
   for now.
   \return size_t
   \retval  0
*/
size_t
CSimulatedVMEList::listCount()
{
  return 0;
}
/*!
   Return the number of trigger inputs the list processor has.
   Since simulated lists only support immediate execution,
   we have no triggers.
*/
size_t
CSimulatedVMEList::triggerCount()
{
  return 0;
}


/*!
   Add a 32 bit single shot read to the list.  This basically
   adds a block read element with a hard coded transfer count of 1.
   \param mode : unsigned short
       Address modifier bits to gate on the bus during the operation.
   \param address : unsigned long
       Address from which the read is done.
*/
void 
CSimulatedVMEList::addRead32(unsigned short mode, unsigned long address)
{
  m_program.push_back(new CBlockReadElement<uint32_t>(mode, address, 1));
}
/*!
  Add a 16 bit single shot read to the list.  This basically
  adds a block read element with a hard coded transfer count of 1.

   \param mode : unsigned short
       Address modifier bits to gate on the bus during the operation.
   \param address : unsigned long
       Address from which the read is done.
*/
void 
CSimulatedVMEList::addRead16(unsigned short mode, unsigned long address)
{
  m_program.push_back(new CBlockReadElement<uint16_t>(mode, address, 1));
}
/*!
  Add an 8 bit single shot read to the list.  This basically
  adds a block read element with a hard coded transfer count of 1.

   \param mode : unsigned short
       Address modifier bits to gate on the bus during the operation.
   \param address : unsigned long
       Address from which the read is done.
*/
void 
CSimulatedVMEList::addRead8(unsigned short mode, unsigned long address)
{
  m_program.push_back(new CBlockReadElement<uint8_t>(mode, address, 1));
}


/*!
   Add a single shot 32 bit  write to the list.  This is just the same as adding
   a block write with a count of 1.
   \param mode  : unsigned short
      Address modifier to put on the VME bus for the write.
   \param address : unsigned long
      Address to which the datum will be written.
   \param data  : unsigned long
       The datum to write.

   Note that when executed this does not consume any buffer space.
*/
void
CSimulatedVMEList::addWrite32(unsigned short mode, unsigned long address, 
			      long datum)
{
  vector<uint32_t> d;
  d.push_back((uint32_t)datum);

  m_program.push_back(new CBlockWriteElement<uint32_t>(mode, address, d));
}


/*!
   Add a single shot 16 bit  write to the list.  This is just the same as adding
   a block write with a count of 1.
   \param mode  : unsigned short
      Address modifier to put on the VME bus for the write.
   \param address : unsigned long
      Address to which the datum will be written.
   \param data  : unsigned long
       The datum to write.

   Note that when executed this does not consume any buffer space.
*/
void
CSimulatedVMEList::addWrite16(unsigned short mode, unsigned long address, 
			      long datum)
{
  vector<uint16_t> d;
  d.push_back((uint16_t)datum);

  m_program.push_back(new CBlockWriteElement<uint16_t>(mode, address, d));
}
/*!
   Add a single shot 8 bit  write to the list.  This is just the same as adding
   a block write with a count of 1.
   \param mode  : unsigned short
      Address modifier to put on the VME bus for the write.
   \param address : unsigned long
      Address to which the datum will be written.
   \param data  : unsigned long
       The datum to write.

   Note that when executed this does not consume any buffer space.
*/
void
CSimulatedVMEList::addWrite8(unsigned short mode, unsigned long address, 
			      long datum)
{
  vector<uint8_t> d;
  d.push_back((uint8_t)datum);

  m_program.push_back(new CBlockWriteElement<uint8_t>(mode, address, d));
}


/*!
   Add a block read with 32 bit width to the interpreter.
   \param mode : unsigned short
      Address modifier to use for the read.
   \param address : unsigned long
      Address of the first transfer.
   \param count : size_t
      Number of 32 bit transfers to tod.
*/
void
CSimulatedVMEList::addBlockRead32(unsigned short mode,
				  unsigned long  address,
				  size_t         count)
{
  m_program.push_back(new CBlockReadElement<uint32_t>(mode, address, count));
}

/*!
   Add a block read with 16 bit width to the interpreter.
   \param mode : unsigned short
      Address modifier to use for the read.
   \param address : unsigned long
      Address of the first transfer.
   \param count : size_t
      Number of 16 bit transfers to tod.
*/
void
CSimulatedVMEList::addBlockRead16(unsigned short mode,
				  unsigned long  address,
				  size_t         count)
{
  m_program.push_back(new CBlockReadElement<uint16_t>(mode, address, count));
}
/*!
   Add a block read with 8 bit width to the interpreter.
   \param mode : unsigned short
      Address modifier to use for the read.
   \param address : unsigned long
      Address of the first transfer.
   \param count : size_t
      Number of 8 bit transfers to tod.
*/
void
CSimulatedVMEList::addBlockRead8(unsigned short mode,
				  unsigned long  address,
				  size_t         count)
{
  m_program.push_back(new CBlockReadElement<uint8_t>(mode, address, count));
}



/*!
   Add a 32 bit block write to the list.  The data are inline with the list,
   therefore to modify the data that will be written, the list must be modified.

   \param mode  : unsigned short
      Address modifier for the writes.
   \param address : unsigned long
      Base address for the block target.
   \param data : vector<uint32_t>
      Data to write.
*/
void
CSimulatedVMEList::addBlockWrite32(unsigned short mode, unsigned long address,
				   vector<uint32_t> data)
{
  m_program.push_back(new CBlockWriteElement<uint32_t>(mode, address, data));
}

/*!
   Add a 16 bit block write to the list.  The data are inline with the list,
   therefore to modify the data that will be written, the list must be modified.

   \param mode  : unsigned short
      Address modifier for the writes.
   \param address : unsigned long
      Base address for the block target.
   \param data : vector<uint16_t>
      Data to write.
*/
void
CSimulatedVMEList::addBlockWrite16(unsigned short mode, unsigned long address,
				   vector<uint16_t> data)
{
  m_program.push_back(new CBlockWriteElement<uint16_t>(mode, address, data));
}


/*!
   Add a 8 bit block write to the list.  The data are inline with the list,
   therefore to modify the data that will be written, the list must be modified.

   \param mode  : unsigned short
      Address modifier for the writes.
   \param address : unsigned long
      Base address for the block target.
   \param data : vector<uint8_t>
      Data to write.
*/
void
CSimulatedVMEList::addBlockWrite8(unsigned short mode, unsigned long address,
				   vector<uint8_t> data)
{
  m_program.push_back(new CBlockWriteElement<uint8_t>(mode, address, data));
}


/*!
   Add an element to the list that will set the count field extraction
   parameters.
   \param rightShift  : uint8_t
       number of bits to shift the count field to the right prior to masking.
   \param mask        : uint32_t
       The mask to apply to the shifted data to extract the transfer count.
 
*/
void 
CSimulatedVMEList::defineCountField(uint8_t rightShift, uint32_t mask)
{
  m_program.push_back(new CCountFieldElement(rightShift, mask));
}

/*!
   Add a countfield read with a width of 32 bits.
   The count field used is the previous application done by a defineCountField.

   \param modifier : unsigned short
      Address modifier to gate on the bus for all transfers.
   \param base     : unsigned long
      Base address of the read.
*/
void
CSimulatedVMEList::addCountFieldRead32(unsigned short modifier, unsigned long base)
{
  m_program.push_back(new CCountFieldRead<uint32_t>(modifier, base));
}

/*!
   Add a countfield read with a width of 16 bits.
   The count field used is the previous application done by a defineCountField.

   \param modifier : unsigned short
      Address modifier to gate on the bus for all transfers.
   \param base     : unsigned long
      Base address of the read.
*/
void
CSimulatedVMEList::addCountFieldRead16(unsigned short modifier, unsigned long base)
{
  m_program.push_back(new CCountFieldRead<uint16_t>(modifier, base));
}

/*!
   Add a countfield read with a width of 8 bits.
   The count field used is the previous application done by a defineCountField.

   \param modifier : unsigned short
      Address modifier to gate on the bus for all transfers.
   \param base     : unsigned long
      Base address of the read.
*/
void
CSimulatedVMEList::addCountFieldRead8(unsigned short modifier, unsigned long base)
{
  m_program.push_back(new CCountFieldRead<uint8_t>(modifier, base));
}



/*!
   Add a hit register read to the list.  The 16 bit word read by this
   list element will be stored and used for later conditional reads.
   \param modifier  : unsigned short
     Address modifier selecting the address space from which the read occurs.
   \param address : unsigned long
      Specifies the address from which the read occurs.
*/
void
CSimulatedVMEList::addHitRegisterRead(unsigned short modifier, unsigned long address)
{
  m_program.push_back(new CHitRegisterRead(modifier, address));
}


/*!
   Add a 32 bit read that conditionalized on the hit register.
   \param terms  : vector<uint16_t>
      The conditional masks against which the hit register is checked.
   \param modifier : unsigned short
       The address modifier of the read.
   \param address  : unsigned long
      The address of the read.
*/
void
CSimulatedVMEList::addConditionalRead32(vector<uint16_t> terms,
					unsigned short modifier, 
					unsigned long address)
{
  m_program.push_back(new CConditionalRead<uint32_t>(modifier, address, 1,
						     terms));
}

/*!
   Add a 16 bit read that conditionalized on the hit register.
   \param terms  : vector<uint16_t>
      The conditional masks against which the hit register is checked.
   \param modifier : unsigned short
       The address modifier of the read.
   \param address  : unsigned long
      The address of the read.
*/
void
CSimulatedVMEList::addConditionalRead16(vector<uint16_t> terms,
					unsigned short modifier, 
					unsigned long address)
{
  m_program.push_back(new CConditionalRead<uint16_t>(modifier, address, 1,
						     terms));
}

/*!
   Add a 8 bit read that conditionalized on the hit register.
   \param terms  : vector<uint16_t>
      The conditional masks against which the hit register is checked.
   \param modifier : unsigned short
       The address modifier of the read.
   \param address  : unsigned long
      The address of the read.
*/
void
CSimulatedVMEList::addConditionalRead8(vector<uint16_t> terms,
					unsigned short modifier, 
					unsigned long address)
{
  m_program.push_back(new CConditionalRead<uint8_t>(modifier, address, 1,
						     terms));
}

/*!
    Add a 32 bit block read that is conditionalized by the hit register.
    \param terms : vector<uint16_t>
       The terms that conditionalize the read.
    \param modifier : unsigned short
       The address modifier that selects the address space from which the read
       can go.
    \param address : unsigned long
       The VME address of the first item to transfer.
    \param count : size_t
       The number of items to transfer at this width.
*/
void
CSimulatedVMEList::addConditionalBlockRead32(STD(vector)<uint16_t> terms,
					     unsigned short modifier, 
					     unsigned long address, size_t count)
{
  m_program.push_back(new CConditionalRead<uint32_t>(modifier, address, count, 
						     terms));
}

/*!
    Add a 16 bit block read that is conditionalized by the hit register.
    \param terms : vector<uint16_t>
       The terms that conditionalize the read.
    \param modifier : unsigned short
       The address modifier that selects the address space from which the read
       can go.
    \param address : unsigned long
       The VME address of the first item to transfer.
    \param count : size_t
       The number of items to transfer at this width.
*/
void
CSimulatedVMEList::addConditionalBlockRead16(STD(vector)<uint16_t> terms,
					     unsigned short modifier, 
					     unsigned long address, size_t count)
{
  m_program.push_back(new CConditionalRead<uint16_t>(modifier, address, count, 
						     terms));
}

/*!
    Add a 8 bit block read that is conditionalized by the hit register.
    \param terms : vector<uint16_t>
       The terms that conditionalize the read.
    \param modifier : unsigned short
       The address modifier that selects the address space from which the read
       can go.
    \param address : unsigned long
       The VME address of the first item to transfer.
    \param count : size_t
       The number of items to transfer at this width.
*/
void
CSimulatedVMEList::addConditionalBlockRead8(STD(vector)<uint16_t> terms,
					     unsigned short modifier, 
					     unsigned long address, size_t count)
{
  m_program.push_back(new CConditionalRead<uint8_t>(modifier, address, count, 
						     terms));
}

/*!
   Specify the trigger for a list.
   Since triggered lists are not supported by simulated lists,
   this will:
   \throw CDeviceIncapable

*/
void
CSimulatedVMEList::specifyTrigger(unsigned int listNumber,
				    unsigned int triggerNumber)
{
  throw CDeviceIncapable("Specify list trigger",
			 "CSimulatedVMEList::specifyTrigger()",
			 "immediate execution only");
}

/*!
   Arm a list.  This would normally enable the list to 
   execute on its specified trigger.  Simulated lists only support
   immediate execution, therefore:
   \throw CDeviceIncapable

*/
void
CSimulatedVMEList::arm(unsigned int listNumber)
{
  throw CDeviceIncapable("arm list",
			 "CSimulatedVMEList::arm()",
			 "immediate list execution only");
}

/*!
   post a read for triggered list data.  Since simulated lists
   only support immediate execution this :
   \throw CDeviceIncapable
*/
void
CSimulatedVMEList::readTriggeredListData(unsigned int listNumber,
					 void* buffer, size_t bufferSize)
{
  throw CDeviceIncapable("read Triggered List Data",
			 "CSimulatedVMEList::readTriggeredListData",
			 "Immediate list execution only");
}

/*!
  Execute the list immediately.  Each list is visited and executed.
  if the pointer ever runs off the end of the buffer a 
  CRangeError Exception is thrown.
   \param pReadData : void* [out]
      Pointer to where the data read will go.
   \param size : size_t
      Number of bytes in pReadData.
   \return size_t
   \retval  Number of bytes transferred to the read buffer.
            It's up to the caller to be able to make sense of this.
*/
size_t CSimulatedVMEList::execute(void* pData, size_t size)
{
  uint8_t*   pStart = (uint8_t*)pData;
  uint8_t*   dest(pStart);

  size_t steps = m_program.size();
  for (int i =0; i < steps; i++) {
    dest = (uint8_t*)(*m_program[i])(*m_pPio, *this, dest);
    if ( (dest - pStart) > size) {
      throw CRangeError(0, size, (dest - pStart),
			"CSimulatedVMEList::execute - executing an immediate list");
    }
  }
  return (dest - pStart);
}


/*!
   set the count field extraction parameters.  Normally this will
   be called from within a list element.  The count field extraction
   parameters are a shift count and a mask.  The are applied to a 
   count bearing item x as follows:
      transferCount = (x >> m_countShift) & m_countMask
   \param shiftCount : uint8_t
      New right shift count for the above equation.
   \param  mask : uint32_t
      New mask for the above equation.
*/
void 
CSimulatedVMEList::setCountExtractionParameters(uint8_t  shiftCount,
						 uint32_t mask)
{
  m_countShift = shiftCount;
  m_countMask  = mask;
}
/*!
  Retrieve the count extraction shift count.  See setCountExtractionParameters
  above for more on this value and how it is used.  
  \return uint8_t
  \retval The current value of m_countShift.

 */
uint8_t
CSimulatedVMEList::getCountRightShift() const
{
  return m_countShift;
}
/*!
   Retrieve the count extraction mask.  See setCountExtractionParameters above
   for more information on this value and how it is used.
   \return uint32_t
   \retval m_countMask
*/
uint32_t
CSimulatedVMEList::getCountMask() const
{
  return m_countMask;
}
/*!
   Set the hit pattern value.  This is normally done by a simulator element
   (program step).  The hit pattern value is then used by later simulator
   elements in the list to conditionalize their operations.
   See documentation of the conditionalized operations e.g.
   CConditionalRead for more information about how this conditionalization
   works.
   \param pattern : uint16_t
      The new conditional pattern.
*/
void
CSimulatedVMEList::setConditionMask(uint16_t pattern)
{
  m_pattern = pattern;
}
/*!
   Return the current hit pattern value.  This is normally called by
   a simulator element (program step) like CConditionalRead to determine
   if the condition for performing the conditional operation is made.
   \return uint16_t
   \retval m_pattern.
*/
uint16_t
CSimulatedVMEList::getConditionMask() const
{
  return m_pattern;
}
