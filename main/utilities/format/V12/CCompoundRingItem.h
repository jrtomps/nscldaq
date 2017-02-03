/*
#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2017.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Author:
#            Jeromy Tompkins
#            NSCL
#            Michigan State University
#            East Lansing, MI 48824-1321
*/

#ifndef DAQ_V12_CCOMPOUNDRINGITEM_H
#define DAQ_V12_CCOMPOUNDRINGITEM_H

#include <V12/CRingItem.h>
#include <V12/CRawRingItem.h>

#include <vector>

namespace DAQ {
namespace V12 {


/*!
 * \brief The CCompositeRingItem class
 *
 * The CompositeRingItem class is the only class that can represent a composite
 * ring item. It is not a leaf type and will have a body that consists of 0 or more
 * ring items. It can be used as follows to build up a composite item.
 *
 * \code
 * #include <V12/CCompositeRingItem.h>
 * #include <V12/CPhysicsEventItem.h>
 *
 * using namespace DAQ::V12;
 *
 * CCompositeRingItem item(COMP_PHYSICS_EVENT, 234, 1);
 *
 * // create a physics event and add it as a child
 * CRingItemPtr pChild(new CPhysicsEventItem);
 * item.appendChild(pChild);
 *
 * // create a composite item that will also be a child and fill it with
 * // a physics event
 * CRingItemPtr pChildComposite(new CCompositeRingItem);
 * pChildComposite.appendChild(CRingItemPtr(new CPhysicsEventItem));
 *
 * // add the child composite item to the toplevel composite
 * item.appendChild(pChildComposite);
 *
 * \endcode
 *
 * The code above creates a ring item that can be represented as:
 *
 * \verbatim
 *              *  (item)
 *             / \
 *   (pChild) *   * (pChildComposite)
 *                 \
 *                  * (physics event)
 * \endverbatim
 *
 * If you already have a set of ring items that you want to be grouped into a
 * composite ring item, you can do so in the following manner:
 *
 * \code
 * #include <V12/CRingStateChangeItem.h>
 * #include <V12/CCompositeRingItem.h>
 * using namespace DAQ::V12;
 *
 * // create the children
 * CRingItemPtr begin1(new CRingStateChangeItem(BEGIN_RUN));
 * CRingItemPtr begin2(new CRingStateChangeItem(BEGIN_RUN));
 *
 * // create the composite
 * CCompositeRingItem composite(COMP_BEGIN_RUN, NULL_TIMESTAMP, 0, {begin1, begin2});
 *
 * \endcode
 *
 * If on the other hand you are trying to decode a data for analysis, you are probably
 * doing the opposite of what has already been demonstrated. You may need to handle raw
 * data. That might look like this:
 *
 * \code
 *
 *  #include <V12/CRawRingItem.h>
 *  #include <V12/CCompositeRingItem.h>
 *  #include <V12/CRingItemFactory.h>
 *
 *  using namespace DAQ::V12;
 *
 *  CRawRingItem rawRingItem;
 *
 *  std::cin >> rawRingItem;
 *
 *  if (rawRingItem.isComposite()) {
 *    auto compositeItem = format_cast<CCompositeRingItem>(rawRingItem);
 *  }
 *
 *  \\ or use the factory
 *
 *  CRingItemPtr pItem = CRingItemFactory::createRingItem(rawRingItem);
 *  if (pItem->isComposite()) {
 *    auto compositeItem = format_cast<CCompositeRingItem>(*pItem);
 *  }
 * \endcode
 *
 * or
 *
 * \code
 *
 *  #include <V12/CRingItemParser.h>
 *  #include <V12/CCompositeRingItem.h>
 *
 *  using namespace DAQ::V12;
 *
 *  char buffer[2048];
 *  std::cin.read(buffer, sizeof(buffer));
 *
 *  CRingItemPtr pItem = Parser::parse(buffer, buffer+sizeof(buffer));
 *  if (pItem->isComposite()) {
 *    auto& compItem = dynamic_cast<CCompositeRingItem&>(*pItem);
 *  }
 *
 *  \\ or you could use the factory
 *
 *  pItem = CRingItemFactory::createRingItem(buffer, buffer+sizeof(buffer));
 *  if (pItem->isComposite()) {
 *    auto& compItem = dynamic_cast<CCompositeRingItem&>(*pItem);
 *  }
 *
 * \endcode
 */
class CCompositeRingItem : public CRingItem
{

private:
    uint32_t m_type;
    uint64_t m_evtTimestamp;
    uint32_t m_sourceId;
    std::vector<CRingItemPtr> m_children;

public:
    CCompositeRingItem();
    CCompositeRingItem(uint32_t type, uint64_t timestamp, uint32_t sourceId,
                      const std::vector<CRingItemPtr>& children = std::vector<CRingItemPtr>());

    CCompositeRingItem(const CRawRingItem& rawItem);

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

private:
    std::string insertIndent(const std::string& text, int nSpaces) const;
};


}  // end V12 namespace
}  // end DAQ namespace

#endif // DAQ_V12_CCOMPOUNDRINGITEM_H
