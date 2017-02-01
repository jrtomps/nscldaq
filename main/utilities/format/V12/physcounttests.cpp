// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"


#include <V12/CRingPhysicsEventCountItem.h>
#include <V12/CRawRingItem.h>
#include <Deserializer.h>

#include <string.h>
#include <time.h>
#include <chrono>

using namespace DAQ;

class physcounttests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(physcounttests);
  CPPUNIT_TEST(simplecons);
  CPPUNIT_TEST(partialcons);
  CPPUNIT_TEST(fullcons);
  CPPUNIT_TEST(castcons_0);
  CPPUNIT_TEST(castcons_1);
  CPPUNIT_TEST(castcons_2);
  CPPUNIT_TEST(accessors);
  CPPUNIT_TEST(comparison_0);
  CPPUNIT_TEST(comparison_1);
  CPPUNIT_TEST(comparison_2);
  CPPUNIT_TEST(comparison_3);
  CPPUNIT_TEST(comparison_4);
  CPPUNIT_TEST(comparison_5);
  CPPUNIT_TEST(comparison_6);
  CPPUNIT_TEST(copycons);
  CPPUNIT_TEST(assign);
  CPPUNIT_TEST(tscons);
  CPPUNIT_TEST(fractionalTime);
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
  void simplecons();
  void partialcons();
  void fullcons();
  void castcons_0();
  void castcons_1();
  void castcons_2();
  void accessors();
  void comparison_0();
  void comparison_1();
  void comparison_2();
  void comparison_3();
  void comparison_4();
  void comparison_5();
  void comparison_6();
  void copycons();
  void assign();
  void tscons();
  void fractionalTime();
  void toRawRingItem_0();
  void toRawRingItem_1();
};

CPPUNIT_TEST_SUITE_REGISTRATION(physcounttests);

// Test construction with default constructor.

void physcounttests::simplecons() {
  V12::CRingPhysicsEventCountItem instance;

  EQMSG("type", V12::PHYSICS_EVENT_COUNT, instance.type());
  EQMSG("evt timestamp", V12::NULL_TIMESTAMP, instance.getEventTimestamp());
  EQMSG("source id", uint32_t(0), instance.getSourceId());
  EQMSG("offset",(uint32_t)0, instance.getTimeOffset());
  EQMSG("divisor", uint32_t(1), instance.getTimeDivisor());
  EQMSG("count", (uint64_t)0, instance.getEventCount());
  
}
//// Test construction given count and offset.

void physcounttests::partialcons()
{
  V12::CRingPhysicsEventCountItem i(12345678, 100);
  EQMSG("count", (uint64_t)12345678, i.getEventCount());
  EQMSG("offset", (uint32_t)100, i.getTimeOffset());
  EQMSG("type", V12::PHYSICS_EVENT_COUNT, i.type());
  EQMSG("evt timestamp", V12::NULL_TIMESTAMP, i.getEventTimestamp());
  EQMSG("source id", uint32_t(0), i.getSourceId());
  EQMSG("divisor", uint32_t(1), i.getTimeDivisor());

}

// Test full construction.
//
void physcounttests::fullcons()
{
  time_t t;
  time(&t);

  V12::CRingPhysicsEventCountItem i(12345678, 100, t);

  EQMSG("timestamp", t, i.getTimestamp());
  EQMSG("count", (uint64_t)12345678, i.getEventCount());
  EQMSG("offset", (uint32_t)100, i.getTimeOffset());
  EQMSG("type", V12::PHYSICS_EVENT_COUNT, i.type());
  EQMSG("evt timestamp", V12::NULL_TIMESTAMP, i.getEventTimestamp());
  EQMSG("source id", uint32_t(0), i.getSourceId());
  EQMSG("divisor", uint32_t(1), i.getTimeDivisor());

}


// Construct from ring item.. good and bad.
//
void physcounttests::castcons_0()
{

    V12::CRawRingItem item;
    item.setType(V12::PHYSICS_EVENT_COUNT);
    item.setSourceId(12);
    item.setEventTimestamp(134);

    auto& body = item.getBody();
    body << uint32_t(34) << uint32_t(45) << std::uint32_t(56) << std::uint64_t(1234567);

    CPPUNIT_ASSERT_NO_THROW( V12::CRingPhysicsEventCountItem(item));
}

// Construct from ring item.. good and bad.
//
void physcounttests::castcons_1()
{

    V12::CRawRingItem item;
    item.setType(V12::PHYSICS_EVENT_COUNT);
    item.setSourceId(12);
    item.setEventTimestamp(134);

    auto& body = item.getBody();
    body << uint32_t(34) << uint32_t(45) << std::uint32_t(56) << std::uint64_t(1234567);

    V12::CRingPhysicsEventCountItem i(item);
    EQMSG("type", V12::PHYSICS_EVENT_COUNT, i.type());
    EQMSG("evt timestamp", uint64_t(134), i.getEventTimestamp());
    EQMSG("source id", uint32_t(12), i.getSourceId());
    EQMSG("offset", (uint32_t)34, i.getTimeOffset());
    EQMSG("timestamp", time_t(45), i.getTimestamp());
    EQMSG("divisor", uint32_t(56), i.getTimeDivisor());
    EQMSG("count", (uint64_t)1234567, i.getEventCount());

}


void physcounttests::castcons_2()
{

    V12::CRawRingItem item;
    item.setSourceId(12);
    item.setEventTimestamp(134);

    auto& body = item.getBody();
    body << uint32_t(34) << uint32_t(45) << std::uint32_t(56) << std::uint64_t(1234567);


    // Bad construction should throw a bad cast:

    CPPUNIT_ASSERT_THROW(V12::CRingPhysicsEventCountItem i(item), std::bad_cast);
}

// Test write accessors (reads have already been tested.
//

void physcounttests::accessors()
{
  V12::CRingPhysicsEventCountItem i(0,0,0);

  i.setEventTimestamp(12345);
  EQMSG("evt timestamp", uint64_t(12345), i.getEventTimestamp());

  i.setSourceId(135);
  EQMSG("source id", uint32_t(135), i.getSourceId());

  i.setTimeOffset(1234);
  EQMSG("offset", (uint32_t)1234, i.getTimeOffset());

  time_t now = time(NULL);
  i.setTimestamp(now);
  EQMSG("timestamp", now, i.getTimestamp());

  i.setEventCount(12345678);
  EQMSG("count", (uint64_t)12345678, i.getEventCount());

  CPPUNIT_ASSERT_THROW_MESSAGE("cannot change type of physics event count item",
                               i.setType(V12::VOID),
                               std::invalid_argument);

  CPPUNIT_ASSERT_NO_THROW(i.setType(V12::PHYSICS_EVENT_COUNT));

}
// Test copy construction

void physcounttests::copycons()
{
  V12::CRingPhysicsEventCountItem original(1234, 23, 1234, 10, 5678, 4);
  V12::CRingPhysicsEventCountItem copy(original);

  ASSERT(original == copy);
}

void physcounttests::assign()
{
  V12::CRingPhysicsEventCountItem original(1234, 23, 1234, 10, 5678, 4);
  V12::CRingPhysicsEventCountItem copy;

  copy = original;
  ASSERT(original == copy);
}


// test construction with timestamps>

void
physcounttests::tscons()
{
  V12::CRingPhysicsEventCountItem item( 0x5567788ll, 1, 54321, 100, 2342, 1 );
  // Check the header

  EQMSG("type", V12::PHYSICS_EVENT_COUNT, item.type());
  EQMSG("size", uint32_t(40), item.size());

  EQMSG("evt timestamp", uint64_t(0x5567788ll), item.getEventTimestamp());
  EQMSG("source id", uint32_t(1), item.getSourceId());

  // Make sure the body contents are correct.

  EQMSG("offset", static_cast<uint32_t>(100), item.getTimeOffset());
  EQMSG("divisor", static_cast<uint32_t>(1),     item.getTimeDivisor());
  EQMSG("timestamp", static_cast<time_t>(2342), item.getTimestamp());
  EQMSG("count", static_cast<uint64_t>(54321), item.getEventCount());
}

void physcounttests::comparison_0()
{
    V12::CRingPhysicsEventCountItem item(0x5567788ll, 1, 54321, 100, 2342, 1 );

    CPPUNIT_ASSERT_MESSAGE("identity item", item == item);
}

void physcounttests::comparison_1()
{
    V12::CRingPhysicsEventCountItem item1(0x5567788ll, 1, 54321, 100, 2342, 1 );
    V12::CRingPhysicsEventCountItem item2(0x1567788ll, 1, 54321, 100, 2342, 1 );

    CPPUNIT_ASSERT_MESSAGE("different evt timestamp", item1 != item2);
}

void physcounttests::comparison_2()
{
    V12::CRingPhysicsEventCountItem item1(0x5567788ll, 1, 54321, 100, 2342, 1 );
    V12::CRingPhysicsEventCountItem item2(0x5567788ll, 2, 54321, 100, 2342, 1 );

    CPPUNIT_ASSERT_MESSAGE("different source id", item1 != item2);
}

void physcounttests::comparison_3()
{
    V12::CRingPhysicsEventCountItem item1(0x5567788ll, 1, 54321, 100, 2342, 1 );
    V12::CRingPhysicsEventCountItem item2(0x5567788ll, 1, 64321, 100, 2342, 1 );

    CPPUNIT_ASSERT_MESSAGE("different event count", item1 != item2);
}


void physcounttests::comparison_4()
{
    V12::CRingPhysicsEventCountItem item1(0x5567788ll, 1, 54321, 100, 2342, 1 );
    V12::CRingPhysicsEventCountItem item2(0x5567788ll, 1, 54321, 101, 2342, 1 );

    CPPUNIT_ASSERT_MESSAGE("different offset", item1 != item2);
}


void physcounttests::comparison_5()
{
    V12::CRingPhysicsEventCountItem item1(0x5567788ll, 1, 54321, 100, 2342, 1 );
    V12::CRingPhysicsEventCountItem item2(0x5567788ll, 1, 54321, 100,    0, 1 );

    CPPUNIT_ASSERT_MESSAGE("different timestamp", item1 != item2);
}

void physcounttests::comparison_6()
{
    V12::CRingPhysicsEventCountItem item1(0x5567788ll, 1, 54321, 100, 2342, 1 );
    V12::CRingPhysicsEventCountItem item2(0x5567788ll, 1, 54321, 100, 2342, 2 );

    CPPUNIT_ASSERT_MESSAGE("different divisor", item1 != item2);
}


// fractionalTime
//   Test of computeElapsedTime method.

void
physcounttests::fractionalTime()
{
  time_t stamp = time(NULL);
  V12::CRingPhysicsEventCountItem item(
      static_cast<uint64_t>(0x112233445567788ll), 1,
      static_cast<uint64_t>(54321), 100, stamp, 3
  );
  
  EQ(static_cast<float>(100.0/3.0), item.computeElapsedTime());
}



void physcounttests::toRawRingItem_0()
{
    V12::CRawRingItem raw;
    V12::CRingPhysicsEventCountItem item(0x11223344ll, 1, 54321, 100, 33, 3);

    item.toRawRingItem(raw);

    EQMSG("size", item.size(), raw.size());
    EQMSG("type", V12::PHYSICS_EVENT_COUNT, raw.type());
    EQMSG("source id", uint32_t(1), raw.getSourceId());
    EQMSG("evt timestamp", uint64_t(0x11223344), raw.getEventTimestamp());

    uint32_t offset, divisor, tstamp;
    uint64_t count;

    Buffer::ContainerDeserializer<Buffer::ByteBuffer> stream(raw.getBody(), raw.mustSwap());
    stream >> offset >> tstamp >> divisor >> count;

    EQMSG("time offset", uint32_t(100), offset);
    EQMSG("timestamp", uint32_t(33), tstamp);
    EQMSG("divisor", uint32_t(3), divisor);
    EQMSG("count", uint64_t(54321), count);
}

void physcounttests::toRawRingItem_1()
{
    V12::CRawRingItem raw;

    // insert some prior data into the body of the raw ring item...
    // make sure that if this happens, it does not break anything.
    raw.getBody().push_back(9);

    V12::CRingPhysicsEventCountItem item(0x11223344ll, 1, 54321, 100, 33, 3);
    item.toRawRingItem(raw);

    uint32_t offset;
    Buffer::ContainerDeserializer<Buffer::ByteBuffer> stream(raw.getBody(), raw.mustSwap());
    stream >> offset;
    EQMSG("time offset", uint32_t(100), offset);
}
