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

#ifndef NSCLDAQV12_CPHYSICSEVENTITEM_H
#define NSCLDAQV12_CPHYSICSEVENTITEM_H

#include <V12/CRawRingItem.h>

namespace DAQ {
  namespace V12 {

  /*!
 * \brief The CPhysicsEventItem class
 *
 * The physics event is a leaf type that has an amorphous set of data in its body.
 * In essence it is just a raw ring item. The PhysicsEvent class is mainly present
 * so that the typeName() method can be overridden to return a different value.
 */
class CPhysicsEventItem : public CRawRingItem
{
public:
  CPhysicsEventItem();
  CPhysicsEventItem(uint64_t timestamp, uint32_t source, const Buffer::ByteBuffer& body = Buffer::ByteBuffer() );

  CPhysicsEventItem(const CRawRingItem& rhs);
  CPhysicsEventItem(const CPhysicsEventItem& rhs) = default;
  virtual ~CPhysicsEventItem();

  CPhysicsEventItem& operator=(const CPhysicsEventItem& rhs) = default;
  virtual bool operator==(const CRingItem& rhs) const;
  virtual bool operator!=(const CRingItem& rhs) const;

  // Virtual methods that all ring items must provide:

  virtual std::string typeName() const;	// Textual type of item.
  
  virtual void setType(uint32_t type);
};


} // end of V12 namespace
} // end DAQ

#endif
