/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2005.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt
*/

/**
 * @file CUnknownFragment.cpp
 * @brief Implements the CUnknownFragment calss for EVB_UNKNOWN_PAYLOAD ring items.
 * @author Ron Fox <fox@nscl.msu.edu>
 */

#include "CUnknownFragment.h"
#include "DataFormat.h"
#include <string.h>
#include <sstream>


/*-----------------------------------------------------------------------------
 * Canonical methods.
 *---------------------------------------------------------------------------*/

/**
 * constructor
 *
 * This is the primary constructor.
 *
 * @param timestamp - Ticks that identify when this fragment was triggered.
 * @param sourceId  - Id of the source that created this fragment.
 * @param barrier   - Barrier id of the fragment o 0 if this was not part of a
 *                    barrier.
 * @param size      - Number of _bytes_ in the payload.
 * @param pPayload  - Pointer to the payload.
 */
CUnknownFragment::CUnknownFragment(
    uint64_t timestamp, uint32_t sourceId, uint32_t barrier, size_t size,
    void* pPayload) :
        CRingItem(
            EVB_UNKNOWN_PAYLOAD, timestamp, sourceId, barrier, itemSize(size))
{
    // fill in the payload as the CRingItem constructor will have filled in
    // the header:
    
    uint8_t* pDest = reinterpret_cast<uint8_t*>(getBodyCursor());
    memcpy(pDest, pPayload, size);
    
    // Update the body cursor and set the item size:
    
    setBodyCursor(pDest + size);
    updateSize();
}
/**
 * destructor
 */
CUnknownFragment::~CUnknownFragment()  {}

/**
 * copy constructor (specific)
 *
 * @param rhs - The object item we are copying in construction.
 */
CUnknownFragment::CUnknownFragment(const CUnknownFragment& rhs) :
    CRingItem(rhs)
{
    
}
/**
 * copy constuctor (generic)
 *
 * @param rhs - The object item we are copying in construction.
 * @throw std::bad_cast if the rhs object is ot a EVB_UNKNOWN_PAYLOAD type.
 */
CUnknownFragment::CUnknownFragment(const CRingItem& rhs) throw(std::bad_cast) :
    CRingItem(rhs)
{
    if (type() != EVB_UNKNOWN_PAYLOAD) throw std::bad_cast();        
}
/**
 * operator=
 *
 * @param rhs - The object that we are being assigned from.
 * @return CUnknownFragment& reference to *this.
 */
CUnknownFragment&
CUnknownFragment::operator=(const CUnknownFragment& rhs)
{
    CRingItem::operator=(rhs);
    return *this;
}
/**
 * operator==
 *    @param rhs - object of comparison.
 *    @return int - nonzero if equality.
 */
int
CUnknownFragment::operator==(const CUnknownFragment& rhs) const
{
    return CRingItem::operator==(rhs);
}
/**
 * operator!=
 * @param rhs - Object of comparison.
 * @return int - nonzero if items don't compare for equality.
 */
int
CUnknownFragment::operator!=(const CUnknownFragment& rhs) const
{
    return CRingItem::operator!=(rhs);
}
/*---------------------------------------------------------------------------
 *  Selectors
 *--------------------------------------------------------------------------*/

/**
 * payloadSize
 *
 * Figure out how big the payload is.  This is just the size of the item
 * with the size of a ring item and a body header removed.
 *
 * @return size_t
 */
size_t
CUnknownFragment::payloadSize() const
{
    return getBodySize();
}
/**
 * payloadPointer
 *
 * Return a pointer to the first byte of the payload
 *
 * @return void*
 */
const void*
CUnknownFragment::payloadPointer() const
{
    return const_cast<const void*>(getBodyPointer());
}
/*----------------------------------------------------------------------------
 * Virtual method overrides;
 *--------------------------------------------------------------------------*/

/**
 * typeName
 *
 * @return std::string textual version of item type.
 */
std::string
CUnknownFragment::typeName() const
{
    return "Fragment with unknown payload";
}
/**
 * toString
 *
 * @return std::string - item converted to string.
 */
std::string
CUnknownFragment::toString() const
{
    return CRingItem::toString();
}
/*-------------------------------------------------------------------------
 * Private utilities
 *-----------------------------------------------------------------------*/

/**
 * itemSize(size_t payloadSize)
 *
 * @param payloadSize - Size of the payload data.
 * @return size_t     - Number of bytes in full item.
 */
size_t
CUnknownFragment::itemSize(size_t payloadSize) const
{
    return payloadSize + sizeof(RingItemHeader) + sizeof(BodyHeader);
}