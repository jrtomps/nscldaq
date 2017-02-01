// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"

#include <V12/DataFormat.h>
#include <V12/CRingStateChangeItem.h>
#include <V12/CRawRingItem.h>
#include <Deserializer.h>

#include <string>
#include <tuple>
#include <time.h>
#include <cstring>


using namespace std;

using namespace DAQ;

class teststate : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(teststate);
  CPPUNIT_TEST(basiccons);
  CPPUNIT_TEST(fullcons_0);
  CPPUNIT_TEST(fullcons_1);
  CPPUNIT_TEST(torawringitem_0);
  CPPUNIT_TEST(comparison_0);
  CPPUNIT_TEST(comparison_1);
  CPPUNIT_TEST(comparison_2);
  CPPUNIT_TEST(comparison_3);
  CPPUNIT_TEST(comparison_4);
  CPPUNIT_TEST(comparison_5);
  CPPUNIT_TEST(accessors_0);
  CPPUNIT_TEST(accessors_1);
  CPPUNIT_TEST(accessors_2);
  CPPUNIT_TEST(accessors_3);
  CPPUNIT_TEST(accessors_4);
  CPPUNIT_TEST(copycons_0);
  CPPUNIT_TEST(copycons_1);
  CPPUNIT_TEST(copycons_2);
  CPPUNIT_TEST(copycons_3);
  CPPUNIT_TEST(copycons_4);
  CPPUNIT_TEST(copycons_5);
  CPPUNIT_TEST(copycons_6);
  CPPUNIT_TEST(copycons_7);
  CPPUNIT_TEST(copycons_8);
  CPPUNIT_TEST(assign_0);
  CPPUNIT_TEST(assign_1);
  CPPUNIT_TEST(assign_2);
  CPPUNIT_TEST(assign_3);
  CPPUNIT_TEST(assign_4);
  CPPUNIT_TEST(assign_5);
  CPPUNIT_TEST(assign_6);
  CPPUNIT_TEST(assign_7);
  CPPUNIT_TEST(assign_8);
  CPPUNIT_TEST(tstampCons);
  CPPUNIT_TEST(fractionalRunTime);
  CPPUNIT_TEST(fromRawBuffer_0);
  CPPUNIT_TEST(fromRawBuffer_1);
  CPPUNIT_TEST_SUITE_END();

private:

public:
  void setUp() {
  }
  void tearDown() {
  }
protected:
  // Tests for items without bodyheaders.
  
  void basiccons();
  void fullcons_0();
  void fullcons_1();
  void torawringitem_0();
  void torawringitem_1();

  void comparison_0();
  void comparison_1();
  void comparison_2();
  void comparison_3();
  void comparison_4();
  void comparison_5();

  void accessors_0();
  void accessors_1();
  void accessors_2();
  void accessors_3();
  void accessors_4();

  void copycons_0();
  void copycons_1();
  void copycons_2();
  void copycons_3();
  void copycons_4();
  void copycons_5();
  void copycons_6();
  void copycons_7();
  void copycons_8();

  void assign_0();
  void assign_1();
  void assign_2();
  void assign_3();
  void assign_4();
  void assign_5();
  void assign_6();
  void assign_7();
  void assign_8();

  void tstampCons();
  
  // Tests for fractional seconds in run offsets.
  void fractionalRunTime();

  void fromRawBuffer_0();
  void fromRawBuffer_1();

  std::tuple<V12::CRingStateChangeItem, V12::CRingStateChangeItem>
  createCopies();
  
};

CPPUNIT_TEST_SUITE_REGISTRATION(teststate);

// Basic construction test.
// Make a state change item, and ensure it's properly initialized.
//
void teststate::basiccons() {
  V12::CRingStateChangeItem item(V12::END_RUN);
  EQMSG("type", V12::END_RUN, item.type());
  EQMSG("title", std::string(), item.getTitle());
  EQMSG("run number",(uint32_t)0, item.getRunNumber());
  EQMSG("offset",(uint32_t)0, item.getElapsedTime());
  EQMSG("offset divisor", uint32_t(1), item.getOffsetDivisor());
  
}

//
// Do a full construction and test that out.
//
void teststate::fullcons_0()
{
  // One where the title fits just fine.

  V12::CRingStateChangeItem item(V12::PAUSE_RUN,
                1234,
                5678,
                314159,
                "This title fits");
  EQ(V12::PAUSE_RUN, item.type());
  EQ((uint32_t)1234, item.getRunNumber());
  EQ((uint32_t)5678, item.getElapsedTime());
  EQ((time_t)(314159), item.getTimestamp());
}

void teststate::fullcons_1() {

  // The title does not fit in this one:

  string title("supercalifragalisticexpialidocious");
  while (title.size() <= 8192) {
    title += title;
  }

  CPPUNIT_ASSERT_NO_THROW(
              V12::CRingStateChangeItem(V12::RESUME_RUN,
                                        1234, 5678, 314159,
                                        title)
              );
}

// Test construction that effectively up-casts a CRingItem.
//
void
teststate::torawringitem_0()
{
  // Successful requires that the underlying ring item have the
  // right type.

  V12::CRingStateChangeItem original(V12::BEGIN_RUN, 12, 34, 56, "Testing");
  V12::CRawRingItem rawBuffer;

  original.toRawRingItem(rawBuffer);

  EQMSG("type", V12::BEGIN_RUN, rawBuffer.type());
  EQMSG("size", original.size(), rawBuffer.size());
  EQMSG("evt timestamp", original.getEventTimestamp(), rawBuffer.getEventTimestamp());
  EQMSG("source id", original.getSourceId(), rawBuffer.getSourceId());

  auto buffer = rawBuffer.getBody();
  Buffer::ContainerDeserializer<Buffer::ByteBuffer> stream(buffer);

  uint32_t run, offset, tstamp, divisor, titleLength;

  stream >> run >> offset >> tstamp >> divisor >> titleLength;

  char *pTitle = new char[titleLength];
  stream.extract<char>(pTitle, pTitle+titleLength);
  string title(pTitle, pTitle+titleLength);
  delete [] pTitle;

  EQMSG("run", uint32_t(12), run);
  EQMSG("timeOffset", uint32_t(34), offset);
  EQMSG("timestamp", uint32_t(56), tstamp);
  EQMSG("divisor", uint32_t(1), divisor);
  EQMSG("length", uint32_t(7), titleLength);
  EQMSG("title", std::string("Testing"), title);

}


void teststate::comparison_0()
{
    V12::CRingStateChangeItem item(V12::BEGIN_RUN);
    CPPUNIT_ASSERT_MESSAGE("identity comparison", item == item);
}

void teststate::comparison_1()
{
    V12::CRingStateChangeItem item1(V12::BEGIN_RUN, 12, 34, 56, "test");
    V12::CRingStateChangeItem item2(V12::END_RUN, 12, 34, 56, "test");

    CPPUNIT_ASSERT_MESSAGE("type different", item1 != item2);
}

void teststate::comparison_2()
{
    V12::CRingStateChangeItem item1(V12::BEGIN_RUN, 12, 34, 56, "test");
    V12::CRingStateChangeItem item2(V12::BEGIN_RUN, 235, 34, 56, "test");

    CPPUNIT_ASSERT_MESSAGE("run different", item1 != item2);
}

void teststate::comparison_3()
{
    V12::CRingStateChangeItem item1(V12::BEGIN_RUN, 12, 34, 56, "test");
    V12::CRingStateChangeItem item2(V12::BEGIN_RUN, 12, 3435, 56, "test");

    CPPUNIT_ASSERT_MESSAGE("offset different", item1 != item2);
}

void teststate::comparison_4()
{
    V12::CRingStateChangeItem item1(V12::BEGIN_RUN, 12, 34, 56, "test");
    V12::CRingStateChangeItem item2(V12::BEGIN_RUN, 12, 34, 2342, "test");

    CPPUNIT_ASSERT_MESSAGE("timestamp different", item1 != item2);
}

void teststate::comparison_5()
{
    V12::CRingStateChangeItem item1(V12::BEGIN_RUN, 12, 34, 56, "test");
    V12::CRingStateChangeItem item2(V12::BEGIN_RUN, 12, 34, 56, "test2");

    CPPUNIT_ASSERT_MESSAGE("title different", item1 != item2);
}

// Test the accessor functions.
void
teststate::accessors_0()
{
    V12::CRingStateChangeItem empty(V12::BEGIN_RUN);
    empty.setRunNumber(1234);
    EQ((uint32_t)1234, empty.getRunNumber());
}

void
teststate::accessors_1()
{
    V12::CRingStateChangeItem empty(V12::BEGIN_RUN);
    empty.setElapsedTime(45);
    EQ((uint32_t)45, empty.getElapsedTime());
}

void
teststate::accessors_2()
{
    V12::CRingStateChangeItem empty(V12::BEGIN_RUN);
    string title("this is my title");
    empty.setTitle(title);
    EQ(title, empty.getTitle());
}

void
teststate::accessors_3()
{
    V12::CRingStateChangeItem empty(V12::BEGIN_RUN);
    empty.setTimestamp(314159);
    EQ((time_t)(314159), empty.getTimestamp());
}

void
teststate::accessors_4()
{
    V12::CRingStateChangeItem empty(V12::BEGIN_RUN);
    empty.setOffsetDivisor(2345);
    EQ(uint32_t(2345),empty.getOffsetDivisor());
}

std::tuple<V12::CRingStateChangeItem, V12::CRingStateChangeItem>
teststate::createCopies()
{
    std::string title("This is a test title");

    V12::CRingStateChangeItem original(0x1234567887654321ll, 234,
                                       V12::BEGIN_RUN,
                                       1234, 0, (time_t)(56789),
                                       title);
    V12::CRingStateChangeItem copy(original); // Copy construction.

    return std::make_tuple(original, copy);
}

/**
 ** test copy construction.. need to wind up with the same sizes and contents as before.
 */
void teststate::copycons_0()
{

  V12::CRingStateChangeItem original(V12::VOID), copy(V12::VOID);
  std::tie(original, copy) = createCopies();
  EQMSG("size", original.size(), copy.size());
}
  // headers must match

void teststate::copycons_1()
{

    V12::CRingStateChangeItem original(V12::VOID), copy(V12::VOID);
    std::tie(original, copy) = createCopies();
  EQMSG("type", original.type(), copy.type());
}

void teststate::copycons_2()
{
    V12::CRingStateChangeItem original(V12::VOID), copy(V12::VOID);
    std::tie(original, copy) = createCopies();
    EQMSG("evt timestamp", original.getEventTimestamp(), copy.getEventTimestamp());

}

void teststate::copycons_3()
{
    V12::CRingStateChangeItem original(V12::VOID), copy(V12::VOID);
    std::tie(original, copy) = createCopies();
    EQMSG("source id", original.getSourceId(), copy.getSourceId());
}

// Contents must match:
void teststate::copycons_4()
{
    V12::CRingStateChangeItem original(V12::VOID), copy(V12::VOID);
    std::tie(original, copy) = createCopies();
    EQMSG("run", original.getRunNumber(), copy.getRunNumber());
}

void teststate::copycons_5()
{
    V12::CRingStateChangeItem original(V12::VOID), copy(V12::VOID);
    std::tie(original, copy) = createCopies();
    EQMSG("elapsed time", original.getElapsedTime(), copy.getElapsedTime());
}

void teststate::copycons_6()
{
    V12::CRingStateChangeItem original(V12::VOID), copy(V12::VOID);
    std::tie(original, copy) = createCopies();
    EQMSG("title", original.getTitle(), copy.getTitle());
}

void teststate::copycons_7()
{
    V12::CRingStateChangeItem original(V12::VOID), copy(V12::VOID);
    std::tie(original, copy) = createCopies();
    EQMSG("timestamp", original.getTimestamp(), copy.getTimestamp());
}

void teststate::copycons_8()
{
    V12::CRingStateChangeItem original(V12::VOID), copy(V12::VOID);
    std::tie(original, copy) = createCopies();
    EQMSG("offset divisor", original.getOffsetDivisor(), copy.getOffsetDivisor());
}

void teststate::assign_0()
{
    V12::CRingStateChangeItem original(V12::RESUME_RUN, 12, 23, 45, "title" );
    V12::CRingStateChangeItem copy(V12::BEGIN_RUN);

    copy = original;
    EQMSG("size", original.size(), copy.size());
}
  // headers must match

void teststate::assign_1()
{
    V12::CRingStateChangeItem original(V12::RESUME_RUN, 12, 23, 45, "title" );
    V12::CRingStateChangeItem copy(V12::BEGIN_RUN);

    copy = original;
  EQMSG("type", original.type(), copy.type());
}

void teststate::assign_2()
{
    V12::CRingStateChangeItem original(V12::RESUME_RUN, 12, 23, 45, "title" );
    V12::CRingStateChangeItem copy(V12::BEGIN_RUN);

    copy = original;
    EQMSG("evt timestamp", original.getEventTimestamp(), copy.getEventTimestamp());

}

void teststate::assign_3()
{
    V12::CRingStateChangeItem original(V12::RESUME_RUN, 12, 23, 45, "title" );
    V12::CRingStateChangeItem copy(V12::BEGIN_RUN);

    copy = original;
    EQMSG("source id", original.getSourceId(), copy.getSourceId());
}

// Contents must match:
void teststate::assign_4()
{
    V12::CRingStateChangeItem original(V12::RESUME_RUN, 12, 23, 45, "title" );
    V12::CRingStateChangeItem copy(V12::BEGIN_RUN);

    copy = original;
    EQMSG("run", original.getRunNumber(), copy.getRunNumber());
}

void teststate::assign_5()
{
    V12::CRingStateChangeItem original(V12::RESUME_RUN, 12, 23, 45, "title" );
    V12::CRingStateChangeItem copy(V12::BEGIN_RUN);

    copy = original;
    EQMSG("elapsed time", original.getElapsedTime(), copy.getElapsedTime());
}

void teststate::assign_6()
{
    V12::CRingStateChangeItem original(V12::RESUME_RUN, 12, 23, 45, "title" );
    V12::CRingStateChangeItem copy(V12::BEGIN_RUN);

    copy = original;
    EQMSG("title", original.getTitle(), copy.getTitle());
}

void teststate::assign_7()
{
    V12::CRingStateChangeItem original(V12::RESUME_RUN, 12, 23, 45, "title" );
    V12::CRingStateChangeItem copy(V12::BEGIN_RUN);

    copy = original;
    EQMSG("timestamp", original.getTimestamp(), copy.getTimestamp());
}

void teststate::assign_8()
{
    V12::CRingStateChangeItem original(V12::RESUME_RUN, 12, 23, 45, "title" );
    V12::CRingStateChangeItem copy(V12::BEGIN_RUN);

    copy = original;
    EQMSG("offset divisor", original.getOffsetDivisor(), copy.getOffsetDivisor());
}


/*
  Test constructors of items with timestamps:
*/
void teststate::tstampCons()
{
    V12::CRingStateChangeItem item(
        0x1234567890123456ll, 1, V12::BEGIN_RUN, 1234, 0, (time_t)666, "This is a test"
    );
    
    // Ensure there's a body header and tht we can get the bits and pieces from it.

    EQMSG("evt timestamp", static_cast<uint64_t>(0x1234567890123456ll), item.getEventTimestamp());
    EQMSG("source id", static_cast<uint32_t>(1), item.getSourceId());
    
    // Ensure we cfan still get the other goodies:
    
    EQMSG("type", V12::BEGIN_RUN, item.type());
    EQMSG("run", static_cast<uint32_t>(1234), item.getRunNumber());
    EQMSG("elapsed time", static_cast<uint32_t>(0), item.getElapsedTime());
    EQMSG("title", std::string("This is a test"), item.getTitle());
    EQMSG("timestamp", static_cast<time_t>(666), item.getTimestamp());
    EQMSG("divisor", static_cast<uint32_t>(1), item.getOffsetDivisor());

}

/*
  Ensure fractional run time offsets work.
*/
void
teststate::fractionalRunTime()
{
    V12::CRingStateChangeItem orig(0x1234567890123456ll, 1, V12::END_RUN,
                                   1234, 5, (time_t)666, "This is a test",
                                   2);
    float offset = orig.computeElapsedTime();
    EQMSG("fractional time", static_cast<float>(5.0/2.0), offset);
}


void teststate::fromRawBuffer_0()
{

    uint32_t size = 47;
    uint32_t sourceId = 12;
    uint64_t evtTstamp = 34;
    uint32_t run = 3452;
    uint32_t offset = 234234;
    uint32_t divisor = 7890;
    time_t timestamp = 1234565;
    char title[8] = "Testing";

    Buffer::ByteBuffer buffer;
    buffer << size;
    buffer << V12::BEGIN_RUN;
    buffer << evtTstamp;
    buffer << sourceId;
    buffer << run;
    buffer << offset;
    buffer << uint32_t(timestamp);
    buffer << divisor;
    buffer << uint32_t(7);
    buffer.insert(buffer.end(), title, title+7);

    V12::CRawRingItem rawItem(buffer);
    V12::CRingStateChangeItem item(rawItem);

    EQMSG("evt timestamp", evtTstamp, item.getEventTimestamp());
    EQMSG("source id", sourceId, item.getSourceId());
    EQMSG("type", V12::BEGIN_RUN, item.type());
    EQMSG("run", run, item.getRunNumber());
    EQMSG("elapsed time", offset, item.getElapsedTime());
    EQMSG("timestamp", timestamp, item.getTimestamp());
    EQMSG("divisor", divisor, item.getOffsetDivisor());
    EQMSG("title", std::string(title), item.getTitle());

}


void teststate::fromRawBuffer_1()
{

    V12::CRawRingItem rawItem;

    CPPUNIT_ASSERT_THROW_MESSAGE(
                "cannot construct from raw buffer with non state change type",
                V12::CRingStateChangeItem item(rawItem),
                std::bad_cast);
}
