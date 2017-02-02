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
 : CDataFormatItem(NULL_TIMESTAMP, 0, 12, 0)
{}

CDataFormatItem::CDataFormatItem(uint64_t tstamp, uint32_t sourceId, uint16_t major, uint16_t minor)
    : m_evtTimestamp(tstamp),
      m_sourceId(sourceId),
      m_major(major),
      m_minor(minor)
{}


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
 * operator==
 *
 *   Compares for equality.
 *
 *   @param rhs - Item to compare to *this.
 *
 *   @return int - nonzero if equal.
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
 * operator!=
 *
 * @param rhs - Item to compare to *this.l
 *
 * @return int  non zero if unequal.
 */
bool
CDataFormatItem::operator!=(const CRingItem& rhs) const
{
    return !(this->operator==(rhs));
}


uint32_t CDataFormatItem::size() const {
    return 24;
}

uint32_t CDataFormatItem::type() const {
    return RING_FORMAT;
}

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

bool CDataFormatItem::isComposite() const {
    return false;
}

bool CDataFormatItem::mustSwap() const {
    return false;
}

void CDataFormatItem::toRawRingItem(CRawRingItem& item) const
{

    item.setType(type());
    item.setEventTimestamp(getEventTimestamp());
    item.setSourceId(getSourceId());

    item.getBody() << m_major << m_minor;

}

/*----------------------------------------------------------------------------
 * Getters.
 */

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

/*----------------------------------------------------------------------------
 * object methods:
 *--------------------------------------------------------------------------*/
/**
 * typeName
 *
 * @return std::string the data type  as a string.
 */
std::string
CDataFormatItem::typeName() const
{
    return std::string("Ring Item format version");
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
    
    out << "Ring items formatted for: " << m_major << '.'
        << m_minor << std::endl;
        
    return out.str();
}


  } // end of V12 namespace
} // end DAQ
