#include "CRawRingItem.h"
#include "DataFormat.h"
#include "Deserializer.h"
#include "ByteOrder.h"
#include <sstream>
#include <iomanip>

namespace DAQ {
  namespace V12 {


    CRawRingItem::CRawRingItem(uint32_t type, uint64_t timestamp, uint32_t sourceId, const Buffer::ByteBuffer& body)
        : m_type(type), m_timestamp(timestamp), m_sourceId(sourceId), m_body(body), m_mustSwap(false) {}

    CRawRingItem::CRawRingItem() : CRawRingItem(VOID, NULL_TIMESTAMP, 0, {}) {}

    CRawRingItem::CRawRingItem(const Buffer::ByteBuffer &rawData) {

        if (rawData.size() < 20) {
            throw std::runtime_error("CRawRingItem::CRawRingItem(Buffer::ByteBuffer&) Buffer contains less data than a header.");
        }

        union IO32 {
            uint32_t s_value;
            char     s_bytes[sizeof(uint32_t)];
        } rawType, rawSize, rawSourceId;

        union IO64 {
            uint64_t s_value;
            char     s_bytes[sizeof(uint64_t)];
        } rawTstamp;

        Buffer::Deserializer<Buffer::ByteBuffer> stream(rawData);
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

        if (rawData.size() < size) {
            throw std::runtime_error("CRawRingItem::CRawRingItem(Buffer::ByteBuffer&) Buffer contains incomplete packet");
        }

        m_body.reserve(size-20);
        m_body.insert(m_body.end(), rawData.begin()+20, rawData.end());

    }

    CRawRingItem::CRawRingItem(const CRingItem& rhs)
    {
        rhs.toRawRingItem(*this);
    }

    CRawRingItem::~CRawRingItem() {}

    CRawRingItem& CRawRingItem::operator=(const CRawRingItem& rhs)
    {
        if (&rhs != this) {
            m_timestamp = rhs.m_timestamp;
            m_type      = rhs.m_type;
            m_sourceId  = rhs.m_sourceId;
            m_body      = rhs.m_body;
            m_mustSwap  = rhs.m_mustSwap;
        }

        return *this;
    }

    int CRawRingItem::operator==(const CRawRingItem& rhs) const {
      if (m_timestamp != rhs.m_timestamp) return 0;
      if (m_type != rhs.m_type) return 0;
      if (m_sourceId != rhs.m_sourceId) return 0;
      if (m_body != rhs.m_body) return 0;
      if (m_mustSwap != rhs.m_mustSwap) return 0;

      return 1;
    }

    int CRawRingItem::operator!=(const CRawRingItem& rhs) const {
      return ( *this == rhs ? 0 : 1 );  
    }

    // Virtual methods that all ring items must provide:
    uint32_t CRawRingItem::size() const { return 3*sizeof(uint32_t)+sizeof(uint64_t)+m_body.size(); }

    uint32_t CRawRingItem::type() const { return m_type; }
    void CRawRingItem::setType(uint32_t type) { m_type = type; }


    uint64_t CRawRingItem::getEventTimestamp() const { return m_timestamp; }
    void CRawRingItem::setEventTimestamp(uint64_t tstamp) { m_timestamp = tstamp; }

    uint32_t CRawRingItem::getSourceId() const { return m_sourceId; }
    void CRawRingItem::setSourceId(uint32_t id) { m_sourceId = id; }

    bool   CRawRingItem::isComposite() const {
      return ((m_type & 0x8000)==0x8000);
    }
    std::string CRawRingItem::typeName() const {
      return std::string("RawRingItem");
    }
  
    // Textual type of item.
    std::string CRawRingItem::toString() const {

      std::ostringstream out;
      uint32_t  bytes = getBodySize();

      Buffer::Deserializer<Buffer::ByteBuffer> bodyStream(getBody());


      out << headerToString(*this);
      out << std::hex << setfill('0');

      uint8_t byte0, byte1;
      int i = 0;
      while (1) {
        bodyStream >> byte0;
        if (bodyStream.eof()) break;
        bodyStream >> byte1;
        if (bodyStream.eof()) {
            // odd number of bytes.
            // this only happens at the end, so we don't need
            // to worry about line endings
            out << std::setw(2) << int(byte0) << " ";
        } else {
            if ( (i%8) == 0 && i!=0) {
                out << std::endl;
            }

            uint16_t value = byte1;
            value = (value<<8) | byte0;
            out << std::setw(4) << value << " ";

            i++;
        }
      }
      out << std::endl;

      return out.str();

    }; // Provide string dump of the item.

    uint32_t CRawRingItem::getBodySize() const {
      return m_body.size();
    }
    uint32_t CRawRingItem::getStorageSize() const {
      return m_body.capacity();
    }

    Buffer::ByteBuffer& CRawRingItem::getBody() {
      return m_body;
    }

    const Buffer::ByteBuffer& CRawRingItem::getBody() const {
      return m_body;
    }


    void CRawRingItem::toRawRingItem(CRawRingItem& item) const {
      item = *this;
    }

    bool CRawRingItem::mustSwap() const {
        return m_mustSwap;
    }

    void CRawRingItem::setMustSwap(bool on) {
      m_mustSwap = on;
    }

  } // end of V12 namespace
} // end DAq
