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
#include "CRingItem.h"
#include "DataFormat.h"

#include <CRingBuffer.h>
#include <CRingSelectionPredicate.h>
#include <string.h>

////////////////////////////////////////////////////////////////////////////////
//
// Constructors and other canonicals.
//

/*!
   Construt the ring item:
   - If the maxbody is larger than CRingItemStaticBufferSize, allocate
     a new buffer and point the item at that, otherwise point it at 
     m_staticBuffer.
   - Pointer m_cursor at the body of the ring item.
   - Calculate and fill in the storage size.
   - Set m_swapNeeded to false.
   - Set the ring item type.

   \param type - The ring item type.  This is only 16 bits wide but stored in a
                 32 bit word so receivers can determine if bytes were swapped.
   \param maxBody - Largest body we can hold

*/
CRingItem::CRingItem(uint16_t type, size_t maxBody) :
  m_pItem(reinterpret_cast<RingItem*>(&m_staticBuffer)),
  m_storageSize(maxBody),
  m_swapNeeded(false)
{

  // If necessary, dynamically allocate (big max item).

  newIfNecessary(maxBody);
  m_pItem->s_header.s_type = type;
  m_pItem->s_header.s_size = 0;
}
/*!
  Copy construct.  This is actually the same as the construction above, 
  however the item contents get memcpied into our ring body. The caller has to
  ensure that m_pCursor reflects the amount of data copied into the ring body.

  \param rhs  - The source of the copy.
*/
CRingItem::CRingItem(const CRingItem& rhs)
{
  // If the storage size is big enough, we need to dynamically allocate
  // our storage

  newIfNecessary(rhs.m_storageSize);

  copyIn(rhs);
}
/*!
    Destroy the item. If the storage size was big, we need to delete the 
    storage as it was dynamically allocated.
*/
CRingItem::~CRingItem()
{
  deleteIfNecessary();
}

/*!
   Assignment.  If necessary destroy the body.  If necessary re-create the body.
   After that it's just a copy in.

   \param rhs  - The object that we are duplicating into *this.
*/
CRingItem&
CRingItem::operator=(const CRingItem& rhs)
{
  if (this != &rhs) {
    deleteIfNecessary();
    newIfNecessary(rhs.m_storageSize);
    copyIn(rhs);
  }

  return *this;
}

/*!
   Comparison for equality.. note that true equality may be time consuming
   to determine, as it requires the contents of the items to be equal which will
   require linear time in the size of the item.
   \param rhs  - refers to the item to be compared with *this>
   \return int
   \retval 0   - Not equal\
   \retval 1   - equal

*/
int
CRingItem::operator==(const CRingItem& rhs) const
{
  // short cut by looking at storage size and swap characteristics first:

  if (m_storageSize != rhs.m_storageSize) return 0;
  if (m_swapNeeded  != rhs.m_swapNeeded ) return 0;

  // Now there's nothing for it but to compare the contents:

  return (memcmp(m_pItem, rhs.m_pItem, sizeof(RingItemHeader) + m_storageSize) == 0);
}
/*!
  Inequality is just the logical inverse of equality.  This can take time, see
  operator==

  \param rhs  - Reference to the object *this will be compared to.
  \return int
  \retval 0   - Objects are not unequal
  \retval 1   - Objects are unequal
*/
int
CRingItem::operator!=(const CRingItem& rhs) const
{
  return !(*this == rhs);
}

//////////////////////////////////////////////////////////////////////////////////////////
//
// Selectors:

/*!

   \return size_t
   \retval The size allocated to the ring item's body.
*/
size_t
CRingItem::getStorageSize() const
{
  return m_storageSize;
}

/*!
   \return size_t
   \retval Amount of data in the body.  This is the difference between the 
           cursor and the start of the body.
*/
size_t
CRingItem::getBodySize() const
{
  return (m_pCursor - m_pItem->s_body);
}
/*!
  \return void*
  \retval Pointer to the body of the ring item. To get the next insertion point,
          see getBodyCursor.
*/
void*
CRingItem::getBodyPointer() 
{
  return m_pItem->s_body;
}
/*!
   \return void*
   \retval Pointer to the next insertion point of the body
*/
void*
CRingItem::getBodyCursor()
{
  return m_pCursor;
}
/*!
   \return void*
   \retval Pointer to the body.  To be usually used by derived classes but...
*/
pRingItem
CRingItem::getItemPointer()
{
  return m_pItem;
}
/*!
   \return uint32_t
   \retval Current type of the item.
*/
uint32_t
CRingItem::type() const
{
  uint32_t rawType = m_pItem->s_header.s_type;
  if (mustSwap()) {
    return swal(rawType);
  }
  else {
    return rawType;
  }
}

///////////////////////////////////////////////////////////////////////////////////////
//
// Mutators.

/*!
  Update the body cursor to reflect data that was put in the item.
  \param pNewCursor - New vlaue for the body cursor.
 
*/
void
CRingItem::setBodyCursor(void* pNewCursor)
{
  m_pCursor = reinterpret_cast<uint8_t*>(pNewCursor);
}

///////////////////////////////////////////////////////////////////////////////////////
//
//   Object operations.

/*!
   Commit the current version of the ring data to a ring.  
   - Calculates the size field of the header
   - puts the data in the ring buffer.

   \param ring  - Reference to the ring buffer in which the item will be put.

   \note The invoking process must already be the producing process for the ring.
   \note This implementation has no mechanism to timeout the put, however that could be
         added later.
   \note This function is non-destructive. There's nothing to stop the caller from
         issuing it on several rings.
   
*/
void
CRingItem::commitToRing(CRingBuffer& ring)
{
  m_pItem->s_header.s_size = sizeof(RingItemHeader) + getBodySize();
  ring.put(m_pItem, m_pItem->s_header.s_size);
}

/*!
   This is primarily intended for items that have been constructed via
   the getFromRing static member.
   \return bool
   \retval true - The byte order on the generating system is different from that of
                  this system.
   \retval false - The byte order on the generating system is the same as that of this
                  system.
*/
bool
CRingItem::mustSwap() const
{
  return m_swapNeeded;
}
   

///////////////////////////////////////////////////////////////////////////////////////
//
//  Static class methods.

/*!
   Fetches the next item in the ring that matches the
   conditions described by the predicate and wraps it in a CRingItem.
   The m_swapNeeded flag will be set in accordance with the byte order of the underlying
   item.  This is determined by knowing that the high 16 bits of the type field
   shall always be zero.
   \param ring  - Reference to the ring from which to get the item
   \param predicate - an object that skips unwanted items in the ring.

   \return CRingItem*
   \retval Pointer to a new, dynamically constructed ring item that has been wrapped around
           the message fetched.

   \note There is no method for specifying a timeout on the wait for a desirable
         message.
*/
CRingItem*
CRingItem::getFromRing(CRingBuffer& ring, CRingSelectionPredicate& predicate)
{
  predicate.selectItem(ring);
  
  // look at the header, figure out the byte order and count so we can
  // create the item and fill it in.
  // We know the full item is available both because of how
  // ring items are inserted atomically and how the predicates work.
  //

  RingItemHeader header;
  ring.peek(&header, sizeof(header)); 

  bool otherOrder(false);
  uint32_t size = header.s_size;
  if ((header.s_type & 0xffff0000) != 0) {
    otherOrder = true;
    union {
      uint32_t  lword;
      uint8_t   bytes[4];
    } convert;
    convert.lword = size;
    size  = 0;
    for (int i =0; i < 4; i++) {
      size |= convert.bytes[i] << (3-i)*8;
    }
  }
  // Create the item and fill it in:

  CRingItem* pItem = new CRingItem(size);
  ring.get(pItem->m_pItem, size, size); // Read the item from the ring.
  pItem->m_pCursor += (size - sizeof(RingItemHeader));
  pItem->m_swapNeeded = otherOrder;

  return pItem;

}

///////////////////////////////////////////////////////////////////////////////////////
//
// Private utilities.
//


/*
 * Common code for copy construction and assignment,
 * copies the contents of some source item into *this.
 * We assume that the final values for m_pCursor and m_pItem
 * have already been set.
 */
void
CRingItem::copyIn(const CRingItem& rhs)
{
  m_storageSize   = rhs.m_storageSize;
  m_swapNeeded  = rhs.m_swapNeeded;
  memcpy(m_pItem, rhs.m_pItem, m_storageSize + sizeof(RingItemHeader));
}


/*
 *   If necessary, delete dynamically allocated buffer space.
 */
void 
CRingItem::deleteIfNecessary()
{
  if (m_storageSize > CRingItemStaticBufferSize) {
    delete [](reinterpret_cast<uint8_t*>(m_pItem));
  }
}
/*
 *  If necessary, create dynamically allocated buffer space and point
 * m_pItem at it.
 */
void
CRingItem::newIfNecessary(size_t size)
{
  if (size > CRingItemStaticBufferSize) {
    m_pItem  = reinterpret_cast<RingItem*>(new uint8_t[m_storageSize + sizeof(RingItemHeader)]);
  }
  else {
    m_pItem = reinterpret_cast<RingItem*>(m_staticBuffer);
  }
  m_pCursor= m_pItem->s_body;

}
/*
** Swap the bytes of a longword:
*/
uint32_t
CRingItem::swal(uint32_t datum)
{
  union {
    uint32_t along;
    uint8_t  bytes[4];
  } swapper;
  swapper.along = datum;

  uint32_t result = swapper.bytes[3] | (swapper.bytes[2] << 8) | 
    (swapper.bytes[1] << 16) | (swapper.bytes[0] << 24);

}
