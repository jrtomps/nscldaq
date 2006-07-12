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
#include "CWienerVC32.h"
#include "CVC32CC32.h"
#include <CVMEInterface.h>
#include <CVMESubsystem.h>
#include <CVMEAddressRange.h>
#include <CDuplicateDevice.h>
#include <RangeError.h>
#include <CInvalidInterfaceType.h>
#include <CNoSuchDevice.h>

#include <sys/types.h>

#ifdef HAVE_STD_NAMEPSACE 
using namespace std;
#endif
/////////////////////////////////////////////////////////////////////////////
// Constants:

static const int nshift(10);
static const int ashift(6);
static const int fshift(2);

//

struct registerNaf {
  size_t        slot;
  unsigned int  function;
  unsigned int  subaddress;

  registerNaf(size_t n,unsigned int a,unsigned int f) :
    slot(n),
    function(f),
    subaddress(a) {}
};

static const registerNaf VC32SCR(0,0,3);
static const registerNaf CC32Status(0,0,0);
static const registerNaf CKey(0,0,16);
static const registerNaf ZKey(0,1,16);
static const registerNaf InhOn(27,0,16);
static const registerNaf InhOff(27,1,16);
static const registerNaf ResetLamFF(28, 0, 16);
static const registerNaf bcastMask(26,0,16);
static const registerNaf bcastOp(25, 0, 0); // Broadcast fills in the rest of this
static const registerNaf lamMask(28,4,0);
static const registerNaf cycleA(30, 0, 16);
static const registerNaf cycleB(30, 1, 16);
static const registerNaf cycleC(10, 2, 16);
static const registerNaf Reset(31, 0, 16);

/////////////////////////////////////////////////////////////////////////////

/*!
    Construct a VC32 interface.
    \param vmecrate : size_t
      Number of the VME crate that the interface is in.
    \param base : uint32_t
        The base address set in the switches on the module.
*/
CWienerVC32::CWienerVC32(size_t vmecrate, uint32_t base) :
  m_Interface(*(CVMESubsystem::getInstance()[vmecrate].createAddressRange(0x39,
							 base,
							 0x7fff))),
  m_pCrate(0)
{
}
/*!
   Destroy the interface.
*/
CWienerVC32::~CWienerVC32()
{
  delete m_pCrate;
  delete &(m_Interface);
}

/*!
   Tell how many crates we have.
*/
size_t 
CWienerVC32::maxCrates() const
{
  return 1;
}
/*!
    Say if we have a crate.  This requires that:
    - Crate number is 0.
    - m_pCrate is not null
    - the crate's online bit is set.
*/
bool 
CWienerVC32::haveCrate(size_t crate)
{
  if ((crate == 0) && m_pCrate) {
    return ((readStatus() & VC32Status::online) != 0) ? true : false;
  }
  else {
    return false;
  }
}

/*!
   Add a crate to the system. 
   - The crate number must be 0.
   - The crate must not already be installed (m_pCrate == 0).
   - Only a valid crate can be installed (CVC32CC32).

   \param crate  : CCAMACCrate& 
      Reference to the crate to add.
   \para number  : size_t
      Number of the crate to add.

   \throw  CRangeError      - If the crate number is not 0.
   \throw  CDuplicateDevice - if all is valid but the crate is already installed
   \throw  CInvalidInterfaceType - If the crate is not a CVC32CC32 object.
*/
void
CWienerVC32::addCrate(CCAMACCrate& crate, size_t number)
{
  if (number != 0) {
    throw CRangeError(0,0,number,
		      "Installing a camac crate, bad crate number");
  }
  if (m_pCrate) {
    throw CDuplicateDevice("CVC32CC323 0", 
			   "Installing a CAMAC crate in CWienerVC32::addCrate");
  }
  CVC32CC32* p  = dynamic_cast<CVC32CC32*>(&crate);
  if (!p) {
    throw CInvalidInterfaceType("Interface must be a CVC32C32",
				"Installing a CAMAC crate in CWienerVC32::addCrate");
  }
  m_pCrate = p;
}
    
/*!
   Remove the crate from the system.  
   - the crate must have been added
  \param number : size_t
     Number of the crate to remove.

  \return CCAMACCrate*
  \retval A pointer to the previously installed camac crate.

  \throw CRangeError    - number != 0.
  \throw CNoSuchDevice  - No crate has been installed.

*/
CCAMACCrate*
CWienerVC32::removeCrate(size_t number)
{
  if (number != 0) {
    throw CRangeError(0,0,number,
			  "Removing a crate, bad crate number");
  }
  if(!m_pCrate) {
    throw CNoSuchDevice("CVC32CC32", 0, "Attempting to remove it");
  }
  CCAMACCrate* pCrate  = static_cast<CCAMACCrate*>(m_pCrate);
  m_pCrate             = static_cast<CVC32CC32*>(NULL);
  return pCrate;
}

/*!
   Get an installed crate.
   \param crate : size_t
      Number of the crate to get.
   \return CCAMACCrate&
   \retval Reference to the indexed camac crate controller.

   \throw CRangeError - Number is not 0.
   \throw CNoSuchDevice - m_pCrate == 0.
*/
CCAMACCrate&
CWienerVC32::operator[](size_t crate)
{
  if (crate != 0) {
    throw CRangeError(0,0,crate,
		      "Indexing a crate in CWienerVC32::");
  }
  if (!m_pCrate) {
    throw CNoSuchDevice("CVC32CC32", 0, 
			"Indexing a crate inCWienerVC32::");
  }
  return dynamic_cast<CCAMACCrate&>(*m_pCrate);
}
/*!
   Return true if crate on line.
*/
bool
CWienerVC32::online(unsigned int crate)
{
  CCAMACCrate* pCrate = &(*this)[crate];
  return haveCrate(crate);
}
/*!
   Return the address range allocated by the controller.
  Since only one address space is used per crate, there's nt much
  of an advantage for each crate to manage its own storage.
  \return CVMEAddressRange&
  \retval  Reference to the address range (m_Interface) allocated
           for these operations.
*/
CVMEAddressRange&
CWienerVC32::getAddressRange()
{
  return m_Interface;
}
/*!
   Offset return the offset into the VME address range for a nfa.
   Note that the caller must know ahead of time if the operation has
   bit 16 set in the f code, as that determines if the operation is a read or
   write... this is academic for all but control operations..1/2 of which are
   reads and 1/2 writes.
   \param n  : size_t
     Number of the slot in which the addressed module is installed.
   \param f  : unsigned int
     Function code to execute.
   \param a  : subaddres of the module to which the function will be addressed.
   
   \return off_t
   \retval The byte offset into the address range.

   \note   - the caller must have validated the slot, function and subaddress.


*/
off_t
CWienerVC32::offset(size_t n, unsigned int f, unsigned int a)
{
  return ((n << nshift) | (a << ashift) | ((f&0xf) << fshift)); // VC32 section 3.2.2.
}

////////////////////////////////////////////////////////////////////////
///////////////////////// register I/O functions  //////////////////////
////////////////////////////////////////////////////////////////////////

/*!
   Read the VC32 status register. This operation does not require any
   cable activity between the VC32 and CC32.  Therefore the CC32 could,
   in fact be unplugged for this to work
   \return uint16_t
   \retval  Current value of the VC32 status register.  The
            nexted class VC32Status contains bit definitions for this
            register (e.g. CWienerVC32::VC32Status::online  is set
            when the CC32 is plugged in and powered up on the other end of
            the cable.).
*/
uint16_t
CWienerVC32::readStatus()
{
  off_t off = offset(VC32SCR.slot, VC32SCR.function, VC32SCR.subaddress);
  return m_Interface.peekw(off/sizeof(uint16_t));
}
/*!
   Odd thing about the VC32Status register is that it can be written as well
   as read even though its function code is clearly a read function.
   \param value : uint16_t
      Value to write in the VC32 SCR.
*/
void
CWienerVC32::writeStatus(uint16_t value)
{
  off_t off = offset(VC32SCR.slot, VC32SCR.function, VC32SCR.subaddress);
  m_Interface.pokew(off/sizeof(uint16_t), value);
}
/*!
    Read the CC32 status.  This register is located in the CC323 module
    on the other end of the cable from us.  Therefore it is necessary
    that the CC32 be plugged in and on. Arguably this is a crate function,
    but the differentiation between crate an interface functions is somewhat
    blurred in the VC32/CC32 controller.
    Note that this register is read-only.
   \return uint16_t  
   \retval the contents of that register.  The bits fro this register
           are defined in the nested class CC32Status.
*/
uint16_t
CWienerVC32::readCC32Status()
{
  off_t off = offset(::CC32Status.slot, ::CC32Status.function, ::CC32Status.subaddress);
  return m_Interface.peekw(off/sizeof(uint16_t));
}
/*!
    Perform a C-cyle on the CAMAC crate.
*/
void
CWienerVC32::C()
{
  off_t off = offset(CKey.slot, CKey.function, CKey.subaddress);
  m_Interface.pokew(off/sizeof(uint16_t), static_cast<uint16_t>(0));
}
/*!
  Perfrom a Z-cycle on the CAMAC crate.
*/
void
CWienerVC32::Z()
{
  off_t off = offset(ZKey.slot, ZKey.function, ZKey.subaddress);
  m_Interface.pokew(off/sizeof(uint16_t), static_cast<uint16_t>(0));
}

/*!
   Inhibit the crate.
*/
void 
CWienerVC32::Inhibit()
{
  off_t off = offset(InhOn.slot, InhOn.function, InhOn.subaddress);
  m_Interface.pokew(off/sizeof(uint16_t), static_cast<uint16_t>(0));
}
/*!
   Remove the crate inhibit.
*/
void
CWienerVC32::unInhibit()
{
  off_t off = offset(InhOff.slot, InhOff.function, InhOff.subaddress);
  m_Interface.pokew(off/sizeof(uint16_t), static_cast<uint16_t>(0));
}

/*!
   Resets the lam flipflop bit in the status register(s).
*/
void
CWienerVC32::resetLamFlipFlop()
{
  off_t off = offset(ResetLamFF.slot, ResetLamFF.function, ResetLamFF.subaddress);
  m_Interface.pokew(off/sizeof(uint16_t), static_cast<uint16_t>(0));
}


/*!
   Read the lam status register.  Note that this is >not< the lam mask
   register as described by the Wiener manual (section 3.8), rather it is the
   Lam-BUS register as described in 3.11.  The bottom 24 bits of this register
   reflect the status of lams for each slot in the crate.  Slot n is represented
   by (1 << (n-1)).
*/
uint32_t
CWienerVC32::readLams()
{
  off_t off = offset(lamMask.slot, lamMask.function, lamMask.subaddress);
  return m_Interface.peekl(off/sizeof(uint32_t));
}


////////////////////////////////////////////////////////////////////////////
///////////////////// multicast operations /////////////////////////////////
////////////////////////////////////////////////////////////////////////////

/*!
   The CC32/VC32 has the capability of doing a multicast 
   write or control operation.  This is done by first selecting the
   set of modules that will be targets of this operation by writing the
   'broadcast mask'.  This mask is just  longword with a bit per slot.
   Slot 1 is 0000001b, slot2 is 00000010b etc.  
   Once the broadcast mask is written, the broadcast operation can be done
   by calling e.g. broadcast32 or broadcast16.

   \param mask : uint32_t
      The slot mask to write.
*/
void
CWienerVC32::setBroadcastMask(uint32_t mask)
{
  off_t off = offset(bcastMask.slot, bcastMask.function, bcastMask.subaddress);
  m_Interface.pokel(off/sizeof(uint32_t), mask);
}
/*!
   Broadcast using a 32 bit transfer.  Note that if the operation
   is a read (assumed to be a control operation), a read is done and
   the data, which should be meaningless, is discareded.
   \param f  : unsigned int
      The function to perform.
   \param a  : unsigned int
      The subaddress to affect for all the targeted modules.
   \param data : uint32_t
      A 32 bit data word that will be written for all function codes
      with the 16's bit set.

*/
void
CWienerVC32::broadcast32(unsigned int f, unsigned int a, uint32_t data)
{
  off_t off = offset(bcastMask.slot, f, a);
  if (f & 0x10) {
    m_Interface.pokel(off/sizeof(uint32_t), data);
  }
  else {
    m_Interface.peekl(off/sizeof(uint32_t));
  }
}
/*!
   Same as broadcast32 but any transfers are done as 16 bit transfers.
*/
void
CWienerVC32::broadcast16(unsigned int f, unsigned int a, uint16_t data)
{
  off_t off = offset(bcastMask.slot, f, a);
  if (f & 0x10) {
    m_Interface.pokew(off/sizeof(uint16_t), data);
  }
  else {
    m_Interface.peekw(off/sizeof(uint16_t));
  }
}
/*!
  Reset the subsystem.
*/
void
CWienerVC32::reset()
{
  off_t off = offset(Reset.slot, Reset.function, Reset.subaddress);
  m_Interface.pokew(off/sizeof(uint16_t), 0);
}
/////////////////////////////////////////////////////////////////////////
///////////////////// Cycle tuning registers    /////////////////////////
/////////////////////////////////////////////////////////////////////////

/*!
  Write the A cycle tune register.    The A cycle tune register controls
  CAMAC cycle timing for slots 1 through 8.

  This, and all cycle tune registers
  contain a bunch of 2 bit fields, one for each slot the register can tune.
  Setting values for each of these 2 bit fields allows you to control the
  timing of CAMAC  operations...effectively implementing FastCAMAC level 1
  functionality.  The 2 bit field values are:
  - 00   - S1 happens 400ns after busy (CAMAC standard).
  - 01   - S1 happens 300ns after busy.
  - 10   - S1 happens 200ns after busy.
  - 11   - S1 happens 200ns after busy and S2 happens 100ns after S1.

  00 adheres strictly to the CAMAC standard and will therefore work with all
compliant modules.  Newer modules, however require shorter setup times than
when CAMAC was defined, so you can play around to see if these other timings
will work for your modules in your application.  Caveat Programmer however.
Unless the module documentation explicitly specifies timings, you may fall into
'margins traps' where timings you set will work just fine for one module but
not for others of the same model number due to component differences.

    \param value  : uint16_t
       The value to write to the Cycle Tuen registesr A
*/
void
CWienerVC32::writeCycleTuneA(uint16_t value)
{
  off_t off = offset(cycleA.slot, cycleA.function, cycleA.subaddress);
  m_Interface.pokew(off/sizeof(uint16_t), value);
}
/*!
   Write the B cycle tune register.  See the remarks for 
   writeCycleTuneA for more information about cycle tune registers.
   The B cycle tune register controls the CAMAC cycle timings for slots
   9 through  16.
*/
void 
CWienerVC32::writeCycleTuneB(uint16_t value)
{
  off_t off = offset(cycleB.slot, cycleB.function, cycleB.subaddress);
  m_Interface.pokew(off/sizeof(uint16_t), value);
    
}
/*!
    Write the C cyle tune register.  See the remarks for 
    writeCycleTuneA for more information about how these registers
    set the CAMAC cyle timings.  The C register sets the timings for slots
    17 through 24.
*/
void 
CWienerVC32::writeCycleTuneC(uint16_t value)
{
  off_t off = offset(cycleC.slot, cycleC.function, cycleC.subaddress);
  m_Interface.pokew(off/sizeof(uint16_t), value);
}

/*!

   Read cycle tune register A.  See writeCycleTuneA for more information.

*/

uint16_t
CWienerVC32::readCycleTuneA()
{
  off_t off = offset(cycleA.slot, cycleA.function, cycleA.subaddress);
  return m_Interface.peekw(off/sizeof(uint16_t));
}
/*!

   Read cycle tune register B.  See writeCycleTuneA for more information.

*/

uint16_t
CWienerVC32::readCycleTuneB()
{
  off_t off = offset(cycleB.slot, cycleB.function, cycleB.subaddress);
  return m_Interface.peekw(off/sizeof(uint16_t));
}

/*!

   Read cycle tune register C.  See writeCycleTuneA for more information.

*/

uint16_t
CWienerVC32::readCycleTuneC()
{
  off_t off = offset(cycleC.slot, cycleC.function, cycleC.subaddress);
  return m_Interface.peekw(off/sizeof(uint16_t));
}

