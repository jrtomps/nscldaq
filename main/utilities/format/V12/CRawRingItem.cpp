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

#include "CRawRingItem.h"
#include "DataFormat.h"
#include "Deserializer.h"
#include "ByteOrder.h"
#include <sstream>
#include <iomanip>

namespace DAQ {
  namespace V12 {



  /*!
     * \brief Construct from explicitly provided data
     *
     * \param type          the type (can be anything)
     * \param timestamp     the event timestamp
     * \param sourceId      the source id
     * \param body          the raw bytes to put in the body
     */
    CRawRingItem::CRawRingItem(uint32_t type, uint64_t timestamp, uint32_t sourceId, const Buffer::ByteBuffer& body)
        : m_type(type), m_timestamp(timestamp), m_sourceId(sourceId), m_body(body), m_mustSwap(false) {}

    /*!
     * \brief Default constructor
     *
     * Constructs the data with a VOID type, NULL_TIMESTAMP, 0 source id, and empty body.
     */
    CRawRingItem::CRawRingItem() : CRawRingItem(VOID, NULL_TIMESTAMP, 0, {}) {}


    /*!
     * \brief Construct from a raw data buffer of byte data
     *
     * \param rawData   the raw data buffer
     *
     * The raw data buffer is essentially copied into this item. At first the header
     * is parsed and stored into native byte order no matter what. If the data needs to
     * be swapped, that is recorded for later retrieval via the mustSwap() method.
     *
     * \throws std::runtime_error if the buffer does not contain a complete ring item
     *
     */
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

        Buffer::ContainerDeserializer<Buffer::ByteBuffer> stream(rawData);
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

    /*!
     * \brief Construct from a generic ring item
     *
     * \param rhs   the item to copy into
     *
     */
    CRawRingItem::CRawRingItem(const CRingItem& rhs)
    {
        rhs.toRawRingItem(*this);
    }

    CRawRingItem::~CRawRingItem() {}

    /*!
     * \brief Equality comparison operator
     *
     * \param rhs   the item to compare this to
     *
     * \retval  true if the body, source id, and evt tstamp are all the same
     * \retval false otherwise
     */
    bool CRawRingItem::operator==(const CRingItem& rhs) const {

      if (m_timestamp != rhs.getEventTimestamp()) return false;
      if (m_type != rhs.type()) return false;
      if (m_sourceId != rhs.getSourceId()) return false;
      if (m_mustSwap != rhs.mustSwap()) return false;

      const CRawRingItem* pRawItem = dynamic_cast<const CRawRingItem*>(&rhs);
      if (!pRawItem) return false;
      if (m_body != pRawItem->getBody()) return false;

      return true;
    }

    /*!
     * \brief Inequality comparison operator
     *
     * \param rhs   the item to compare this to
     *
     * \retval true if the body, source id, or evt tstamp is different
     * \retval false otherwise
     */
    bool CRawRingItem::operator!=(const CRingItem& rhs) const {
      return !( *this == rhs );
    }


    /*!
     * \brief Compute the size
     *
     * \return 20 + size of the body
     *
     * This has O(1) complexity.
     */
    uint32_t CRawRingItem::size() const {
        return 3*sizeof(uint32_t)+sizeof(uint64_t)+m_body.size();
    }

    uint32_t CRawRingItem::type() const { return m_type; }
    void CRawRingItem::setType(uint32_t type) { m_type = type; }


    uint64_t CRawRingItem::getEventTimestamp() const { return m_timestamp; }
    void CRawRingItem::setEventTimestamp(uint64_t tstamp) { m_timestamp = tstamp; }

    uint32_t CRawRingItem::getSourceId() const { return m_sourceId; }
    void CRawRingItem::setSourceId(uint32_t id) { m_sourceId = id; }


    /*!
     * \brief Check whether the composite bit (i.e. bit 15) is set
     * \return  true if bit 15 is set in the type
     * \retval false otherwise
     */
    bool   CRawRingItem::isComposite() const {
      return ((m_type & 0x8000)==0x8000);
    }

    /*!
     * \return "RawRingItem"
     */
    std::string CRawRingItem::typeName() const {
      return std::string("RawRingItem");
    }
  
    /*!
     * \brief Convert to a textual representation
     *
     * \return string
     *
     * Because the format of the body is unknown, it is represented as
     * a series of 16-bit words printed in hexadecimal. If the size of the
     * body is an odd number of bytes, the last byte is printed as a single
     * byte without any padding.
     */
    std::string CRawRingItem::toString() const {

      std::ostringstream out;
      uint32_t  bytes = getBodySize();

      Buffer::ContainerDeserializer<Buffer::ByteBuffer> bodyStream(getBody());


      out << headerToString(*this);

      if (mustSwap()) {
          out << "** Data is NOT in native byte order **" << std::endl;
      }

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

    };


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

    /*!
     * \brief Copy contents of this into another raw ring item
     *
     * \param item  the ring item to copy into
     */
    void CRawRingItem::toRawRingItem(CRawRingItem& item) const {
      item = *this;
    }

    /*!
     * \retval false body data is in native byte order
     * \retval true otherwise
     */
    bool CRawRingItem::mustSwap() const {
        return m_mustSwap;
    }

    void CRawRingItem::setMustSwap(bool on) {
      m_mustSwap = on;
    }

  } // end of V12 namespace
} // end DAq
