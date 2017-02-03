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

#ifndef DAQ_V12_CPHYSICSEVENTITEM_H
#define DAQ_V12_CPHYSICSEVENTITEM_H

#include <V12/CRawRingItem.h>

namespace DAQ {
  namespace V12 {

  /*!
 * \brief The CPhysicsEventItem class
 *
 * The physics event is a leaf type that has an amorphous set of data in its body.
 * In essence it is just a raw ring item. The PhysicsEvent class is mainly present
 * so that the typeName() method can be overridden to return a different value.
 *
 * The CPhysicsEventItem is always a leaf item. It may or may not require byte swapping,
 * so anyone parsing the body should be careful to treat it properly. The
 * Buffer::ContainerDeserializer class is useful for this purpose. For example:
 *
 * \code
 * #include <V12/CPhysicsEventItem.h>
 * #include <Deserializer.h>
 *
 * CRawRingItem rawItem(someDataBuffer);
 * CPhysicsEventItem item(rawItem);
 *
 * // create a deserializer to properly handle the byte swapping
 * Buffer::Deserializer<Buffer::ByteBuffer>
 *       bodyStream(rawItem.getBody(), rawItem.mustSwap());
 *
 * // Read and print out the body in 16-bit chunks until there is no more data
 * uint16_t value;
 * bodyStream >> value;
 * while (!bodyStream.eof()) {
 *    std::cout << value << std::endl;
 *    bodyStream >> value;
 * }
 *
 * \endcode
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

  virtual bool isComposite() const;

};


} // end of V12 namespace
} // end DAQ

#endif
