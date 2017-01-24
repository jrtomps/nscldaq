
#ifndef NSCLDAQV12_CRAWRINGITEM_H
#define NSCLDAQV12_CRAWRINGITEM_H 1

#include "CRingItem.h"
#include <ByteBuffer.h>
#include <memory>

namespace DAQ {
namespace V12 {

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
  explicit CRawRingItem(const CRingItem& rhs);
  virtual ~CRawRingItem();

  CRawRingItem& operator=(const CRawRingItem& rhs);
  int operator==(const CRawRingItem& rhs) const;
  int operator!=(const CRawRingItem& rhs) const;

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
