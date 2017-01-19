// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include <string>

#include "Asserts.h"
#include "V12/DataFormat.h"
#include "V12/CGlomParameters.h"
#include "V12/CRawRingItem.h"
#include "Deserializer.h"

// Tests for glom parameter ring item class:

using namespace DAQ;
using namespace DAQ::V12;

class GlomItemTests : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(GlomItemTests);
    CPPUNIT_TEST(notBuilding);
    CPPUNIT_TEST(isBuilding);
    CPPUNIT_TEST(structure_0);
    CPPUNIT_TEST(structure_1);
    CPPUNIT_TEST(comparison_0);
    CPPUNIT_TEST(comparison_1);
    CPPUNIT_TEST(comparison_2);
    CPPUNIT_TEST(comparison_3);
    CPPUNIT_TEST(comparison_4);
    CPPUNIT_TEST(comparison_5);
    CPPUNIT_TEST(constructFromRaw_0);
    CPPUNIT_TEST(constructFromRaw_1);
    CPPUNIT_TEST(constructFromRaw_2);
    CPPUNIT_TEST_SUITE_END();
private:

public:
  void setUp() {
  }
  void tearDown() {
  }
protected:
    void notBuilding();
    void isBuilding();
    void structure_0();
    void structure_1();
    void comparison_0();
    void comparison_1();
    void comparison_2();
    void comparison_3();
    void comparison_4();
    void comparison_5();
    void constructFromRaw_0();
    void constructFromRaw_1();
    void constructFromRaw_2();
};

CPPUNIT_TEST_SUITE_REGISTRATION(GlomItemTests);

// Test parameters if not building:

void
GlomItemTests::notBuilding()
{
    CGlomParameters item(0x123456, 23, static_cast<uint64_t>(1234), false, CGlomParameters::last);

    EQMSG("evt tstamp", uint64_t(0x123456), item.getEventTimestamp());
    EQMSG("source id", uint32_t(23), item.getSourceId());
    EQMSG("coinc ticks", static_cast<uint64_t>(1234), item.coincidenceTicks());
    EQMSG("building", false, item.isBuilding());
    EQMSG("policy", CGlomParameters::last, item.timestampPolicy());
}
//
// Test parametrs if building:

void
GlomItemTests::isBuilding()
{
    CGlomParameters item(static_cast<uint64_t>(1234), true, CGlomParameters::first);
    EQ(true, item.isBuilding());
}

// Make sure construction produces structurally correct stuff:

void
GlomItemTests::structure_0()
{
    CGlomParameters item(1234, true, CGlomParameters::average);
    CRawRingItem raw(VOID);

    item.toRawRingItem(raw);
    
        
    EQMSG("size", item.size(), raw.size());
    EQMSG("type", EVB_GLOM_INFO, raw.type());
        
    EQMSG("evt timestamp", NULL_TIMESTAMP, raw.getEventTimestamp());
    EQMSG("source id", uint32_t(0), raw.getSourceId());

    // Body contents:
    Buffer::Deserializer<Buffer::ByteBuffer> stream(raw.getBody(), raw.mustSwap());

    uint64_t ticks;
    uint16_t isBuilding, policy;
    stream >> ticks >> isBuilding >> policy;

    EQMSG("ticks", static_cast<uint64_t>(1234), ticks);
    EQMSG("is building", uint16_t(1), isBuilding);
    EQMSG("policy", GLOM_TIMESTAMP_AVERAGE, policy);
}


// test that if the body of the raw ring item is not empty, it does not
// taint the remainder result of the toRawRingItem() method.
void
GlomItemTests::structure_1()
{
    CGlomParameters item(1234, true, CGlomParameters::average);
    CRawRingItem raw(VOID);

    raw.getBody() << 234; // load the body with some info


    item.toRawRingItem(raw);

    // Body contents:
    Buffer::Deserializer<Buffer::ByteBuffer> stream(raw.getBody(), raw.mustSwap());

    uint64_t ticks;
    stream >> ticks;

    EQMSG("ticks", static_cast<uint64_t>(1234), ticks);

}


void GlomItemTests::comparison_0()
{
    CGlomParameters item(1234, 3, 234, true, CGlomParameters::first);
    CPPUNIT_ASSERT_MESSAGE("identity comparison", item == item);
}


void GlomItemTests::comparison_1()
{
    CGlomParameters item1(1234, 3, 234, true, CGlomParameters::first);
    CGlomParameters item2(2234, 3, 234, true, CGlomParameters::first);
    CPPUNIT_ASSERT_MESSAGE("different evt timestamp", item1 != item2);
}

void GlomItemTests::comparison_2()
{
    CGlomParameters item1(1234, 3, 234, true, CGlomParameters::first);
    CGlomParameters item2(1234, 4, 234, true, CGlomParameters::first);
    CPPUNIT_ASSERT_MESSAGE("different source id", item1 != item2);
}

void GlomItemTests::comparison_3()
{
    CGlomParameters item1(1234, 3, 234, true, CGlomParameters::first);
    CGlomParameters item2(1234, 3, 334, true, CGlomParameters::first);
    CPPUNIT_ASSERT_MESSAGE("different ticks", item1 != item2);
}

void GlomItemTests::comparison_4()
{
    CGlomParameters item1(1234, 3, 234, true, CGlomParameters::first);
    CGlomParameters item2(1234, 3, 234, false, CGlomParameters::first);
    CPPUNIT_ASSERT_MESSAGE("different build flag", item1 != item2);
}


void GlomItemTests::comparison_5()
{
    CGlomParameters item1(1234, 3, 234, true, CGlomParameters::first);
    CGlomParameters item2(1234, 3, 234, true, CGlomParameters::average);
    CPPUNIT_ASSERT_MESSAGE("different tstamp policy", item1 != item2);
}



void GlomItemTests::constructFromRaw_0()
{
    CRawRingItem raw(EVB_GLOM_INFO);
    raw.setSourceId(3);
    raw.setEventTimestamp(2335252);
    auto& body = raw.getBody();
    body << uint64_t(3456);
    body << uint16_t(1);
    body << uint16_t(2);

    CGlomParameters params(raw);

    EQMSG("type", EVB_GLOM_INFO, params.type());
    EQMSG("size", uint32_t(32), params.size());
    EQMSG("source id", uint32_t(3), params.getSourceId());
    EQMSG("ticks", uint64_t(3456), params.coincidenceTicks());
    EQMSG("policy",CGlomParameters::average, params.timestampPolicy());
    EQMSG("building", true, params.isBuilding());
}


void GlomItemTests::constructFromRaw_1()
{
    CRawRingItem raw(VOID);

    CPPUNIT_ASSERT_THROW_MESSAGE("glom parameters must construct from EVB_GLOM_INFO type",
                                 CGlomParameters params(raw),
                                 std::bad_cast);
}


void GlomItemTests::constructFromRaw_2()
{
    CRawRingItem raw(EVB_GLOM_INFO);
    raw.setSourceId(3);
    raw.setEventTimestamp(2335252);
    auto& body = raw.getBody();
    body << uint64_t(3456);
    body << uint16_t(1);
    body << uint16_t(10);

    CPPUNIT_ASSERT_THROW_MESSAGE("invalid policy during construction causes failure",
                                 CGlomParameters params(raw),
                                 std::invalid_argument);

}

