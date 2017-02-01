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

#include "V12/CPhysicsEventItem.h"
#include "V12/DataFormat.h"
#include <sstream>
#include <stdio.h>

#include <iostream>

namespace DAQ {
  namespace V12 {

  /*
 * All the canonical methods just delegate to the base class
 */


  CPhysicsEventItem::CPhysicsEventItem() :
      CRawRingItem() { setType(PHYSICS_EVENT);}
  
  CPhysicsEventItem::CPhysicsEventItem(
          uint64_t timestamp, uint32_t source, const Buffer::ByteBuffer& body) :
      CRawRingItem(PHYSICS_EVENT, timestamp, source, body)
  {

  }

  CPhysicsEventItem::CPhysicsEventItem(const CRawRingItem& rhs)
      : CRawRingItem(rhs)
  {
      if (type() != PHYSICS_EVENT) {
          throw std::bad_cast();
      }
  }


  CPhysicsEventItem::CPhysicsEventItem(const CPhysicsEventItem& rhs) :
      CRawRingItem(rhs) {}

  CPhysicsEventItem::~CPhysicsEventItem() {}

  CPhysicsEventItem&
  CPhysicsEventItem::operator=(const CPhysicsEventItem& rhs)
  {
    CRawRingItem::operator=(rhs);
    return *this;
  }

  bool
  CPhysicsEventItem::operator==(const CRingItem& rhs) const
  {
    return CRawRingItem::operator==(rhs);
  }

  bool
  CPhysicsEventItem::operator!=(const CRingItem& rhs) const
  {
    return CRawRingItem::operator!=(rhs);
  }

  /*--------------------------------------------------
 *
 * Virtual method overrides.
 */

  /**
 * typeName
 *    Returns the type name associated with the item.
 *
 * @return std::string  - "Event".
 */
  std::string
  CPhysicsEventItem::typeName() const
  {
      return "Physics Event";
  }


  } // end of V12 namespace
} // end of DAQ
