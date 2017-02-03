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

#ifndef DAQ_V12_CRAWRINGITEM_H
#define DAQ_V12_CRAWRINGITEM_H

#include "V12/CRingItem.h"
#include <V12/DataFormat.h>

#include <ByteBuffer.h>
#include <Deserializer2.h>

namespace DAQ {
namespace V12 {

/*!
 * \brief The CRawRingItem class
 *
 * The CRawRingItem class is the most general class of the version 12.0 data
 * format classes. It is intended to represent any type of ring item as a blob of
 * byte data. Every ring item has a gauranteed header of source id, type, size, and
 * event timestamp. The body of each item, if present, depends on the type. However,
 * we can store it as a serialized set of bytes. The CRawRingItem is for this purpose.
 * In fact, we intend for it to be used for IO type purposes.
 *
 * \code
 * #include <V12/CRawRingItem.h>
 *
 * using namespace DAQ::V12;
 *
 * CRawRingItem rawItem;
 * std::cin >> rawItem;
 *
 * // from here we can construct any ring item
 * // here are a few methods to do so:
 * // 1. Use the CRingItemFactory
 * CRingItemPtr pItem = RingItemFactory::createRingItem(rawItem);
 *
 * // 2. Use the V12::format_cast. This may fail at runtime.
 * CRingScalerItem item = format_cast<CRingScalerItem>(rawItem);
 *
 * // 3. Explicit construction
 * CRingScalerItem item(rawItem);
 *
 * // then maybe you will want to go the other way. that can be done
 * // two ways:
 * // 1. Use the toRawRingItem
 * CRawRingItem toOutput;
 * pItem->toRawRingItem(toOutput);
 *
 * // 2. Use format cast
 * CRawRingItem toOutput = format_cast<CRawRingItem>(*pItem);
 *
 * \endcode
 *
 * The other interesting thing about the CRawRingItem is that its type can be both a
 * composite or a leaf item. This is different than all other classes that
 * derive from the CRingItem base class. The body of the object may have a different
 * endianness than the native machine.
 */
class CRawRingItem : public CRingItem {

private:
  uint32_t            m_type;
  uint32_t            m_sourceId;
  uint64_t            m_timestamp;
  Buffer::ByteBuffer  m_body;
  bool                m_mustSwap;

public:
  explicit CRawRingItem();
  explicit CRawRingItem(uint32_t type, uint64_t timestamp, uint32_t sourceId, const Buffer::ByteBuffer& body=Buffer::ByteBuffer());
  explicit CRawRingItem(const Buffer::ByteBuffer& rawData);

  template<class ByteIterator> CRawRingItem(ByteIterator beg, ByteIterator end);

  explicit CRawRingItem(const CRingItem& rhs);
  virtual ~CRawRingItem();

  CRawRingItem& operator=(const CRawRingItem& rhs) = default;
  virtual bool operator==(const CRingItem& rhs) const;
  virtual bool operator!=(const CRingItem& rhs) const;

  // Virtual methods that all ring items must provide:
  virtual uint32_t size() const;

  virtual uint32_t type() const;
  virtual void setType(uint32_t type);

  virtual uint64_t getEventTimestamp() const;
  virtual void     setEventTimestamp(uint64_t tstamp);

  virtual uint32_t getSourceId() const;
  virtual void     setSourceId(uint32_t id);

  virtual bool     isComposite() const;
  virtual bool     mustSwap() const;
  virtual void     setMustSwap(bool on);

  virtual std::string typeName() const;	// Textual type of item.
  virtual std::string toString() const; // Provide string dump of the item.

  uint32_t getBodySize() const;
  uint32_t getStorageSize() const;

  Buffer::ByteBuffer& getBody();
  const Buffer::ByteBuffer& getBody() const;

  void toRawRingItem(CRawRingItem& item) const;

  template<class T> std::unique_ptr<T> as() const;

};


template<class ByteIterator> CRawRingItem::CRawRingItem(ByteIterator beg, ByteIterator end)
{
    if (std::distance(beg, end) < 20) {
        throw std::runtime_error("CRawRingItem::CRawRingItem<ByteIter>(ByteIter,ByteIter) Buffer contains less data than a header.");
    }

    union IO32 {
        uint32_t s_value;
        char     s_bytes[sizeof(uint32_t)];
    } rawType, rawSize, rawSourceId;

    union IO64 {
        uint64_t s_value;
        char     s_bytes[sizeof(uint64_t)];
    } rawTstamp;

    typename Buffer::RangeDeserializer<ByteIterator> stream(beg, end);
    stream >> rawSize.s_value;
    stream >> rawType.s_value;
    stream >> rawTstamp.s_value;
    stream >> rawSourceId.s_value;

    m_mustSwap = (((rawType.s_value & 0xffff) == 0) && (rawType.s_value != V12::VOID));

    uint32_t size;
    if (m_mustSwap) {
        BO::CByteSwapper swapper(true);
        size = swapper.copyAs<uint32_t>(rawSize.s_bytes);
        m_type = swapper.copyAs<uint32_t>(rawType.s_bytes);
        m_timestamp = swapper.copyAs<uint64_t>(rawTstamp.s_bytes);
        m_sourceId = swapper.copyAs<uint32_t>(rawSourceId.s_bytes);
    } else {
        size = rawSize.s_value;
        m_type = rawType.s_value;
        m_timestamp = rawTstamp.s_value;
        m_sourceId = rawSourceId.s_value;
    }

    if (std::distance(beg, end) < size) {
        throw std::runtime_error("CRawRingItem::CRawRingItem<ByteIter>(ByteIter, ByteIter) Buffer contains incomplete packet");
    }

    m_body.reserve(size-20);
    m_body.insert(m_body.end(), stream.pos(), stream.end());
}




} // end of V12 namespace
} // end DAQ



/*! \brief Convert to a CRingItem
 *
 * All CRingItem-derived types must able to construct from a CRawRingItem.
 */
template<class T>
std::unique_ptr<T> DAQ::V12::CRawRingItem::as() const
{
  return std::unique_ptr<T>(new T(*this));
}

#endif
