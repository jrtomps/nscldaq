#ifndef DAQ_V12_CCOMPOUNDRINGITEM_H
#define DAQ_V12_CCOMPOUNDRINGITEM_H

#include <V12/CRingItem.h>
#include <V12/CRawRingItem.h>

#include <vector>

namespace DAQ {
namespace V12 {

template<class RingItemType>
class CCompoundRingItem : public CRingItem
{
public:
    typedef RingItemType child_type;

private:
    uint32_t m_type;
    uint64_t m_evtTimestamp;
    uint32_t m_sourceId;
    std::vector<RingItemType> m_children;

public:
    CCompoundRingItem() : CCompoundRingItem(0, NULL_TIMESTAMP, 0, {}) {}
    CCompoundRingItem(uint32_t type, uint64_t timestamp, uint32_t sourceId,
                      const std::vector<RingItemType>& children = std::vector<RingItemType>())
        : m_type(type), m_evtTimestamp(timestamp), m_sourceId(sourceId), m_children()
    {}


    CCompoundRingItem(const CRawRingItem& rawItem) {}


    uint32_t size() const {
        uint32_t size = 20; // we have at least a header
        for (auto& child : m_children) {
            size += child.size();
        }
        return size;
    }

    uint32_t type() const {
        return m_type;
    }
    void setType(uint32_t type) {}

    uint32_t getSourceId() const {
        return m_sourceId;
    }
    void setSourceId(uint32_t id)
    {
        m_sourceId = id;
    }

    uint64_t getEventTimestamp() const {
        return m_evtTimestamp;
    }
    void setEventTimestamp(uint64_t stamp) {
        m_evtTimestamp = stamp;
    }

    bool isComposite() const {
        return true;
    }
    bool mustSwap() const {
        return false;
    }

    void toRawRingItem(CRawRingItem& rawBuffer) const {

        rawBuffer.setType(type() | 0x8000); // set the compound bit in case it was not set
        rawBuffer.setSourceId(getSourceId());
        rawBuffer.setEventTimestamp(getEventTimestamp());

        auto& body = rawBuffer.getBody();
        body.clear();

        body.reserve(size()); // reserves 20 bytes more than necessary, but that is not a big deal.

        for (auto& child : m_children) {
            CRawRingItem chunk(child);
            body << chunk.size();
            body << chunk.type();
            body << chunk.getEventTimestamp();
            body << chunk.getSourceId();
            body.insert(body.end(), chunk.getBody().begin(), chunk.getBody().end());
        }
    }

    std::string typeName() const {
        return std::string();
    }

    std::string toString() const {
        return std::string();
    }

    std::vector<RingItemType>& getChildren() {
        return m_children;
    }

    const std::vector<RingItemType> getChildren() const
    {
        return m_children;
    }

    void appendChild(const RingItemType& item)
    {
        m_children.push_back(item);
    }
};


}  // end V12 namespace
}  // end DAQ namespace

#endif // DAQ_V12_CCOMPOUNDRINGITEM_H
