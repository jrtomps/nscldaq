// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"

#include <V12/DataFormat.h>
#include <V12/CRingTextItem.h>
#include <V12/CRawRingItem.h>
#include <Deserializer.h>

#include <string>
#include <vector>
#include <time.h>
#include <string.h>
#include <iostream>

using namespace std;
using namespace DAQ;

class texttests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(texttests);
  CPPUNIT_TEST(simplecons_0);
  CPPUNIT_TEST(simplecons_1);
  CPPUNIT_TEST(fullcons_0);
  CPPUNIT_TEST(fullcons_1);
  CPPUNIT_TEST(castcons_0);
  CPPUNIT_TEST(castcons_1);
  CPPUNIT_TEST(accessors_0);
  CPPUNIT_TEST(comparison_0);
  CPPUNIT_TEST(comparison_1);
  CPPUNIT_TEST(comparison_2);
  CPPUNIT_TEST(comparison_3);
  CPPUNIT_TEST(comparison_4);
  CPPUNIT_TEST(comparison_5);
  CPPUNIT_TEST(comparison_6);
  CPPUNIT_TEST(comparison_7);
  CPPUNIT_TEST(copycons);
  CPPUNIT_TEST(tscons_0);
  CPPUNIT_TEST(tscons_1);
  CPPUNIT_TEST(fractionalRunTime);
  CPPUNIT_TEST(assign_0);
  CPPUNIT_TEST(toRawRingItem_0);
  CPPUNIT_TEST_SUITE_END();


private:

public:
  void setUp() {
  }
  void tearDown() {
  }
protected:
  void simplecons_0();
  void simplecons_1();
  void fullcons_0();
  void fullcons_1();
  void castcons_0();
  void castcons_1();
  void accessors_0();
  void comparison_0();
  void comparison_1();
  void comparison_2();
  void comparison_3();
  void comparison_4();
  void comparison_5();
  void comparison_6();
  void comparison_7();
  void copycons();
  void tscons_0();
  void tscons_1();
  void fractionalRunTime();
  void assign_0();
  void toRawRingItem_0();
};

CPPUNIT_TEST_SUITE_REGISTRATION(texttests);

// Test simple construction

void texttests::simplecons_0() {
  // Make some strings:

  string s1("String 1");
  string s2("string 2");
  string s3("string 3");
  string s4("last string");

  vector<string> strings;
  strings.push_back(s1);
  strings.push_back(s2);
  strings.push_back(s3);
  strings.push_back(s4);

  V12::CRingTextItem item(V12::PACKET_TYPES, strings);
  
  
  EQMSG("offset", (uint32_t)0, item.getTimeOffset());
  EQMSG("string count", (uint32_t)4, item.getStringCount());

  // Check the contents:

  std::vector<std::string> storedStrings = item.getStrings();
  EQ(s1, storedStrings[0]);
  EQ(s2, storedStrings[1]);
  EQ(s3, storedStrings[2]);
  EQ(s4, storedStrings[3]);

}

void texttests::simplecons_1() {
  // Make some strings:

  CPPUNIT_ASSERT_THROW_MESSAGE( "text items must have a text type",
                                V12::CRingTextItem item(V12::VOID, {}),
                                std::invalid_argument);
}

// Test the full constructor.

void texttests::fullcons_0()
{
  string s1("String 1");
  string s2("string 2");
  string s3("string 3");
  string s4("last string");

  vector<string> strings;
  strings.push_back(s1);
  strings.push_back(s2);
  strings.push_back(s3);
  strings.push_back(s4);

  V12::CRingTextItem item(V12::PACKET_TYPES, strings,
             1234,
             5678);
  
  EQMSG("offset",(uint32_t)1234, item.getTimeOffset());
  EQMSG("string count", (uint32_t)4, item.getStringCount());
  EQMSG("timestamp", (time_t)5678, item.getTimestamp());

  // Check the contents:

  auto& itemStrings = item.getStrings();
  EQ(s1, itemStrings[0]);
  EQ(s2, itemStrings[1]);
  EQ(s3, itemStrings[2]);
  EQ(s4, itemStrings[3]);
}


void texttests::fullcons_1() {
    CPPUNIT_ASSERT_THROW_MESSAGE( "text items must have a text type",
                                  V12::CRingTextItem item(V12::VOID, {}, 0, 0),
                                  std::invalid_argument);
}

// test the 'casting' constructor.

void texttests::castcons_0()
{
  // Create a RingItem that is actually a text ring item:

  V12::CRawRingItem ritem;
  ritem.setType(V12::PACKET_TYPES);
  ritem.setSourceId(123);
  ritem.setEventTimestamp(0x12345654321);

  auto& body = ritem.getBody();
  body << uint32_t(1234);
  body << uint32_t(4321);
  body << uint32_t(4);
  body << uint32_t(1);

  string s1("String 1");
  string s2("string 2");
  string s3("string 3");
  string s4("last string");

  body.insert(body.end(), s1.begin(), s1.end());
  body.push_back(0); // null terminate
  body.insert(body.end(), s2.begin(), s2.end());
  body.push_back(0);// null terminate
  body.insert(body.end(), s3.begin(), s3.end());
  body.push_back(0);// null terminate
  body.insert(body.end(), s4.begin(), s4.end());
  body.push_back(0);// null terminate

  V12::CRingTextItem item(ritem);
  EQMSG("offset",(uint32_t)1234, item.getTimeOffset());
  EQMSG("timestamp", (time_t)4321, item.getTimestamp());
  EQMSG("count", (uint32_t)4, item.getStringCount());
  EQMSG("divisor", uint32_t(1), item.getTimeDivisor());

  auto strings = item.getStrings();
  EQMSG("string 0", s1, strings[0]);
  EQMSG("string 1", s2, strings[1]);
  EQMSG("string 2", s3, strings[2]);
  EQMSG("string 3", s4, strings[3]);

}

void texttests::castcons_1()
{
    V12::CRawRingItem bad;
    CPPUNIT_ASSERT_THROW_MESSAGE( "cannot construct from non-text type",
                                  V12::CRingTextItem item(bad),
                                  std::bad_cast );
}


// test the accessor functions:

void texttests::accessors_0()
{
  string s1("String 1");
  string s2("string 2");
  string s3("string 3");
  string s4("last string");

  vector<string> strings;
  strings.push_back(s1);
  strings.push_back(s2);
  strings.push_back(s3);
  strings.push_back(s4);


  V12::CRingTextItem item(V12::PACKET_TYPES, strings);

  vector<string> content = item.getStrings();
  ASSERT(strings == content);

  //  The simpler ones.

  item.setEventTimestamp(0x1234235);
  EQMSG("evt timestamp", uint64_t(0x1234235), item.getEventTimestamp());

  item.setSourceId(890);
  EQMSG("source id", uint32_t(890), item.getSourceId());

  item.setTimeOffset(1234);
  EQMSG("offset", (uint32_t)1234, item.getTimeOffset());

  item.setTimestamp(66776);
  EQMSG("timestamp", (time_t)66776, item.getTimestamp());

  item.setTimeDivisor(54733);
  EQMSG("divisor", uint32_t(54733), item.getTimeDivisor());

  CPPUNIT_ASSERT_NO_THROW_MESSAGE("Text item  can become other text type",
                               item.setType(V12::MONITORED_VARIABLES));

  CPPUNIT_ASSERT_THROW_MESSAGE("Cannot set text item to non-text type",
                               item.setType(V12::PHYSICS_EVENT),
                               std::invalid_argument);

  item.setType(V12::MONITORED_VARIABLES);
  EQMSG("type", V12::MONITORED_VARIABLES, item.type());



}


void texttests::comparison_0()
{
    V12::CRingTextItem item(V12::PACKET_TYPES, {"s0", "s1", "s2"});

    CPPUNIT_ASSERT_MESSAGE("identity comparison", item == item);
}

void texttests::comparison_1()
{
    V12::CRingTextItem item(V12::PACKET_TYPES, 0x12345, 12, {"s1"}, 234, 567, 2);
    V12::CRingTextItem item2(V12::MONITORED_VARIABLES, 0x12345, 12, {"s1"}, 234, 567, 2);

    CPPUNIT_ASSERT_MESSAGE("different type", item != item2);

}

void texttests::comparison_2()
{
    V12::CRingTextItem item(V12::PACKET_TYPES, 0x12345, 12, {"s1"}, 234, 567, 2);
    V12::CRingTextItem item2(V12::PACKET_TYPES, 0x22345, 12, {"s1"}, 234, 567, 2);

    CPPUNIT_ASSERT_MESSAGE("different evt timestamp", item != item2);
}


void texttests::comparison_3()
{
    V12::CRingTextItem item(V12::PACKET_TYPES, 0x12345, 12, {"s1"}, 234, 567, 2);
    V12::CRingTextItem item2(V12::PACKET_TYPES, 0x12345, 22, {"s1"}, 234, 567, 2);

    CPPUNIT_ASSERT_MESSAGE("different source ids", item != item2);
}

void texttests::comparison_4()
{
    V12::CRingTextItem item(V12::PACKET_TYPES, 0x12345, 12, {"s1"}, 234, 567, 2);
    V12::CRingTextItem item2(V12::PACKET_TYPES, 0x12345, 12, {"s2"}, 234, 567, 2);

    CPPUNIT_ASSERT_MESSAGE("different strings", item != item2);
}

void texttests::comparison_5()
{
    V12::CRingTextItem item(V12::PACKET_TYPES, 0x12345, 12, {"s1"}, 234, 567, 2);
    V12::CRingTextItem item2(V12::PACKET_TYPES, 0x12345, 12, {"s1"}, 235, 567, 2);

    CPPUNIT_ASSERT_MESSAGE("different offsets", item != item2);
}

void texttests::comparison_6()
{
    V12::CRingTextItem item(V12::PACKET_TYPES, 0x12345, 12, {"s1"}, 234, 567, 2);
    V12::CRingTextItem item2(V12::PACKET_TYPES, 0x12345, 12, {"s1"}, 234, 568, 2);

    CPPUNIT_ASSERT_MESSAGE("different timestamps", item != item2);
}

void texttests::comparison_7()
{
    V12::CRingTextItem item(V12::PACKET_TYPES, 0x12345, 12, {"s1"}, 234, 567, 2);
    V12::CRingTextItem item2(V12::PACKET_TYPES, 0x12345, 12, {"s1"}, 234, 567, 3);

    CPPUNIT_ASSERT_MESSAGE("different divisor", item != item2);
}

// test coyp construction.

void texttests::copycons()
{
  vector<std::string> testStrings; // With apologies to Theo Geissle aka Dr. Seuss.
  testStrings.push_back("one string");
  testStrings.push_back("two strings");
  testStrings.push_back("three strings more");
  testStrings.push_back("red string");
  testStrings.push_back("blue string");
  testStrings.push_back("theres the door..."); // added later :)

  V12::CRingTextItem original(V12::MONITORED_VARIABLES,
             testStrings,
             1234, 5678);
  V12::CRingTextItem copy(original);

  ASSERT(original == copy);
}
//// Test construction with timestamps:

void
texttests::tscons_0()
{
    string s1("String 1");
    string s2("string 2");
    string s3("string 3");
    string s4("last string");
  
    vector<string> strings;
    strings.push_back(s1);
    strings.push_back(s2);
    strings.push_back(s3);
    strings.push_back(s4);

    time_t stamp = time(NULL);
  
    size_t stringSize = 0;
    for (int i = 0; i < strings.size(); i++) {
        stringSize += strlen(strings[i].c_str()) + 1;
    }
  
    V12::CRingTextItem item(
      V12::PACKET_TYPES, static_cast<uint64_t>(0x8877665544332211ll), 1,
      strings, 1234, stamp, 2
    );

    //Check out the header:
    
    EQMSG("type", V12::PACKET_TYPES, item.type());
    EQMSG("size", uint32_t(75), item.size());
    
    EQMSG("evt timestamp", uint64_t(0x8877665544332211ll),
          item.getEventTimestamp());
    EQMSG("source id", uint32_t(1), item.getSourceId());

    // Ensure the body pointers from pItem and from the class jibe:
    
    // Check the body contents against what was constructed:
    
    EQMSG("offset", uint32_t(1234), item.getTimeOffset());
    EQMSG("timestamp", time_t(stamp), item.getTimestamp());
    EQMSG("string count", uint32_t(strings.size()), item.getStringCount());
    EQMSG("offset divisor", uint32_t(2), item.getTimeDivisor());

    auto itemStrings = item.getStrings();
    for (int i = 0; i < strings.size(); i++) {
        EQ(strings[i], itemStrings[i]);
    }
}

void
texttests::tscons_1()
{
    CPPUNIT_ASSERT_THROW_MESSAGE("text item must have text type",
                                 V12::CRingTextItem item(V12::BEGIN_RUN, 0, 1, {}, 1234, 0, 2 ),
                                 std::invalid_argument);

}


// Test computeElapsedTime
//
void
texttests::fractionalRunTime()
{
    V12::CRingTextItem item(V12::PACKET_TYPES, 0, 0, {}, 1234, time(nullptr), 3);
    EQ(float(1234.0/3.0), item.computeElapsedTime());
}

void texttests::assign_0()
{
    V12::CRingTextItem item(V12::PACKET_TYPES, 0, 0, {}, 1234, time(nullptr), 3);
    V12::CRingTextItem item2(V12::MONITORED_VARIABLES, {});

    item2 = item;

    ASSERT(item == item2);
}



void texttests::toRawRingItem_0()
{
    V12::CRawRingItem empty;
    V12::CRingTextItem item(V12::MONITORED_VARIABLES, 12, 34, {"56", "78"}, 1234, 56, 78);

    item.toRawRingItem(empty);

    EQMSG("size", item.size(), empty.size());
    EQMSG("type", item.type(), empty.type());
    EQMSG("evt timestamp", item.getEventTimestamp(), empty.getEventTimestamp());
    EQMSG("source id", item.getSourceId(), empty.getSourceId());

    auto& body = empty.getBody();
    uint32_t offset, timestamp, divisor, count;
    unsigned char temp;
    char str0[2], str1[2];
    Buffer::ContainerDeserializer<Buffer::ByteBuffer> stream(body);
    stream >> offset >> timestamp >> count >> divisor;
    stream.extract(str0, str0+2);
    stream.extract(temp);
    stream.extract(str1, str1+2);
    stream.extract(temp);

    EQMSG("offset", uint32_t(1234), offset);
    EQMSG("timestamp", uint32_t(56), timestamp);
    EQMSG("divisor", uint32_t(78), divisor);
    EQMSG("count", uint32_t(2), count);
    EQMSG("string 0", std::string("56"), std::string(str0, str0+2));
    EQMSG("string 1", std::string("78"), std::string(str1, str1+2));


}
