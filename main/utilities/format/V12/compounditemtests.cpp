// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include <string>

#include "Asserts.h"
#include "V12/DataFormat.h"
#include "V12/CCompoundRingItem.h"
#include "V12/CRawRingItem.h"
#include "V12/CRingStateChangeItem.h"
#include "V12/CPhysicsEventItem.h"
#include "V12/CRingPhysicsEventCountItem.h"
#include "Deserializer.h"

#include <iostream>
#include <sstream>

// Tests for glom parameter ring item class:

using namespace DAQ;
using namespace DAQ::V12;

class CCompoundRingItemTests : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CCompoundRingItemTests);
    CPPUNIT_TEST(size_0);
    CPPUNIT_TEST(size_1);
    CPPUNIT_TEST(appendChild_0);
    CPPUNIT_TEST(toRawRingItem_0);
    CPPUNIT_TEST(toRawRingItem_1);
    CPPUNIT_TEST(toString_0);
    CPPUNIT_TEST(toString_1);
    CPPUNIT_TEST(typeName_0);
    CPPUNIT_TEST(comparison_0);
    CPPUNIT_TEST(comparison_1);
    CPPUNIT_TEST(comparison_2);
    CPPUNIT_TEST(comparison_3);
    CPPUNIT_TEST(comparison_4);
    CPPUNIT_TEST(comparison_5);
    CPPUNIT_TEST(fromRawRingItem_0);
    CPPUNIT_TEST(fromRawRingItem_1);
    CPPUNIT_TEST_SUITE_END();
private:

public:
  void setUp() {
  }
  void tearDown() {
  }
protected:

  void size_0() {
      CCompoundRingItem item;
      EQMSG("empty size", uint32_t(20), item.size());
  }

  void size_1() {
      CCompoundRingItem item;
      item.appendChild(CRingItemPtr(new CPhysicsEventItem(23, 34, {2,3,4,5})));

      EQMSG("size", uint32_t(44), item.size());
  }


  void appendChild_0() {
      CCompoundRingItem item;
      item.appendChild(CRingItemPtr(new CRingStateChangeItem(BEGIN_RUN)));

      EQMSG("n children", size_t(1), item.getChildren().size());
  }

  void toRawRingItem_0() {
      CCompoundRingItem item(BEGIN_RUN, 12,23);
      CRawRingItem raw;

      item.toRawRingItem(raw);

      EQMSG("size", item.size(), raw.size());
      EQMSG("type", item.type() | 0x8000, raw.type());
      EQMSG("evt timestamp", item.getEventTimestamp(), raw.getEventTimestamp());
      EQMSG("source id", item.getSourceId(), raw.getSourceId());
      EQMSG("composite", true, raw.isComposite());
  }

  void toRawRingItem_1() {
      CCompoundRingItem item(BEGIN_RUN, 12,34);
      CRingItemPtr pChild(new CRingStateChangeItem(BEGIN_RUN, 2, 3, 4, "testing"));
      item.appendChild(pChild);
      CRawRingItem raw;

      item.toRawRingItem(raw);

      EQMSG("size", uint32_t(20) + pChild->size(), raw.size());
      EQMSG("type", item.type() | 0x8000, raw.type());
      EQMSG("evt timestamp", item.getEventTimestamp(), raw.getEventTimestamp());
      EQMSG("source id", item.getSourceId(), raw.getSourceId());
      EQMSG("composite", true, raw.isComposite());
  }

  void toString_0() {
      CCompoundRingItem item(BEGIN_RUN, 12, 23);
      std::stringstream asString;
      asString << "Composite Item" << std::endl;
      asString << "Size (bytes): 20" << std::endl;
      asString << "Type:         Begin Run" << std::endl;
      asString << "Timestamp:    12" << std::endl;
      asString << "Source Id:    23" << std::endl;
      asString << "Number of subitems: 0" << std::endl;

      EQMSG("empty compound item", asString.str(), item.toString());

  }

  void toString_1() {
      CCompoundRingItem item(BEGIN_RUN, 12, 23);
      CRingItemPtr pChild(new CRingStateChangeItem(34, 12, BEGIN_RUN, 3452,
                                                     0, 1485797295, "Testing"));
      item.appendChild(pChild);
      std::stringstream asString;
      asString << "Composite Item" << std::endl;
      asString << "Size (bytes): 67" << std::endl;
      asString << "Type:         Begin Run" << std::endl;
      asString << "Timestamp:    12" << std::endl;
      asString << "Source Id:    23" << std::endl;
      asString << "Number of subitems: 1" << std::endl;
      asString << "---- Subitem #0" << std::endl;
      asString << "Mon Jan 30 12:28:15 2017 : Run State Change : Begin Run at 0 seconds into the run" << std::endl;
      asString << "Size (bytes): 47" << std::endl;
      asString << "Type:         Begin Run" << std::endl;
      asString << "Timestamp:    34" << std::endl;
      asString << "Source Id:    12" << std::endl;
      asString << "Title     : Testing" << std::endl;
      asString << "Run Number: 3452" << std::endl;

      EQMSG("empty compound item", asString.str(), item.toString());

  }

  void typeName_0() {

      CCompoundRingItem item(PHYSICS_EVENT, 12, 34);

      EQMSG("typename", std::string("Physics Event"), item.typeName());
  }


  void comparison_0() {
    CCompoundRingItem item(PHYSICS_EVENT, 12, 34);
    item.appendChild(CRingItemPtr(new CPhysicsEventItem(22, 33)));

    CPPUNIT_ASSERT_MESSAGE("identity comparison",  item == item);
  }

  void comparison_1() {
    CCompoundRingItem item1(PHYSICS_EVENT, 12, 34);
    item1.appendChild(CRingItemPtr(new CPhysicsEventItem(22, 33)));

    CCompoundRingItem item2(PHYSICS_EVENT_COUNT, 12, 34);
    item2.appendChild(CRingItemPtr(new CRingPhysicsEventCountItem(22, 33)));

    CPPUNIT_ASSERT_MESSAGE("different types",  item1 != item2);
  }

  void comparison_2() {
    CCompoundRingItem item1(PHYSICS_EVENT, 12, 34);
    item1.appendChild(CRingItemPtr(new CPhysicsEventItem(22, 33)));

    CCompoundRingItem item2(PHYSICS_EVENT, 22, 34);
    item2.appendChild(CRingItemPtr(new CPhysicsEventItem(22, 33)));

    CPPUNIT_ASSERT_MESSAGE("different evt timestamp",  item1 != item2);
  }

  void comparison_3() {
    CCompoundRingItem item1(PHYSICS_EVENT, 12, 34);
    item1.appendChild(CRingItemPtr(new CPhysicsEventItem(22, 33)));

    CCompoundRingItem item2(PHYSICS_EVENT, 12, 44);
    item2.appendChild(CRingItemPtr(new CPhysicsEventItem(22, 33)));

    CPPUNIT_ASSERT_MESSAGE("different source id",  item1 != item2);
  }

  void comparison_4() {
    CCompoundRingItem item1(PHYSICS_EVENT, 12, 34);
    item1.appendChild(CRingItemPtr(new CPhysicsEventItem(22, 33)));

    CCompoundRingItem item2(PHYSICS_EVENT, 12, 34);

    CPPUNIT_ASSERT_MESSAGE("different number of children",  item1 != item2);
  }

  void comparison_5() {
    CCompoundRingItem item1(PHYSICS_EVENT, 12, 34);
    item1.appendChild(CRingItemPtr(new CPhysicsEventItem(22, 33)));

    CCompoundRingItem item2(PHYSICS_EVENT, 12, 34);
    item2.appendChild(CRingItemPtr(new CPhysicsEventItem(23, 33)));

    CPPUNIT_ASSERT_MESSAGE("different children",  item1 != item2);
  }


  void fromRawRingItem_0() {
    CRawRingItem rawItem(COMP_PHYSICS_EVENT, 12, 23);
    auto& body = rawItem.getBody();
    body << uint32_t(21) << PHYSICS_EVENT << uint64_t(12) << uint32_t(23);
    body << uint8_t(1);

    CCompoundRingItem item(rawItem);
    EQMSG("size", uint32_t(41), item.size());
    EQMSG("type", COMP_PHYSICS_EVENT, item.type());
    EQMSG("tstamp", uint64_t(12), item.getEventTimestamp());
    EQMSG("source id", uint32_t(23), item.getSourceId());
    EQMSG("n children", size_t(1), item.getChildren().size());
    EQMSG("child size", uint32_t(21), item.getChildren()[0]->size());
    EQMSG("child type", PHYSICS_EVENT, item.getChildren()[0]->type());
    EQMSG("child evt tstamp", uint64_t(12), item.getChildren()[0]->getEventTimestamp());
    EQMSG("child source id", uint32_t(23), item.getChildren()[0]->getSourceId());

    auto& physEvent = dynamic_cast<CPhysicsEventItem&>(*item.getChildren()[0]);
    CPPUNIT_ASSERT_MESSAGE("child body", Buffer::ByteBuffer({1}) == physEvent.getBody());
  }



  void fromRawRingItem_1() {
      CRawRingItem rawItem(COMP_PHYSICS_EVENT, 12, 23);
      auto& body = rawItem.getBody();
      body << uint32_t(41) << COMP_PHYSICS_EVENT << uint64_t(12) << uint32_t(23);
      body << uint32_t(21) << PHYSICS_EVENT << uint64_t(12) << uint32_t(23);
      body << uint8_t(1);
      body << uint32_t(22) << PHYSICS_EVENT << uint64_t(21) << uint32_t(32);
      body << uint8_t(1) << uint8_t(2);

      CCompoundRingItem item(rawItem);
      EQMSG("size", uint32_t(83), item.size());
      EQMSG("type", COMP_PHYSICS_EVENT, item.type());
      EQMSG("tstamp", uint64_t(12), item.getEventTimestamp());
      EQMSG("source id", uint32_t(23), item.getSourceId());
      EQMSG("n children", size_t(2), item.getChildren().size());

      CPPUNIT_ASSERT_NO_THROW(dynamic_cast<CCompoundRingItem&>(*item.getChildren()[0]));
      CPPUNIT_ASSERT_NO_THROW(dynamic_cast<CPhysicsEventItem&>(*item.getChildren()[1]));


      auto& compound = dynamic_cast<CCompoundRingItem&>(*item.getChildren()[0]);
      EQMSG("nchildren of subitem", size_t(1), compound.getChildren().size());
      CPPUNIT_ASSERT_NO_THROW(dynamic_cast<CPhysicsEventItem&>(*compound.getChildren()[0]));

      auto pItem = compound.getChildren()[0];
      EQMSG("size of child", uint32_t(21), pItem->size());
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(CCompoundRingItemTests);

// Test parameters if not building:

