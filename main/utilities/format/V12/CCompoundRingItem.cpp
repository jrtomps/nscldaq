

#include <V12/CCompoundRingItem.h>
#include <V12/DataFormat.h>
#include <V12/CRingItemParser.h>
#include <ByteBuffer.h>
#include <ByteOrder.h>
#include <Deserializer2.h>
#include <sstream>
#include <iterator>
#include <utility>
#include <algorithm>

namespace DAQ {
namespace V12 {

CCompoundRingItem::CCompoundRingItem()
: CCompoundRingItem(0, NULL_TIMESTAMP, 0, {}) {}

CCompoundRingItem::CCompoundRingItem(uint32_t type,
                                     uint64_t timestamp,
                                     uint32_t sourceId,
                                     const std::vector<CRingItemPtr>& children)
    : m_type(type), m_evtTimestamp(timestamp), m_sourceId(sourceId), m_children()
{}

CCompoundRingItem::CCompoundRingItem(const CRawRingItem& rawItem)
{

    m_type = rawItem.type();
    m_evtTimestamp = rawItem.getEventTimestamp();
    m_sourceId = rawItem.getSourceId();

    auto& body = rawItem.getBody();

    typedef typename Buffer::ByteBuffer::const_iterator ByteIterator;
    typename Buffer::RangeDeserializer<ByteIterator> stream(body.begin(),
                                                        body.end(),
                                                        rawItem.mustSwap());

    uint32_t type, size, sourceId;
    uint64_t tstamp;

    size_t bytesInBuffer = body.size();
    if (bytesInBuffer < 20) {
        throw std::runtime_error("DAQ::V12::Parser::parseCompound() insufficient data to parse header");
    }

    ByteIterator it = body.begin();
    ByteIterator end = body.end();
    while (it < end) {

        bool swapNeeded;
        uint32_t size, type;
        std::pair<CRingItemUPtr, ByteIterator> result;

        Parser::peekHeaderType(it, end, size, type, swapNeeded);
        if (Parser::isCompound(type)) {
            result = Parser::parseCompound(it, std::min(it+size, end));
        } else {
            result = Parser::parseLeaf(it, std::min(it+size, end));
        }

        appendChild(std::move(std::get<0>(result)));
        it = std::get<1>(result);

    }
}


bool CCompoundRingItem::operator==(const CRingItem& rhs) const
{
    if (m_type != rhs.type()) return false;
    if (m_evtTimestamp != rhs.getEventTimestamp()) return false;
    if (m_sourceId != rhs.getSourceId()) return false;

    const auto pItem = dynamic_cast<const CCompoundRingItem*>(&rhs);
    auto& rhsChildren = pItem->getChildren();
    if (m_children.size() != rhsChildren.size()) return false;

    for (int i=0; i<m_children.size(); ++i) {
        auto pThisChild = m_children[i];
        auto pThatChild = rhsChildren[i];

        if (*pThisChild != *pThatChild) return false;
    }
    return true;
}

bool CCompoundRingItem::operator!=(const CRingItem& rhs) const
{
    return !(*this == rhs);
}


uint32_t CCompoundRingItem::size() const {
    uint32_t size = 20; // we have at least a header
    for (auto pChild : m_children) {
        size += pChild->size();
    }
    return size;
}

uint32_t CCompoundRingItem::type() const {
    return m_type;
}
void CCompoundRingItem::setType(uint32_t type) {
    m_type = type;
}

uint32_t CCompoundRingItem::getSourceId() const {
    return m_sourceId;
}
void CCompoundRingItem::setSourceId(uint32_t id)
{
    m_sourceId = id;
}

uint64_t CCompoundRingItem::getEventTimestamp() const {
    return m_evtTimestamp;
}
void CCompoundRingItem::setEventTimestamp(uint64_t stamp) {
    m_evtTimestamp = stamp;
}

bool CCompoundRingItem::isComposite() const {
    return true;
}
bool CCompoundRingItem::mustSwap() const {
    return false;
}

void CCompoundRingItem::toRawRingItem(CRawRingItem& rawBuffer) const {

    rawBuffer.setType(type() | 0x8000); // set the compound bit in case it was not set
    rawBuffer.setSourceId(getSourceId());
    rawBuffer.setEventTimestamp(getEventTimestamp());

    auto& body = rawBuffer.getBody();
    body.clear();

    body.reserve(size()); // reserves 20 bytes more than necessary, but that is not a big deal.

    for (auto pChild : m_children) {
        CRawRingItem chunk(*pChild);
        body << chunk.size();
        body << chunk.type();
        body << chunk.getEventTimestamp();
        body << chunk.getSourceId();
        body.insert(body.end(), chunk.getBody().begin(), chunk.getBody().end());
    }
}

std::string CCompoundRingItem::typeName() const {
    switch(0x7fff & type()) {
    case BEGIN_RUN:
        return "Begin Run";
    case END_RUN:
        return "End Run";
    case PAUSE_RUN:
        return "Pause Run";
    case RESUME_RUN:
        return "Resume Run";
    case ABNORMAL_ENDRUN:
        return "Abnormal End Run";
    case RING_FORMAT:
        return "Data Format";
    case PACKET_TYPES:
        return "Packet Types";
    case MONITORED_VARIABLES:
        return "Monitored Variables";
    case PERIODIC_SCALERS:
        return "Periodic Scalers";
    case PHYSICS_EVENT:
        return "Physics Event";
    case PHYSICS_EVENT_COUNT:
        return "Physics Event Count";
    case EVB_GLOM_INFO:
        return "Glom Parameters";
    default:
        return "Generic Compound Item";
    }

}

std::string CCompoundRingItem::toString() const {
    std::stringstream result;
    result << "Composite Item" << std::endl;
    result << headerToString(*this);
    result << "Number of subitems: " << m_children.size() << std::endl;

    int index = 0;
    for (auto pChild : m_children) {
        result << "---- Subitem #" << index++ << std::endl;
        result << pChild->toString();
    }
    return result.str();
}

std::vector<CRingItemPtr>& CCompoundRingItem::getChildren() {
    return m_children;
}

const std::vector<CRingItemPtr> CCompoundRingItem::getChildren() const
{
    return m_children;
}

void CCompoundRingItem::appendChild(CRingItemPtr item)
{
    m_children.push_back(item);
}

}
}

