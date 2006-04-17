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
#include "CSBSPio.h"
#include "CSBSVmeDMATransfer.h"
#include "CVMEInterface.h"


/*!
   Construct a PIO object.  The object is capable of doing
   any sort of single shot access to the VME bus in a not very efficient
   manner.
*/
CSBSPio::CSBSPio(bt_desc_t handle) :
  m_handle(handle)
{}

/*!
   Destruction is only needed because we want to supply an unbroken
   chain of virtual destructors back to the base class.
*/
CSBSPio::~CSBSPio()
{
}
/*!
  Write a 32 bit longword to the vme bus.
  \param modifier : unsigned short
       Address modifier to assert on the bus during the write.
  \param address : unsigned long
       The base address of the write (this is a byte address).
       as unaligned transfers are supported.
  \param value   : long
       The value to write.

   We depend on the writexx utility to do the actual data transfer.
*/
void
CSBSPio::write32(unsigned short modifier, unsigned long address, long value)
{
  writexx(modifier, address, &value, sizeof(long));
}


/*!
   Write a 16 bit short, just like write 32 except for the width of the data.
*/
void
CSBSPio::write16(unsigned short modifier, unsigned long address, short value)
{
  writexx(modifier, address, &value, sizeof(short));
}
/*!
  Write an 8 bit byte... like write32 but the width is 8 bits.
*/
void
CSBSPio::write8(unsigned short modifier, unsigned long address, char value)
{
  writexx(modifier, address, &value, sizeof(char));
}

/// stubs for now:

unsigned long
CSBSPio::read32(unsigned short modifier, unsigned long address)
{
  return 0ul;
}
unsigned short 
CSBSPio::read16(unsigned short modifier, unsigned long address)
{
  return 0;
}
unsigned char 
CSBSPio::read8(unsigned short modifier, unsigned long address)
{
  return 0;
}

//////////////////////////////////////////////////////////////////////////

//  Write an arbitrary item to vme bus:
//  All the parameters are essentially the same as the
//  write* public members, but value is a pointer.
//  and width is the number of bytes to write.
//
// We'll create a DMA object to do the operation.
void
CSBSPio::writexx(unsigned short modifier, unsigned long address,
		 void* value,  size_t width)
{
  CSBSVmeDMATransfer xfer(m_handle, modifier, CVMEInterface::TW_32,
			  address, width);
  xfer.Write(value);
}

