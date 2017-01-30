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
#include "Deserializer.h"

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
    CPPUNIT_TEST(typeName_0);
    CPPUNIT_TEST_SUITE_END();
private:

public:
  void setUp() {
  }
  void tearDown() {
  }
protected:

  void size_0() {
      CCompoundRingItem<CPhysicsEventItem> item;
      EQMSG("empty size", uint32_t(20), item.size());
  }

  void size_1() {
      CCompoundRingItem<CPhysicsEventItem> item;
      item.appendChild(CPhysicsEventItem(23, 34, {2,3,4,5}));

      EQMSG("size", uint32_t(44), item.size());
  }


  void appendChild_0() {
      CCompoundRingItem<CRingStateChangeItem> item;
      item.appendChild(CRingStateChangeItem(BEGIN_RUN));

      EQMSG("n children", size_t(1), item.getChildren().size());
  }

  void toRawRingItem_0() {
      CCompoundRingItem<CRingStateChangeItem> item(BEGIN_RUN, 12,23);
      CRawRingItem raw;

      item.toRawRingItem(raw);

      EQMSG("size", item.size(), raw.size());
      EQMSG("type", item.type() | 0x8000, raw.type());
      EQMSG("evt timestamp", item.getEventTimestamp(), raw.getEventTimestamp());
      EQMSG("source id", item.getSourceId(), raw.getSourceId());
      EQMSG("composite", true, raw.isComposite());
  }

  void toRawRingItem_1() {
      CCompoundRingItem<CRingStateChangeItem> item(BEGIN_RUN, 12,34);
      CRingStateChangeItem child(BEGIN_RUN, 2, 3, 4, "testing");
      item.appendChild(child);
      CRawRingItem raw;

      item.toRawRingItem(raw);

      EQMSG("size", uint32_t(20) + child.size(), raw.size());
      EQMSG("type", item.type() | 0x8000, raw.type());
      EQMSG("evt timestamp", item.getEventTimestamp(), raw.getEventTimestamp());
      EQMSG("source id", item.getSourceId(), raw.getSourceId());
      EQMSG("composite", true, raw.isComposite());
  }

  void toString_0() {
      CPPUNIT_FAIL("not implemented yet");
  }

  void typeName_0() {
      CPPUNIT_FAIL("not implemented yet");
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(CCompoundRingItemTests);

// Test parameters if not building:

