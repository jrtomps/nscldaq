/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2017.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
     Jeromy Tompkins
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

#include "V12/CPhysicsEventItem.h"
#include "V12/DataFormat.h"

namespace DAQ {
  namespace V12 {

  /*!
   * \brief Construct an empty physics event
   */
  CPhysicsEventItem::CPhysicsEventItem() :
      CRawRingItem() { setType(PHYSICS_EVENT);}
  
  /*!
   * \brief Explicitly construct a physics event
   * \param timestamp   the event timestamp
   * \param source      the source id
   * \param body        the raw data to store as the body
   */
  CPhysicsEventItem::CPhysicsEventItem(
          uint64_t timestamp, uint32_t source, const Buffer::ByteBuffer& body) :
      CRawRingItem(PHYSICS_EVENT, timestamp, source, body)
  {
  }

  /*!
   * \brief Construct from a raw ring item
   *
   * \param rhs the raw ring item
   *
   *
   */
  CPhysicsEventItem::CPhysicsEventItem(const CRawRingItem& rhs)
      : CRawRingItem(rhs)
  {
      if (type() != PHYSICS_EVENT) {
          throw std::bad_cast();
      }
  }

  CPhysicsEventItem::~CPhysicsEventItem() {}

  /*!
   * \brief Equality comparison operator
   *
   * \param rhs the ring item to compare to
   *
   * \retval true if the body, event timestamp, type, and source id are the same
   * \retval false otherwise
   */
  bool
  CPhysicsEventItem::operator==(const CRingItem& rhs) const
  {
    return CRawRingItem::operator==(rhs);
  }

  /*!
   * \brief Inequality comparison operator
   *
   * \param rhs the ring item to compare to
   *
   * \retval true if the body, event timestamp, type, or source id is different
   * \retval false otherwise
   */
  bool
  CPhysicsEventItem::operator!=(const CRingItem& rhs) const
  {
    return CRawRingItem::operator!=(rhs);
  }

  /**
 * typeName
 *    Returns the type name associated with the item.
 *
 * @return std::string  - "Physics Event".
 */
  std::string
  CPhysicsEventItem::typeName() const
  {
      return "Physics Event";
  }

  /*!
   * \brief CPhysicsEventItem::setType
   *
   * \param type    must be V12::PHYSICS_EVENT
   *
   * \throws std::invalid_argument if type is V12::PHYSICS_EVENT
   */
  void CPhysicsEventItem::setType(uint32_t type) {
      if (type != PHYSICS_EVENT) {
          throw std::invalid_argument("CPhysicsEventItem::setType() must be provided type PHYSICS_EVENT");
      }
  }

  /*!
   * \return false (Physics events are always leaf types)
   */
  bool CPhysicsEventItem::isComposite() const {
      return false;
  }

  } // end of V12 namespace
} // end of DAQ
