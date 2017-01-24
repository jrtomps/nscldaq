// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include <string>

#include "Asserts.h"
#include "V12/DataFormat.h"
#include "V12/CAbnormalEndItem.h"
#include "V12/CRawRingItem.h"
#include "Deserializer.h"

// Tests for glom parameter ring item class:

using namespace DAQ;
using namespace DAQ::V12;

class CAbnormalEndRunTests : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CAbnormalEndRunTests);
    CPPUNIT_TEST(comparison_0);
    CPPUNIT_TEST(comparison_1);
    CPPUNIT_TEST(comparison_2);
    CPPUNIT_TEST(constructFromRaw_0);
    CPPUNIT_TEST(toRawRingItem_0);
    CPPUNIT_TEST(toRawRingItem_1);
    CPPUNIT_TEST_SUITE_END();
private:

public:
  void setUp() {
  }
  void tearDown() {
  }
protected:
    void comparison_0();
    void comparison_1();
    void comparison_2();
    void constructFromRaw_0();
    void constructFromRaw_1();
    void toRawRingItem_0();
    void toRawRingItem_1();
};

CPPUNIT_TEST_SUITE_REGISTRATION(CAbnormalEndRunTests);

// Test parameters if not building:


void CAbnormalEndRunTests::comparison_0()
{
    CAbnormalEndItem item;
    CPPUNIT_ASSERT_MESSAGE("identity comparison", item == item);
}


void CAbnormalEndRunTests::comparison_1()
{
    CAbnormalEndItem item1;
    CAbnormalEndItem item2;
    item2.setSourceId(2323);
    CPPUNIT_ASSERT_MESSAGE("different source id", item1 != item2);
}


void CAbnormalEndRunTests::comparison_2()
{
    CAbnormalEndItem item1;
    CAbnormalEndItem item2;
    item2.setEventTimestamp(1223);
    CPPUNIT_ASSERT_MESSAGE("different evt timestamp", item1 != item2);
}


void CAbnormalEndRunTests::constructFromRaw_0()
{
    CRawRingItem raw(ABNORMAL_ENDRUN, NULL_TIMESTAMP, 0);

    CAbnormalEndItem item(raw);

    EQMSG("type", ABNORMAL_ENDRUN, item.type());
    EQMSG("size", uint32_t(20), item.size());
    EQMSG("source id", uint32_t(0), item.getSourceId());
    EQMSG("evt tstamp", NULL_TIMESTAMP, item.getEventTimestamp());
}


void CAbnormalEndRunTests::constructFromRaw_1()
{
    CRawRingItem raw;

    CPPUNIT_ASSERT_THROW_MESSAGE("abnormal end run must construct from EVB_GLOM_INFO type",
                                 CAbnormalEndItem item(raw),
                                 std::bad_cast);
}

void CAbnormalEndRunTests::toRawRingItem_0()
{
    CRawRingItem raw;
    CAbnormalEndItem item;
    item.setEventTimestamp(234);
    item.setSourceId(2323);
    item.toRawRingItem(raw);

    EQMSG("size", item.size(), raw.size());
    EQMSG("type", ABNORMAL_ENDRUN, raw.type());
    EQMSG("evt timestamp", item.getEventTimestamp(), raw.getEventTimestamp());
    EQMSG("source id", item.getSourceId(), raw.getSourceId());
    EQMSG("empty body", size_t(0), raw.getBody().size());
}

void CAbnormalEndRunTests::toRawRingItem_1()
{
    // we are going to preload the raw ring item with data... this _must_ get wiped out
    // during the toRawRingItem method.
    CRawRingItem raw;
    raw.getBody().push_back(1);

    CAbnormalEndItem item;
    item.toRawRingItem(raw);

    EQMSG("empty body", size_t(0), raw.getBody().size());
}
