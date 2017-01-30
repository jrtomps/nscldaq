#include "CRawRingItem.h"
#include "DataFormat.h"
#include "Deserializer.h"
#include "ByteOrder.h"
#include <sstream>
#include <iomanip>

namespace DAQ {
  namespace V12 {


    CRawRingItem::CRawRingItem(uint32_t type, uint64_t timestamp, uint32_t sourceId, const Buffer::ByteBuffer& body)
        : CProductionRingItem(timestamp, sourceId), m_type(type), m_body(body), m_mustSwap(false) {}

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
            setEventTimestamp(swapper.copyAs<uint64_t>(rawTstamp.s_bytes));
            setSourceId(swapper.copyAs<uint32_t>(rawSourceId.s_bytes));
        } else {
            size = rawSize.s_value;
            m_type = rawType.s_value;
            setEventTimestamp(rawTstamp.s_value);
            setSourceId(rawSourceId.s_value);
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
            m_type      = rhs.m_type;
            m_body      = rhs.m_body;
            m_mustSwap  = rhs.m_mustSwap;
            CProductionRingItem::operator=(rhs);
        }

        return *this;
    }

    int CRawRingItem::operator==(const CRawRingItem& rhs) const {
      if (m_type != rhs.m_type) return 0;
      if (getSourceId() != rhs.getSourceId()) return 0;
      if (getEventTimestamp() != rhs.getEventTimestamp()) return 0;
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
