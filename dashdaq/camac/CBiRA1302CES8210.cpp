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


// implementation of CBiRA1302CES8210

#include <config.h>
#include "CBiRA1302CES8210.h"
#include <CVMESubsystem.h>
#include <CVMEInterface.h>
#include <CVMEAddressRange.h>
#include <CCESCBD8210.h>
#include <CInvalidInterfaceType.h>


/*!
   Create a crate.  In the end we need to have our vme address range
   created.  We will also ensure that the interface is valid.
   \param interface : CCAMACInterface&
      Reference to the interface we are plugged into.
   \param vmeCrate  : unsigned int
      VME crate number our interface is in.
   \param camacCrate : unsigned int
      CAMAC crate number selected on the front panel of our module.

    \throw CInvalidInterfaceType
    - interface is not a CCESCBD8210.
    \throw other from other members of other classes.

*/
CBiRA1302CES8210::CBiRA1302CES8210(CCAMACInterface& interface,
				   unsigned int     vmeCrate,
				   unsigned int     camacCrate)  :
  CCAMACCrate(interface),
  m_pCrate(0)
{
  // Require the right kind of interface:

  CCESCBD8210* pInterface = dynamic_cast<CCESCBD8210*>(&interface);
  if (!pInterface) {
    throw CInvalidInterfaceType("CAMAC interface is not a CCESCBD8210",
				"Constructing a CBiRA1302CES8210 crate");
  }
  
  // Now get the appropriate VME interface

  CVMESubsystem sys;
  CVMEInterface& vmebus(sys[vmeCrate]);
  
  // Have the camac interface tell us what the base address is:

  unsigned long base = pInterface->base(camacCrate);
  size_t length      = pInterface->mapsize();

  m_pCrate = vmebus.createAddressRange(0x3d, base, length);
  
}
/*!
  Destroy the crate:
*/
CBiRA1302CES8210::~CBiRA1302CES8210()
{
  delete m_pCrate;
}
/////////////////////////////////////////////////////////////////////////
//  Operations on the controller:

/*!
   Perform a C cycle.  The C cycle pulses the C line in the
   CAMAC data way for us this means doing an N28.A9.F26
*/
void
CBiRA1302CES8210::C()
{
  control(28,26,9);

}
/*!
  Perform a Z cycle.  The Z cycle pulses the the Z line in the CAMAC
  data way.  This is done via an N28.A8.F26
*/
void
CBiRA1302CES8210::Z()
{
  control(28,26,8);

}
/*!
   Inhibit the crate.  This is done by asserting the I line on the 
   data way.  The I line remains asserted until explicitly released.
   This is done via a N30.A9.F26
*/
void
CBiRA1302CES8210::Inhibit()
{
  control(30,26,9);

}
/*!
   Uninhibit the crate.  This is done by de-asserting the I line on the
   dataway.  The I line remains deasserted until explicitly asserted.
   This is done via n N30.A9.F24
*/
void
CBiRA1302CES8210::Uninhibit()
{
  control(30,24,9);
}
/*!
   Return the state of the I line. This is done by 
   doing a N30.A9.F27 and returning the Q result from that.
*/
bool
CBiRA1302CES8210::isInhibited()
{
  control(30,27,9);
  return Q();

}
/*!
   Read the graded lam register from the module.
   Contrary to the description in the 1302 manual, this is a
   24 bit register not a 5 bit register.  
   In any event, the read is done via a N30.A0.F0
*/
unsigned long
CBiRA1302CES8210:: readGL()
{
  return read(30, 0, 0);
}
///////////////////////////////////////////////////////////////////////
//
//  Operations on modules in the dataway.

/*!
  read a 24 bit longword from a module.
  \param slot  : size_t
     The slot to read from.
  \param f     : unsigned int
     The function code to read with.
  \param a : unsigned int
     The subaddress to read from.
  \return unsigned int
  \retval the 24 bits read from the module.
 
   Note that if the function, slot and subaddress are not ok,
   an exception will be thrown by CCAMACCrate's appropriate checking
   function.
*/
unsigned int
CBiRA1302CES8210::read(size_t slot, unsigned int f, unsigned int a)
{
  validateRead(slot, f, a);
  unsigned long addr = offset(slot, f, a, false);
 
  // Note the interface is d16:

  unsigned long data = m_pCrate->peekw(addr/sizeof(short)) ;
  data               = (data << 16)                     | 
                        m_pCrate->peekw((addr+sizeof(short))/sizeof(short));

  return (data & 0xffffff);	// Don't want to chance bits set in top 8 bits.
}

/*!
   Read a 16 bit word.  This is almost like the previous function.
*/
unsigned short
CBiRA1302CES8210::read16(size_t slot, unsigned int f, unsigned int a)
{
  validateRead(slot, f, a);
  unsigned long addr = offset(slot, f, a);
  return m_pCrate->peekw(addr/sizeof(short));
}
/*!
   Write a 24 bit longword.
   Parameter like for read but add:
   \param datum : unsigned int
      24 bit datum to write.
*/
void
CBiRA1302CES8210::write(size_t slot, unsigned int f, unsigned int a,
			unsigned int datum)
{
  validateWrite(slot, f, a);
  unsigned long addr = offset(slot, f, a, false);

  // Interface has a 16 bit data path!!

  m_pCrate->pokew(addr/sizeof(short), datum >> 16);
  m_pCrate->pokew(addr/sizeof(short)+1, datum & 0xffff);

}
/*!
  Same as for write but only writes a 16 bit word.
*/
void
CBiRA1302CES8210::write16(size_t slot, unsigned int f, unsigned int a,
			  unsigned short datum)
{
  validateWrite(slot, f, a);
  unsigned long addr = offset(slot, f, a);
  m_pCrate->pokew(addr/sizeof(short), datum);
}
/*!
   Perform a control function.. parameters are as for the read members.
*/
void
CBiRA1302CES8210::control(size_t slot, unsigned int f, unsigned int a)
{
  requireControl(f);
  requireSlot(slot);
  requireSubaddress(a);

  unsigned long addr = offset(slot, f, a);
  m_pCrate->pokew(addr/sizeof(short), 0);
}

/////////////////////////////////////////////////////////////////////////
//    Test functions.

/*!
   Return the X value of the last operation.  Note that for the CES
   branch highway driver, you can only return the X value from the
   most recent operation on \em that branch.    If other CAMAC functions
   on other crates in the branch have been performed, the X/Q responses
   for the most recent one on this crate get lost.
*/
bool
CBiRA1302CES8210::X()
{
  CCESCBD8210&     cbd(getInterface());
  return cbd.lastX();
}
/*!
   Return the Q value of the last operation.  Similar restritions apply
   as for X()
*/
bool
CBiRA1302CES8210::Q()
{
  CCESCBD8210&    cbd(getInterface());
  return  cbd.lastQ();
}

////////////////////////////////////////////////////////////////////////
//   Private utilities

/* offset  - compute the offset to a fna.
 */
unsigned long
CBiRA1302CES8210::offset(size_t slot, unsigned int function, 
			 unsigned int subaddress, bool shortTransfer)
{
  CCESCBD8210&     cbd(getInterface());

  return cbd.offset(slot, function, subaddress, shortTransfer);
}
/*
  Validate a read's f,n,a...throwing exceptions if not valid.
*/
void
CBiRA1302CES8210::validateRead(size_t n, unsigned int f, unsigned int a)
{
  requireRead(f);
  requireSlot(n);
  requireSubaddress(a);
}
/*
   Validate a write's f.n.a.. throwing exceptions if not valid.
*/
void
CBiRA1302CES8210::validateWrite(size_t n, unsigned int f, unsigned int a)
{
  requireWrite(f);
  requireSlot(n);
  requireSubaddress(a);
}
/* 
   Get the interface pointer of the right type or throw
*/
CCESCBD8210& 
CBiRA1302CES8210::getInterface()
{
  CCAMACInterface& interface(CCAMACCrate::getInterface());
  CCESCBD8210& result(dynamic_cast<CCESCBD8210&>(interface));
  return result;
}
