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

// Implementation of CCESCBD8210.  

#include <config.h>
#include "CCESCBD8210.h"
#include <CCAMACCrate.h>
#include <CVMEAddressRange.h>
#include <CVMESubsystem.h>
#include <CVMEInterface.h>
#include <RangeError.h>
#include <CDuplicateDevice.h>
#include <CNoSuchDevice.h>

#include <stdio.h>

#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif


static const int    MAXBRANCH(7);
static const long   BRANCH0   (0x800000);
static const long   BRANCHSIZE(0x100000);
static const long   CRATESIZE (0x020000);

static const unsigned long SLOTSHIFT(11);
static const unsigned long SUBSHIFT(7);
static const unsigned long FSHIFT(2);
static const unsigned long SHORTBIT(2);

typedef struct _naf {
  size_t       slot;
  unsigned int subaddress;
  unsigned int function;
} naf;

static const naf CSR = {29, 0, 0};
static const naf ITF = {29, 0, 4};
static const naf BTB = {29, 0, 9};
static const naf BZ  = {29, 0, 9};


/*!
   Construct an interface:
   \param vmeCrate  : unsigned int
      Number of the VME itnerface we are stuffed in.
   \param branchNumber : unsigned int
      Branch number set on our front panel selector.

   \throw CRangeError if:
     - The branch is out of range.
     - The VME crate number is invalid.

*/
CCESCBD8210::CCESCBD8210(unsigned int vmeCrate,
			 unsigned int branchNumber) :
  m_pRegisters(0),
  m_VMECrate(vmeCrate),
  m_Branch(branchNumber)
{
  setCrateCount(7);		// 1-7 though not 0-6.

  if (branchNumber > MAXBRANCH) {
    throw CRangeError(0, MAXBRANCH, branchNumber,
		      "creating a CESCBD8210 instance");
  }

  for (int i =0; i < 8; i++) {
    m_Crates[i] = static_cast<CCAMACCrate*>(NULL);
  }

  CVMESubsystem sys;
  CVMEInterface& interface = sys[vmeCrate]; // Will throw if needed.
  m_pRegisters = interface.createAddressRange(0x3d, // Supervisory data is safest.
					      BRANCH0+branchNumber*BRANCHSIZE,
					      CRATESIZE);
}
/*!
   Return the correct base address for a crate in this branch:
*/
unsigned long
CCESCBD8210::base(unsigned crate)
{
  return (BRANCH0 + m_Branch*BRANCHSIZE ) | CRATESIZE*crate;

}
/*!  return the length of a crate map:
 */
size_t mapsize()
{
  return CRATESIZE;
}
/*!
   When destructing a branch it is important to 
   - Remove and delete all crates (we are assuming all crates are
     dynamically allocated !!
   - destroy the address range to our registers.
*/
CCESCBD8210::~CCESCBD8210()
{
  for(int i =0; i < 8; i++) {
    if(m_Crates[i]) {
      delete m_Crates[i];
      m_Crates[i] = static_cast<CCAMACCrate*>(NULL);
    }
  }
  delete m_pRegisters;
}
//////////////////////////////////////////////////////////////////////////

/*!
   Turn an naf in a crate into an offset into the crate that can be peeked/
   poked. 
   \param slot   : size_t
        Slot number
   \param function : unsigned int
        Function code to execute.
   \param subaddress : unsigned int
        Subadress to reference.
   \param shortTransfer : bool [true]
       True if transfers are 16 bit wide.

*/
unsigned long
CCESCBD8210::offset(size_t slot, unsigned int function, unsigned int subaddress,
		    bool shortTransfer)
{
  unsigned long result = (slot       << SLOTSHIFT)  |
                         (function   << FSHIFT)     |
                         (subaddress << SUBSHIFT)   |
                         (shortTransfer ? SHORTBIT : 0);
  return result;
}
/*!
   Peform a branch zero on the branch.   This is done by writing 0
   to the branch zero register.
   
*/
void
CCESCBD8210::Z()
{
  unsigned long address = offset(BZ.slot, BZ.function, BZ.subaddress);
  m_pRegisters->pokew(address, 0);
}
/*!
  Read the control status register.
*/
unsigned short
CCESCBD8210::readCSR()
{
  unsigned long csrloc = offset(CSR.slot, CSR.function, CSR.subaddress);
  return   m_pRegisters->peekw(csrloc);
}
/*!
   Write the CSR:
   \param datum : unsigned short
      The thing to write.

*/
void
CCESCBD8210::writeCSR(unsigned short datum)
{
  unsigned address = offset(CSR.slot, CSR.function, CSR.subaddress);
  m_pRegisters->pokew(address, datum);
}
/*!   
   Return true if the last operation on the branch resulted in an X
*/
bool
CCESCBD8210::lastX() 
{
  unsigned short csr = readCSR();
  return (csr & csr_X) != 0;
}
/*!
   Return true if the last operation on the branch resulted in a Q
*/
bool
CCESCBD8210::lastQ()
{
  unsigned short csr = readCSR();
  return (csr & csr_Q) != 0;
}

/*!  

    Read the interrupt flag register.

*/
unsigned short 
CCESCBD8210::readITF()
{
  unsigned long address = offset(ITF.slot, ITF.function, ITF.subaddress);
  return m_pRegisters->peekw(address);
}
/*!
   Write the interrupt flag register (this is needed to clear the IT2/IT4 bits).
*/
void
CCESCBD8210::writeITF(unsigned short datum)
{
  unsigned long address = offset(ITF.slot, ITF.function, ITF.subaddress);
  m_pRegisters->pokew(address, datum);
}

/*!
   Read the branch timing register.  That will describe the set of crates that
   are online.
*/
unsigned short
CCESCBD8210::readBTB()
{
  unsigned long address = offset(BTB.slot, BTB.function, BTB.subaddress);
  return m_pRegisters->peekw(address);
}

/*!
  Return true if the selected crate is installed.  This does nothing to determine
  the online status of the crate.   Not all interfaces may be able to determine
  this, however this one can do that through use of the readBTB() member.
  \param crate : size_t
      Crate number selected.
  
  \note If the crate number is illegal, by definition we don't have that crate.

*/
bool
CCESCBD8210::haveCrate(size_t crate)
{
  if ((crate == 0) || (crate > maxCrates())) {
    return false;
  }

  return m_Crates[crate] != static_cast<CCAMACCrate*>(NULL);
}

/*!
  Add a crate to the system.  The crate must have been dynamically
  constructed (the factory will do this).  Clearly it does no good 
  to add a crate of the wrong type to the system, however we don't
  check that at this point.
  \param crate : CCAMACCrate& 
     Reference to the crate to add.
  \param number : size_t
     Which crate number is added (1-7).  The crate number is the
     number set in the thumbweel; of the controller.
  
  \throw CDuplicateDevice 
    - If that crate has already been installed.
  \throw CRangeError
    - If number is illegal.
*/
void
CCESCBD8210::addCrate(CCAMACCrate& crate, size_t number)
{
  if ((number == 0) || (number > maxCrates())) {
    throw CRangeError(1, 7, number, 
		      string(" Adding a crate to a CES8210 interface"));
  }
  if (haveCrate(number)) {
    char message[100];
    snprintf(message, sizeof(message)-1, 
	      "CES8210 CAMAC Crate number %d ", number);
    throw CDuplicateDevice(message, "installing a CAMAC crate in a CES8210");
  }
  m_Crates[number] = &crate;
}

/*!
   Remove a CAMAC crate from the system.  The callser is handed a pointer to the
   crate.  The caller can safely assume that the pointer can be passed to 
   destroy if the crate is no longer needed, however it is up to the caller
   to do the destruction.  Destroying this interface \em will, however
   destroy all the crates in the system.
   \param number : size_t
      Number of the crate to destroy.

   \return CCAMACCrate* 
   \retval NULL - No such crate, or illegal number.
   \retval not NULL - Pointer to the removed crate.
*/
CCAMACCrate*
CCESCBD8210::removeCrate(size_t number)
{
  if ((number == 0) || (number > maxCrates())) {
    return static_cast<CCAMACCrate*>(NULL);
  }
  return m_Crates[number];
}
/*!
   Get a reference to a specific crate.
   \param index  : size_t
      Crate number to get.
   \return CCAMACCrate&
   \retval Reference to crate number \em index.

   \throw CRangeError
      - If the index is invalid.
   \throw CNoSuchDevice 
      - If no device has been installed  in that index.
*/
CCAMACCrate&
CCESCBD8210::operator[](size_t index)
{
  if ((index == 0) || (index > maxCrates())) {
    throw CRangeError(1, 7, index,
		      "Indexing CESCBD8210 controlled crates");
  }
  CCAMACCrate* crate = m_Crates[index];
  if(!crate) {
    throw CNoSuchDevice("CCESCBD8210 controlled crate", index,
			"Indexing CES CBD 8210 controlled crates");
  }

  return *crate;
}
/*!
   Return true if a crate is online.  Note that this says
   nothing about whether or not the crate is installed, only if there
   is a physical controller that is online at the other end of the branch
   highway cable from us.
   \param index  : size_t
       Crate number to check.k
   \return bool
   \retval true the crate is online.
   \retval false the crate is not online.

   \throw CRangeError if the index is not valid.
   
*/
bool
CCESCBD8210::online(size_t index)
{
  if ((index == 0) || (index > maxCrates())) {
    throw CRangeError(1,7, index, "Checking online status of a CESCBD8210 crate");

  }
  unsigned short mask = (1 << index);
  return (readBTB() & mask) != 0;
}
