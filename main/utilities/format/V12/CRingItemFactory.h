/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2017.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
        Jeromy Tompkins
         NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/


#ifndef DAQ_V12_CRINGITEMFACTORY_H
#define DAQ_V12_CRINGITEMFACTORY_H

#include <CRingItem.h>

namespace DAQ {
  namespace V12 {

/**
 * \brief A factory to construct ring items
 *
 * The factory provides a mechanism  to convert raw data into the
 * appropriate derived type from raw data.
 *
 */
class CRingItemFactory
{
public:
  static CRingItemUPtr createRingItem(const CRawRingItem& item);

  template<class ByteIterator>
  static CRingItemUPtr createRingItem(ByteIterator beg, ByteIterator end);

  static bool   isKnownItemType(const uint32_t type);
};

/*! \brief Construct a ring item from a range of contiguous byte data
 *
 * \param beg   points to data representing the beginning of the ring item (i.e. the size)
 * \param end   points to the address immediately after valid contiguous byte data
 *
 *
 * At present, this is a bit wasteful of CPU cycles because it performs an extra copy.
 * It would be much more efficient if we constructed directly from the range.
 *
 * In any case, it is expected that the range of bytes [beg, end), represent
 * a complete ring item. It is not an error for the range to be larger than the
 * ring item pointed to by beg. In that case, only the first ring item is extracted
 * and returned.
 *
 * The returned item is a CRingItemUPtr(i.e. unique_ptr), so ownership is explicitly passed to the caller.
 * Typically, users will want to immediately convert this to a CRingItemPtr (i.e. shared_ptr), because
 * that is more in line with how people typically will use it. The semantics
 * of a CRingItemPtr are much more natural and simpler. Here is the recommended
 * usage:
 *
 * \code
 * #include "V12/CRingITemFactory.h"
 *
 * using namespace DAQ::V12;
 *
 * CRingItemPtr pItem = CRingItemFactory::createRingItem(buffer.begin(), buffer.end());
 *
 * \endcode
 *
 * The fact that this is a templated function means that you can call it with any
 * iterator type that refers to byte data. In other words, char*, unsigned char*,
 * const char*, const unsigned char*, uint8_t*, const uint8_t*, std::vector<char>::iterator,
 * std::array<char>::iterator, etc... will all work.
 *
 */
template<class ByteIterator>
CRingItemUPtr CRingItemFactory::createRingItem(ByteIterator beg,
                                               ByteIterator end)
{
    CRawRingItem rawItem(beg,end);
    return createRingItem(rawItem);
}





} // end of V12 namespace
} // end DAQ
#endif
