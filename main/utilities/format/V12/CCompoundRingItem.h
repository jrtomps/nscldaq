#ifndef DAQ_V12_CCOMPOUNDRINGITEM_H
#define DAQ_V12_CCOMPOUNDRINGITEM_H

#include <V12/CRingItem.h>
#include <V12/CRawRingItem.h>

#include <vector>

namespace DAQ {
namespace V12 {

class CCompoundRingItem : public CRingItem
{

private:
    uint32_t m_type;
    uint64_t m_evtTimestamp;
    uint32_t m_sourceId;
    std::vector<CRingItemPtr> m_children;

public:
    CCompoundRingItem();
    CCompoundRingItem(uint32_t type, uint64_t timestamp, uint32_t sourceId,
                      const std::vector<CRingItemPtr>& children = std::vector<CRingItemPtr>());

    CCompoundRingItem(const CRawRingItem& rawItem);

    virtual bool operator==(const CRingItem& rhs) const;
    virtual bool operator!=(const CRingItem& rhs) const;

    uint32_t size() const;

    uint32_t type() const;
    void setType(uint32_t type);

    uint32_t getSourceId() const;
    void setSourceId(uint32_t id);

    uint64_t getEventTimestamp() const;
    void setEventTimestamp(uint64_t stamp);

    bool isComposite() const;
    bool mustSwap() const;

    void toRawRingItem(CRawRingItem& rawBuffer) const;

    std::string typeName() const;
    std::string toString() const;

    std::vector<CRingItemPtr>& getChildren();

    const std::vector<CRingItemPtr> getChildren() const;
    void appendChild(CRingItemPtr item);
};


}  // end V12 namespace
}  // end DAQ namespace

#endif // DAQ_V12_CCOMPOUNDRINGITEM_H
