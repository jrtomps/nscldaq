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
#include "CVC32CC32.h"
#include <CVMEAddressRange.h>
#include "CWienerVC32.h"

#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif

/*!
   Consruct a CAMAC crate.  The interface and
   its address space are saved.
*/
CVC32CC32::CVC32CC32(CWienerVC32& interface)  :
  CCAMACCrate(interface),
  m_pCrate(&(interface.getAddressRange())),
  m_Interface(interface)
{
  
}
/*  

   At this time there's no need to do anything to destroy the crate:
*/
CVC32CC32::~CVC32CC32()
{
}
/////////////////////////////////////////////////////////////////////////////
///////////////////////  Control operations /////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

/*!
   Do a C cycle on the crate:
*/
void
CVC32CC32::C()
{
  m_Interface.C();
}
/*!
   Do A Z cycle on the crate:
*/
void
CVC32CC32::Z()
{
  m_Interface.Z();
}
/*!
   Inhibit the crate.
*/
void
CVC32CC32::Inhibit()
{
  m_Interface.Inhibit();
}
/*!
   Remove the crate inhibit:
*/
void
CVC32CC32::Uninhibit()
{
  m_Interface.unInhibit();
}
/*!
  Return true if the crate is inhibited.  We read the CC32 status
  register and check the CWienerVC32::CC32Status::Inhibit bit:
*/
bool
CVC32CC32::isInhibited()
{
  uint16_t status = m_Interface.readCC32Status();

  return ((status & CWienerVC32::CC32Status::Inhibit)  != 0) ? true : false;

}
/*!
   Read the state of the lams. 
*/
unsigned long
CVC32CC32::readGL()
{
  uint32_t masks = m_Interface.readLams();
  return masks & 0xffffff;
}
/////////////////////////////////////////////////////////////////////////////
///////////////////////// Dataway operations ////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/*!
    Do a 32 bit read from the CAMAC crate:
    \param slot  : size_t
       Slot number to target the read request 
    \param f     : unsigned int
       Function code to use for the read, must be in the range [0,7]
    \param a     : unsigned int
       Read subaddress.
    \return unsigned int
    \retval Results of the readl.
*/
unsigned int
CVC32CC32::read(size_t slot, unsigned int f, unsigned int a)
{
  off_t off = validReadOffset(slot,f,a); // Throws on errors.
  return m_pCrate->peekl(off/sizeof(unsigned int));
}
/*!
   Do a 16 bit read from the CAMAC crate; parameters are as for read.
*/
unsigned short
CVC32CC32::read16(size_t slot, unsigned int f, unsigned int a)
{
  off_t off = validReadOffset(slot,f,a);
  return m_pCrate->peekw(off/sizeof(unsigned short));
}
/*!
   Do a 24 bit write to a CAMAC module.
   Parameters are as for read with an additional:
   \param data : unsigned int
      Data to write.  Only the least significant 24 bits matter.
*/
void
CVC32CC32::write(size_t slot, unsigned int f, unsigned int a, unsigned int datum)
{
  off_t off = validWriteOffset(slot, f, a);
  m_pCrate->pokel(off/sizeof(unsigned int), datum);
}
/*!
 Do a 16 bit write to a CAMAC module.
 parameters are as for write().
*/
void
CVC32CC32::write16(size_t slot, unsigned int f, unsigned int a, unsigned short datum)
{
  off_t off  = validWriteOffset(slot, f, a);
  m_pCrate->pokew(off/sizeof(unsigned short), datum);
}
/*!
   Do a control operation.
*/
void
CVC32CC32::control(size_t slot, unsigned int f, unsigned int a)
{
  off_t off = validControlOffset(slot, f, a);

  // The top bit of the f code is determined by the transfer direction:

  if (f & 0x10) {
    m_pCrate->pokew(off/sizeof(uint16_t), 0);
  }
  else {
    m_pCrate->peekw(off/sizeof(uint16_t));
  }
}
/////////////////////////////////////////////////////////////////////////////
//////////////////////// Data way response checks ///////////////////////////
/////////////////////////////////////////////////////////////////////////////

/*!
  Check the X response of the last oepation .  This is a matter of reading
  the CC32's status register and checking the CWienerVC32::CC32Status::X bit.
*/
bool
CVC32CC32::X()
{
  uint16_t status = m_Interface.readCC32Status();
  return ((status & CWienerVC32::CC32Status::X) != 0) ? true : false;
}
/*!
  Check the Q response of the last operation.  This is also a bit
  (CWienerVC32::CC32Status::Q) in the CC32 status register.
*/
bool
CVC32CC32::Q()
{
  uint16_t status = m_Interface.readCC32Status();
  return ((status & CWienerVC32::CC32Status::Q) != 0) ? true : false;
}

/////////////////////////////////////////////////////////////////////////////
////////////////////////// Validating offset computers //////////////////////
/////////////////////////////////////////////////////////////////////////////

/*
   Validate n,a,f for a read and return the offset.
*/
off_t
CVC32CC32::validReadOffset(size_t n, unsigned int f, unsigned int a)
{
  requireRead(f);
  requireSlot(n);
  requireSubaddress(a);
  return m_Interface.offset(n,f,a);
}
/*
  Validate n,a,f for a write operation and return the offset.
*/
off_t
CVC32CC32::validWriteOffset(size_t n, unsigned int f, unsigned int a)
{
  requireWrite(f);
  requireSlot(n);
  requireSubaddress(a);
  return m_Interface.offset(n,f,a);
}

/*
   Validate an n,a,f for a control operation and return the offset.
*/
off_t
CVC32CC32::validControlOffset(size_t n, unsigned int f, unsigned int a)
{
  requireControl(f);
  requireSlot(n);
  requireSubaddress(a);
  return m_Interface.offset(n,f,a);

}


////////////////////////////////////////////////////////////////////////////
//////////////////////////// Test support //////////////////////////////////
////////////////////////////////////////////////////////////////////////////

CWienerVC32&
CVC32CC32::getInterface() 
{
  return m_Interface;
}
