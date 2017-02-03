

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



/*!
 * \brief Default constructor
 *
 * The default constructor sets the source ID to 0, type to VOID, and
 * the timestamp to NULL_TIMESTAMP. There are no children.
 */
CCompositeRingItem::CCompositeRingItem()
: CCompositeRingItem(VOID | 0x8000, NULL_TIMESTAMP, 0, {}) {}

/*!
 * \brief CCompositeRingItem::CCompositeRingItem
 * \param type      the type of the item (this should have bit 16 set)
 * \param timestamp event timestamp
 * \param sourceId  source id
 * \param children  vector of children
 */
CCompositeRingItem::CCompositeRingItem(uint32_t type,
                                     uint64_t timestamp,
                                     uint32_t sourceId,
                                     const std::vector<CRingItemPtr>& children)
    : m_type(type), m_evtTimestamp(timestamp), m_sourceId(sourceId), m_children()
{
    if ((type & 0x8000) != 0x8000) {
        throw std::invalid_argument("CCompositeRingItem() must pass type argument with composite bit set");
    }
}

/*!
 * \brief Construct from a raw ring item
 *
 * \param rawItem the raw ring item to parse
 *
 * A complete parse of the raw ring item's body will be carried out (recursively
 * parsing all children as well). This is therefore
 * somewhat of an expensive operation at the moment. In the future we might want to
 * be lazy about it and only parse when the user requests access to the children.
 * This will properly handle the situation when the raw ring item has a different
 * byte ordering than the local machine.
 *
 * \throws std::runtime_error if the buffer contains incomplete ring items
 * \throws std::runtime_error if the type of the children (lowest 15 bits) do not match the type of the parent.
 */
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


/*!
 * \brief Equality comparison operator
 * \param rhs   the ring item to compare to
 *
 * \retval true if all data are identical (including children)
 * \retval false otherwise
 *
 * \throws std::bad_cast if rhs is not a CCompositeRingItem
 */
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

/*!
 * \brief Inequality comparison operator
 *
 * \param rhs the ring item to compare to
 *
 * \retval true if not equal
 * \retval false otherwise
 */
bool CCompositeRingItem::operator!=(const CRingItem& rhs) const
{
    return !(*this == rhs);
}


/*!
 * \return the entire size of the ring item
 *
 * The size is computed everytime it is requested. It includes the size
 * of all children. In the future we might consider caching the size
 * and recomputing it as necessary. At the moment, it has a complexity that
 * is linear to the number of all children.
 */
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

/*!
 * \brief Set the type
 * \param type  the new type
 *
 * \throws std::invalid_argument if the composite bit is not set in the type
 */
void CCompositeRingItem::setType(uint32_t type) {
    if ((type & 0x8000) != 0x8000) {
        throw std::invalid_argument("CCompositeRingItem::setType() must pass argument with composite bit set");
    }
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

/*! \retval true  always*/
bool CCompositeRingItem::isComposite() const {
    return true;
}

/*! \retval false*/
bool CCompositeRingItem::mustSwap() const {
    return false;
}

/*!
 * \brief Serializes the ring item and its children into the body of a raw ring item
 * \param rawBuffer the raw ring item to fill in
 *
 *
 */
void CCompositeRingItem::toRawRingItem(CRawRingItem& rawBuffer) const {

    rawBuffer.setType(type() | 0x8000); // set the Composite bit in case it was not set
    rawBuffer.setSourceId(getSourceId());
    rawBuffer.setEventTimestamp(getEventTimestamp());

    auto& body = rawBuffer.getBody();
    body.clear();

    body.reserve(size()-20); // the minus 20 is to exclude for the header

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


/*!
 * \brief Outputs a string that contains the entire tree structure
 *
 * \return a string that represents the ring item contents
 */
std::string CCompositeRingItem::toString() const {
    std::stringstream result;
    result << "Composite Item" << std::endl;
    result << headerToString(*this);
    result << "# subitems   : " << m_children.size() << std::endl;

    int index = 0;
    for (auto pChild : m_children) {
        result << "+-- Subitem #" << index++ << std::endl;

        std::string text = insertIndent(pChild->toString(), 4);
        result << text;
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

/*!
 * \brief Insert spaces after all newlines except for the last one
 *
 * \param text      the source string
 * \param nSpaces   the number of spaces to insert
 *
 * \return a new string with proper indenting
 */
std::string CCompositeRingItem::insertIndent(const std::string& text, int nSpaces) const
{
    std::string newText = text;
    std::string spaces;
    for (int i=0; i<nSpaces; i++) {
        spaces += " ";
    }

    int index=0;

    newText.insert(0, spaces);
    index = newText.find('\n');
    while (index != std::string::npos && (index != newText.length()-1)) {
        newText.insert(index+1, spaces);
        index = newText.find('\n', index+1);
    }

    return newText;
}


} // end V12
} // end DAQ

