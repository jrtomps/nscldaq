/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2009.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/


#ifndef DAQ_V12_CRINGITEMFACTORY_H
#define DAQ_V12_CRINGITEMFACTORY_H

#include <CRingItem.h>
#include <memory>

namespace DAQ {
  namespace V12 {

/**
 * This class is a factory for the correct type of ring item.
 * It is used to effectively up-cast CRingItem base class objects
 * to specific CRingItem derived classes.  Currently the only use case
 * is to be able to use the itemType and toString members to get the correct
 * results for the actual underlying ring item.
 */
class CRingItemFactory
{
public:
  static CRingItemUPtr createRingItem(const CRawRingItem& item);

//  static std::unique_ptr<CRingItem> createRingItem(uint32_t type);

  template<class ByteIterator>
  static CRingItemUPtr createRingItem(ByteIterator beg, ByteIterator end);

  static bool   isKnownItemType(const uint32_t type);
};




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
