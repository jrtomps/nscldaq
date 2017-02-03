/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2005.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt
*/
/**
 * @file CDataFormatItem.h
 * @brief Implement the CDataFormatItem a class for producing data format
 *        ring items.
 * @author Ron Fox<fox@nscl.msu.edu>
 */

#include "V12/CDataFormatItem.h"
#include "V12/DataFormat.h"
#include "V12/CRawRingItem.h"
#include "Deserializer.h"
#include "ByteBuffer.h"

#include <sstream>

namespace DAQ {
  namespace V12 {

/*-----------------------------------------------------------------------------
 * Canonicals
 *---------------------------------------------------------------------------*/

/**
 * constructor
 *
 *   Constructs a data format ring item whose payload contains the current
 *   data format major and minor versions encoded in DataFormat.h
 */
CDataFormatItem::CDataFormatItem()
 : CDataFormatItem(NULL_TIMESTAMP, 0, FORMAT_MAJOR, FORMAT_MINOR)
{}

/*!
 * \brief Constructs data format ring item with information provided explicitly
 *
 * \param tstamp        event timestamp
 * \param sourceId      the source id
 * \param major         major version
 * \param minor         minor version
 */
CDataFormatItem::CDataFormatItem(uint64_t tstamp, uint32_t sourceId, uint16_t major, uint16_t minor)
    : m_evtTimestamp(tstamp),
      m_sourceId(sourceId),
      m_major(major),
      m_minor(minor)
{}


/*!
 * \brief Construct from raw data
 *
 * \param rawItem   the raw data item
 *
 * Byte swapping is handled automatically.
 *
 * \throws std::runtime_error if insufficient data is provided in the body of raw ring item
 */
CDataFormatItem::CDataFormatItem(const CRawRingItem& rawItem)
{
    m_evtTimestamp = rawItem.getEventTimestamp();
    m_sourceId = rawItem.getSourceId();

    auto& body = rawItem.getBody();

    if (body.size() < 4) {
        throw std::runtime_error("CDataFormatItem(const CRawRingItem&) cannot construct from incomplete data buffer");
    }
    Buffer::ContainerDeserializer<Buffer::ByteBuffer> bodyStream(body, rawItem.mustSwap());

    bodyStream >> m_major >> m_minor;
}

CDataFormatItem::~CDataFormatItem() {}

/**
 * \brief Equality comparison operator
 *
 *   @param rhs - Item to compare to *this.
 *
 *   @return true if major, minor, tstamp, and source id are the same
 */
bool
CDataFormatItem::operator==(const CRingItem& rhs) const
{
    if (m_evtTimestamp != rhs.getEventTimestamp()) return false;
    if (m_sourceId != rhs.getSourceId()) return false;

    auto pItem = dynamic_cast<const CDataFormatItem*>(&rhs);
    if (!pItem) {
        return false;
    }

    if (m_major != pItem->getMajor()) return false;
    if (m_minor != pItem->getMinor()) return false;

    return true;
}

/**
 * \brief Inequality comparison operator
 *
 * @param rhs - Item to compare to *this.
 *
 * @return true if major, minor, source id, or tstamp is different
 * \retval false otherwise
 */
bool
CDataFormatItem::operator!=(const CRingItem& rhs) const
{
    return !(this->operator==(rhs));
}

/*!
 * \return 24 (always the same number of bytes)
 */
uint32_t CDataFormatItem::size() const {
    return 24;
}

/*!
 * \return V12::RING_FORMAT
 */
uint32_t CDataFormatItem::type() const {
    return RING_FORMAT;
}

/*!
 * \brief Set the type
 *
 * \param type  must be V12::RING_FORMAT
 *
 * \throws std::invalid_argument if user passes type other than V12::RING_FORMAT
 */
void CDataFormatItem::setType(uint32_t type) {
    if (type != RING_FORMAT) {
        throw std::invalid_argument("CDataFormatItem::setType() argument must be RING_FORMAT");
    }
}

uint64_t CDataFormatItem::getEventTimestamp() const {
    return m_evtTimestamp;
}

void CDataFormatItem::setEventTimestamp(uint64_t tstamp) {
    m_evtTimestamp = tstamp;
}

uint32_t CDataFormatItem::getSourceId() const {
    return m_sourceId;
}

void CDataFormatItem::setSourceId(uint32_t id) {
    m_sourceId = id;
}


/*!
 * CDataFormatItems are always leaf types
 *
 * \return false
 */
bool CDataFormatItem::isComposite() const {
    return false;
}

/*!
 * CDataFormatItems are always stored in native byte order
 *
 * \return false
 */
bool CDataFormatItem::mustSwap() const {
    return false;
}

/*!
 * \brief Serialize data into a raw ring item
 * \param item  the raw ring item to fill with the data
 */
void CDataFormatItem::toRawRingItem(CRawRingItem& item) const
{
    item.getBody().clear();

    item.setType(type());
    item.setEventTimestamp(getEventTimestamp());
    item.setSourceId(getSourceId());

    item.getBody() << m_major << m_minor;

}


/**
 * major
 *
 * Returns the format major revision level.
 *
 * @return uint16_t
 */
uint16_t
CDataFormatItem::getMajor() const
{
    return m_major;
}
/**
 * minor
 *
 * Returns the format minor version.
 *
 * @return uint16_t
 */
uint16_t
CDataFormatItem::getMinor() const
{
    return m_minor;
}

/**
 * typeName
 *
 * @return std::string the data type  as a string.
 */
std::string
CDataFormatItem::typeName() const
{
    return std::string("Data Format");
}
/**
 * toString
 *
 * @return std::string - text version of the body of the event.
 */
std::string
CDataFormatItem::toString() const
{
    std::ostringstream out;
    
    out << headerToString(*this);

    out << "Data Version : " << m_major << '.'
        << m_minor << std::endl;
        
    return out.str();
}


  } // end of V12 namespace
} // end DAQ
