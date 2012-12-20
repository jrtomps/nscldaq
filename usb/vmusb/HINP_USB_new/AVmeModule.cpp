#ifndef __AVMEMODULE_CPP
#define __AVMEMODULE_CPP

#include "AVmeModule.h"
#include <RangeError.h>
#include <stdio.h>

#include <assert.h>
#include <daqdatatypes.h>

// Manifest constants:

static const int SLOT_RANGE[2]   = {1, 21};

// Constructor with base address spelled out
AVmeModule::AVmeModule (std::string name, unsigned int slot, long base) :
	m_sName(name),
	m_nSlot(slot),
	m_lBase(base)
{
  // validate the slot:
  if(!ValidSlot(slot)) {
    throw CRangeError(SLOT_RANGE[0], SLOT_RANGE[1], slot,
		      "Instantiating a Vme module");
  }
  // Assign pointer to VME device
  // Right now we only support A32 devices
  //  m_pDevice = (int*)CVMEInterface::Open(CVMEInterface::A32, 0);
}

// Constructor with base address spelled out and crate number
AVmeModule::AVmeModule (std::string name, unsigned int crate, unsigned int slot, long base) :
	m_sName(name),
	m_nCrate(crate),
	m_nSlot(slot),
	m_lBase(base)
{
  // validate the slot:
  if(!ValidSlot(slot)) {
    throw CRangeError(SLOT_RANGE[0], SLOT_RANGE[1], slot,
		      "Instantiating a Vme module");
  }
  // Assign pointer to VME device
  // Right now we only support A32 devices
  //  m_pDevice = (int*)CVMEInterface::Open(CVMEInterface::A32, crate);
}

// Constructor with geographical base address
AVmeModule::AVmeModule (std::string name, unsigned int slot) :
	m_sName(name),
	m_nSlot(slot)
{
  // validate the slot:
  if(!ValidSlot(slot)) {
    throw CRangeError(SLOT_RANGE[0], SLOT_RANGE[1], slot,
		      "Instantiating a Vme module");
  }

  // setup the base address
  m_lBase = slot<<27;
  // Assign pointer to VME device
  // Right now we only support A32 devices
  //  m_pDevice = (int*)CVMEInterface::Open(CVMEInterface::A32, 0);
}

// Constructor with geographical base address and crate number
AVmeModule::AVmeModule (std::string name, unsigned int crate, unsigned int slot) :
	m_sName(name),
	m_nCrate(crate),
	m_nSlot(slot)
{
  // validate the slot:
  if(!ValidSlot(slot)) {
    throw CRangeError(SLOT_RANGE[0], SLOT_RANGE[1], slot,
		      "Instantiating a Vme module");
  }

  // setup the base address
  m_lBase = slot<<27;
  // Assign pointer to VME device
  // Right now we only support A32 devices
  //  m_pDevice = (int*)CVMEInterface::Open(CVMEInterface::A32, crate);
}

// Destructor
AVmeModule::~AVmeModule()
{
  //	CVMEInterface::Close(m_pDevice);
}

/*!
   Copy construction. This is invoked when e.g. an object is passed by value
   to a function.  The copy constructor makes a clone of the rhs object.

   \param rhs - the source of the copy.
*/
AVmeModule::AVmeModule(const AVmeModule& rhs) :
  m_sName(rhs.m_sName),
  m_nCrate(rhs.m_nCrate),
  m_nSlot(rhs.m_nSlot),
  m_lBase(rhs.m_lBase),
  m_pDevice(rhs.m_pDevice)
{}

/*!
   Assignment operation.  This member function supports assignment of
   an object of this class to an object of the same class.

   \param aAVmeModule - rhs of the assignment.

   \return Reference to the lhs of the assignment. This allows = to be chained.
*/
AVmeModule& AVmeModule::operator= (const AVmeModule& aAVmeModule)
{
    if (this != &aAVmeModule) {

       m_sName = aAVmeModule.m_sName;
       m_nSlot = aAVmeModule.m_nSlot;
       m_nCrate = aAVmeModule.m_nCrate;
       m_lBase = aAVmeModule.m_lBase;
       m_pDevice = aAVmeModule.m_pDevice;

    }
    return *this;
}

/*!
  Compare for equality.  If equal, all members are equal.  The pointer
  is derived from b,c,n so it is not compared:
  */
int
AVmeModule::operator==(const AVmeModule& rhs) const
{
  return ((m_lBase == rhs.m_lBase)      &&
	  (m_pDevice  == rhs.m_pDevice)       &&
	  (m_nCrate  == rhs.m_nCrate)       &&
	  (m_nSlot   == rhs.m_nSlot));
}

// Functions for class AVmeModule

/*!
   Determines if slot is a valid slot number.
   \param slot - Slot number to check.
   */
bool
AVmeModule::ValidSlot(unsigned int slot)
{
  return ((slot >= SLOT_RANGE[0]) && (slot <= SLOT_RANGE[1]));
}

// Function to map a chunck of memory of the module
// length is in Bytes
// return a pointer to the mapped memory
long*
AVmeModule::Map(long offset, long length)
{
  //	return ((long*)CVMEInterface::Map(m_pDevice, m_lBase+offset, length));
  return 0;
}



#endif
