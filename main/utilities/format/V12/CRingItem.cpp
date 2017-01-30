
#include <V12/CRingItem.h>
#include <V12/DataFormat.h>
#include <iostream>
#include <sstream>


namespace DAQ {
namespace V12 {


CProductionRingItem::CProductionRingItem() : CProductionRingItem(NULL_TIMESTAMP, 0, {}) {}

CProductionRingItem::CProductionRingItem(uint64_t tstamp, uint32_t sourceId, const std::vector<CRingItemPtr>& children)
  : m_evtTimestamp(tstamp), m_sourceId(sourceId), m_children(children)
  {}

CProductionRingItem& CProductionRingItem::operator=(const CProductionRingItem& rhs)
{
  if (this != &rhs) {
    m_evtTimestamp = rhs.m_evtTimestamp;
    m_sourceId = rhs.m_sourceId;
  }

  return *this;
}

uint32_t CProductionRingItem::getSourceId() const
{
    return m_sourceId;
}

void CProductionRingItem::setSourceId(uint32_t sourceId) {
    m_sourceId = sourceId;
}

uint64_t CProductionRingItem::getEventTimestamp() const
{
    return m_evtTimestamp;
}

void CProductionRingItem::setEventTimestamp(uint64_t tstamp)
{
    m_evtTimestamp = tstamp;
}

const std::vector<CRingItemPtr>& CProductionRingItem::getChildren() const
{
    return m_children;
}

std::vector<CRingItemPtr>& CProductionRingItem::getChildren()
{
    return m_children;
}



std::string headerToString(const DAQ::V12::CRingItem &item) {
    std::ostringstream result;
    result << "Size (bytes): " << item.size() << std::endl;
    result << "Type:         " << item.typeName() << std::endl;
    result << "Timestamp:    " << item.getEventTimestamp() << std::endl;
    result << "Source Id:    " << item.getSourceId()  << std::endl;

    return result.str();
    }

} // end V12 namespace
} // end DAQ namespace
