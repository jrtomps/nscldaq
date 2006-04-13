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
#include "CSBSVmeAddressRange.h"
#include "CSBSVmeException.h"

#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif

/*!
   Construct an address range.  Errors are reported as
   exceptions.
   \param handle  : bt_desc_t [in]
      Handle open on a bit 3 device via bt_open.
      This is the device that will be used to perform the mapping.
   \param addressModifier : unsigned short [in]
      This is the address modifier that will  be placed on the bus for each
      bus cycle to the address range conducted by this class.
   \param base : unsigned long [in]
      Base address of the map.
   \param bytes : size_t [in]
      Number of bytes in the map.
*/
CSBSVmeAddressRange::CSBSVmeAddressRange(bt_desc_t handle, 
					  unsigned short addressModifier,
					  unsigned long  base,
					  size_t         bytes) :
					 CVMEAddressRange(addressModifier,
							  base,
							  bytes),
					 m_handle(handle)
{
  // We need to get the current am for the unit so it can be restored:

  bt_devdata_t oldAm;
  bt_error_t status = bt_get_info(m_handle, BT_INFO_MMAP_AMOD, &oldAm);
  if (status != BT_SUCCESS) {
    throw CSBSVmeException(status,
			   "CSBSVmeAddressRange::construction - get info");
  }
  // Set the address modifier we want

  bt_devdata_t newAm = addressModifier;
  status              = bt_set_info(m_handle, BT_INFO_MMAP_AMOD, newAm);
  if (status != BT_SUCCESS) {
    throw CSBSVmeException(status, 
			   "CSBSVmeAddressRange::construction - set info");
  }
  //  Create the memory map:

  status = bt_mmap(m_handle, &m_pMap, base, bytes, BT_RDWR, BT_SWAP_NONE);
  if (status != BT_SUCCESS) {
    throw CSBSVmeException(status,
			   "CSBSVmeAddressRange::construction - mmap");
  }
  // Restore the old MMAP AMOD:

  status = bt_set_info(m_handle, BT_INFO_MMAP_AMOD, oldAm);
  if (status != BT_SUCCESS) {
    unmap();			// Can't worry about status here.
    throw CSBSVmeException(status,
			   "CSBSVmeAddressRange::construction - restore mmap amod");
  }

}
/*!
    Destroy an address range.
*/
CSBSVmeAddressRange::~CSBSVmeAddressRange()
{
  unmap();
}

/*!
    Return a pointer ot the map.
*/
void*
CSBSVmeAddressRange::mappingPointer()
{
  return m_pMap;
}



/*!
    poke a longword of data at the region.
    \param offset : size_t
      longword offset to the location to poke.
    \param data   : long
      The longword to poke.
    
    Note that RangeCheck is used to ensure that offset is in range.
*/
void
CSBSVmeAddressRange::pokel(size_t offset, long data)
{
  poke(offset, data);
}
/*!
   Poke a word of data at the region.
   \param offset : size_t 
      word offset to the location to poke.
   \param data  : short
      The data to poke.

    Note that RangeCheck is used to ensure that the offset is in range.


*/
void
CSBSVmeAddressRange::pokew(size_t offset, short data)
{
  poke(offset, data);
}

/*!   
   Poke a byte of data at the region
   \param offset : size_t
      Byte offset to the location to poke.
   \param data   : char
      Byte to poke.

   Note that the offset will be range checked.
*/
void
CSBSVmeAddressRange::pokeb(size_t offset, char data)
{
  poke(offset, data);
}

/*!
   Peek an unsigned long from an offset in memory.
   \param offset : size_t
     The longword offset to the long to peek.

   Note the offset will be range checked.
*/
unsigned long
CSBSVmeAddressRange::peekl(size_t offset)
{
  return peek<unsigned long>(offset);

}
/*!
   See above.. but peeks a word.
*/
unsigned short
CSBSVmeAddressRange::peekw(size_t offset)
{
  return peek<unsigned short>(offset);
}

/*!
   See above but peeks a byte
*/
unsigned char
CSBSVmeAddressRange::peekb(size_t offset)
{
  return peek<unsigned char>(offset);
}

//////////////////////////////////////////////////////////////////

// Utility to unmap from the VME range:

void
CSBSVmeAddressRange::unmap()
{
  bt_unmmap(m_handle,
	    m_pMap, size());
}
