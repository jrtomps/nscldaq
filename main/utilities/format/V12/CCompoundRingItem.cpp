

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

CCompositeRingItem::CCompositeRingItem()
: CCompositeRingItem(0, NULL_TIMESTAMP, 0, {}) {}

CCompositeRingItem::CCompositeRingItem(uint32_t type,
                                     uint64_t timestamp,
                                     uint32_t sourceId,
                                     const std::vector<CRingItemPtr>& children)
    : m_type(type), m_evtTimestamp(timestamp), m_sourceId(sourceId), m_children()
{}

CCompositeRingItem::CCompositeRingItem(const CRawRingItem& rawItem)
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
        throw std::runtime_error("DAQ::V12::Parser::parseComposite() insufficient data to parse header");
    }

    ByteIterator it = body.begin();
    ByteIterator end = body.end();
    while (it < end) {

        bool swapNeeded;
        uint32_t size, type;
        std::pair<CRingItemUPtr, ByteIterator> result;

        Parser::peekHeaderType(it, end, size, type, swapNeeded);
        if (Parser::isComposite(type)) {
            result = Parser::parseComposite(it, std::min(it+size, end));
        } else {
            result = Parser::parseLeaf(it, std::min(it+size, end));
        }

        appendChild(std::move(std::get<0>(result)));
        it = std::get<1>(result);
    }

    if ( ! Parser::isTypeConsistent(*this, m_type)) {
        throw std::runtime_error("CCompositeRingItem(CRawRingItem&) type consistency was violated");
    }
}


bool CCompositeRingItem::operator==(const CRingItem& rhs) const
{
    if (m_type != rhs.type()) return false;
    if (m_evtTimestamp != rhs.getEventTimestamp()) return false;
    if (m_sourceId != rhs.getSourceId()) return false;

    const auto pItem = dynamic_cast<const CCompositeRingItem*>(&rhs);
    auto& rhsChildren = pItem->getChildren();
    if (m_children.size() != rhsChildren.size()) return false;

    for (int i=0; i<m_children.size(); ++i) {
        auto pThisChild = m_children[i];
        auto pThatChild = rhsChildren[i];

        if (*pThisChild != *pThatChild) return false;
    }
    return true;
}

bool CCompositeRingItem::operator!=(const CRingItem& rhs) const
{
    return !(*this == rhs);
}


uint32_t CCompositeRingItem::size() const {
    uint32_t size = 20; // we have at least a header
    for (auto pChild : m_children) {
        size += pChild->size();
    }
    return size;
}

uint32_t CCompositeRingItem::type() const {
    return m_type;
}
void CCompositeRingItem::setType(uint32_t type) {
    m_type = type;
}

uint32_t CCompositeRingItem::getSourceId() const {
    return m_sourceId;
}
void CCompositeRingItem::setSourceId(uint32_t id)
{
    m_sourceId = id;
}

uint64_t CCompositeRingItem::getEventTimestamp() const {
    return m_evtTimestamp;
}
void CCompositeRingItem::setEventTimestamp(uint64_t stamp) {
    m_evtTimestamp = stamp;
}

bool CCompositeRingItem::isComposite() const {
    return true;
}
bool CCompositeRingItem::mustSwap() const {
    return false;
}

void CCompositeRingItem::toRawRingItem(CRawRingItem& rawBuffer) const {

    rawBuffer.setType(type() | 0x8000); // set the Composite bit in case it was not set
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

std::string CCompositeRingItem::typeName() const {
    switch(0x7fff & type()) {
    case BEGIN_RUN:
        return "Composite Begin Run";
    case END_RUN:
        return "Composite End Run";
    case PAUSE_RUN:
        return "Composite Pause Run";
    case RESUME_RUN:
        return "Composite Resume Run";
    case ABNORMAL_ENDRUN:
        return "Composite Abnormal End Run";
    case RING_FORMAT:
        return "Composite Data Format";
    case PACKET_TYPES:
        return "Composite Packet Types";
    case MONITORED_VARIABLES:
        return "Composite Monitored Variables";
    case PERIODIC_SCALERS:
        return "Composite Periodic Scalers";
    case PHYSICS_EVENT:
        return "Composite Physics Event";
    case PHYSICS_EVENT_COUNT:
        return "Composite Physics Event Count";
    case EVB_GLOM_INFO:
        return "Composite Glom Parameters";
    default:
        return "Generic Composite Item";
    }

}

std::string CCompositeRingItem::toString() const {
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

std::vector<CRingItemPtr>& CCompositeRingItem::getChildren() {
    return m_children;
}

const std::vector<CRingItemPtr> CCompositeRingItem::getChildren() const
{
    return m_children;
}

void CCompositeRingItem::appendChild(CRingItemPtr item)
{
    m_children.push_back(item);
}

}
}

