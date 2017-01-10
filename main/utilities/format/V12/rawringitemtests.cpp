// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"

#include <V12/DataFormat.h>
#include <V12/CRawRingItem.h>
#include <ByteBuffer.h>
#include <iostream>

using namespace std;
using namespace DAQ;

std::ostream& operator<<(std::ostream& stream, const Buffer::ByteBuffer& buffer) {

    stream << "(";
    for (auto& byte : buffer) stream << int(byte) << " ";
    stream << ")";
    return stream;
}


class CRawRingItemTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(CRawRingItemTests);
  CPPUNIT_TEST(simpleCtor_0);
  CPPUNIT_TEST(simpleCtor_1);
  CPPUNIT_TEST(simpleCtor_2);
  CPPUNIT_TEST(simpleCtor_3);
  CPPUNIT_TEST(simpleCtor_4);
  CPPUNIT_TEST(simpleCtor_5);
  CPPUNIT_TEST(constructor_0);
  CPPUNIT_TEST(constructor_1);
  CPPUNIT_TEST(constructor_2);
  CPPUNIT_TEST(constructor_3);
  CPPUNIT_TEST(constructor_4);
  CPPUNIT_TEST(constructor_5);
  CPPUNIT_TEST(copyConstruct_0);
  CPPUNIT_TEST(copyConstruct_1);
  CPPUNIT_TEST(copyConstruct_2);
  CPPUNIT_TEST(copyConstruct_3);
  CPPUNIT_TEST(copyConstruct_4);
  CPPUNIT_TEST(copyConstruct_5);
  CPPUNIT_TEST(constructFromBuffer_0);
  CPPUNIT_TEST(constructFromBuffer_1);
  CPPUNIT_TEST(constructFromBuffer_2);
  CPPUNIT_TEST(constructFromBuffer_3);
  CPPUNIT_TEST(constructFromBuffer_4);
  CPPUNIT_TEST(constructFromBuffer_5);
  CPPUNIT_TEST(constructFromSwappedBuffer_0);
  CPPUNIT_TEST(constructFromSwappedBuffer_1);
  CPPUNIT_TEST(constructFromSwappedBuffer_2);
  CPPUNIT_TEST(constructFromSwappedBuffer_3);
  CPPUNIT_TEST(constructFromSwappedBuffer_4);
  CPPUNIT_TEST(constructFromSwappedBuffer_5);
  CPPUNIT_TEST(isComposite_0);
  CPPUNIT_TEST(isComposite_1);
  CPPUNIT_TEST(toString_0);
  CPPUNIT_TEST(toString_1);
  CPPUNIT_TEST(compare_0);
  CPPUNIT_TEST(compare_1);
  CPPUNIT_TEST(compare_2);
  CPPUNIT_TEST(compare_3);
  CPPUNIT_TEST(compare_4);
  CPPUNIT_TEST(compare_5);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp() {
  }
  void tearDown() {
  }

  void simpleCtor_0() {
        V12::CRawRingItem item(23);
        EQMSG("type", uint32_t(23), item.type());
  }

  void simpleCtor_1() {
        V12::CRawRingItem item(23);
        EQMSG("sourceId", uint32_t(0), item.getSourceId());
  }

  void simpleCtor_2() {
        V12::CRawRingItem item(23);
        EQMSG("tstamp", V12::NULL_TIMESTAMP, item.getEventTimestamp());
  }
  void simpleCtor_3() {
        V12::CRawRingItem item(23);
        EQMSG("body empty", size_t(0), item.getBody().size());
  }

  void simpleCtor_4() {
        V12::CRawRingItem item(23);
        EQMSG("size", uint32_t(20), item.size());
  }

  void simpleCtor_5 () {

      V12::CRawRingItem item(23);
      EQMSG("swap", false, item.mustSwap());

  }


  void constructor_0() {
      V12::CRawRingItem item(23, 0x123456789, 12, {0,1,2,3,4});
      EQMSG("type", uint32_t(23), item.type());
  }

  void constructor_1() {
      V12::CRawRingItem item(23, 0x123456789, 12, {0,1,2,3,4});
      EQMSG("source id", uint32_t(12), item.getSourceId());
  }

  void constructor_2() {
      V12::CRawRingItem item(23, 0x123456789, 12, {0,1,2,3,4});
      EQMSG("tstamp", uint64_t(0x123456789), item.getEventTimestamp());
  }

  void constructor_3() {
      V12::CRawRingItem item(23, 0x123456789, 12, {0,1,2,3,4});
      EQMSG("body", Buffer::ByteBuffer({0,1,2,3,4}), item.getBody());
  }

  void constructor_4() {
      V12::CRawRingItem item(23, 0x123456789, 12, {0,1,2,3,4});
      EQMSG("size", uint32_t(25), item.size());
  }

  void constructor_5() {
      V12::CRawRingItem item(23, 0x123456789, 12, {0,1,2,3,4});
      EQMSG("swap", false, item.mustSwap());
  }



  void copyConstruct_0 () {
      V12::CRawRingItem item(32, 0x123456789, 12, {0,1,2,3,4});
      V12::CRawRingItem copy(item);
      EQMSG("type", uint32_t(32), copy.type());

  }

  void copyConstruct_1 () {
      V12::CRawRingItem item(32, 0x123456789, 12, {0,1,2,3,4});
      V12::CRawRingItem copy(item);
      EQMSG("source id", uint32_t(12), copy.getSourceId());
  }

  void copyConstruct_2 () {
      V12::CRawRingItem item(32, 0x123456789, 12, {0,1,2,3,4});
      V12::CRawRingItem copy(item);
      EQMSG("tstamp", uint64_t(0x123456789), copy.getEventTimestamp());
  }

  void copyConstruct_3 () {
      V12::CRawRingItem item(32, 0x123456789, 12, {0,1,2,3,4});
      V12::CRawRingItem copy(item);
      EQMSG("body", Buffer::ByteBuffer({0,1,2,3,4}), copy.getBody());
  }

  void copyConstruct_4 () {
      V12::CRawRingItem item(32, 0x123456789, 12, {0,1,2,3,4});
      V12::CRawRingItem copy(item);
      EQMSG("size", uint32_t(25), copy.size());
  }

  void copyConstruct_5 () {
      V12::CRawRingItem item(32, 0x123456789, 12, {0,1,2,3,4});
      V12::CRawRingItem copy(item);
      EQMSG("swap", false, copy.mustSwap());
  }


  Buffer::ByteBuffer generateRawData() {
      uint32_t type = 1234;
      uint64_t tstamp = 0x56781234;
      uint32_t id = 345;
      uint32_t size = 21;
      Buffer::ByteBuffer body = {9};

      Buffer::ByteBuffer buffer;
      buffer << size;
      buffer << type;
      buffer << tstamp;
      buffer << id;
      buffer << body;

      return buffer;
  }

  void constructFromBuffer_0 () {

      V12::CRawRingItem item(generateRawData());
      EQMSG("type", uint32_t(1234), item.type());

  }

  void constructFromBuffer_1 () {

      V12::CRawRingItem item(generateRawData());
      EQMSG("source id", uint32_t(345), item.getSourceId());

  }

  void constructFromBuffer_2 () {

      V12::CRawRingItem item(generateRawData());
      EQMSG("size", uint32_t(21), item.size());

  }

  void constructFromBuffer_3 () {

      V12::CRawRingItem item(generateRawData());
      EQMSG("body", Buffer::ByteBuffer({9}), item.getBody());

  }

  void constructFromBuffer_4 () {

      V12::CRawRingItem item(generateRawData());
      EQMSG("swap", false, item.mustSwap());

  }
  void constructFromBuffer_5 () {

      V12::CRawRingItem item(generateRawData());
      EQMSG("tstamp", uint64_t(0x56781234), item.getEventTimestamp());

  }


  Buffer::ByteBuffer generateSwappedRawData() {
      uint32_t type = 0x34120000;
      uint64_t tstamp = 0x3412785600000000;
      uint32_t id = 0x45030000;
      uint32_t size = 0x15000000;
      Buffer::ByteBuffer body = {9};

      Buffer::ByteBuffer buffer;
      buffer << size;
      buffer << type;
      buffer << tstamp;
      buffer << id;
      buffer << body;

      return buffer;
  }

  void constructFromSwappedBuffer_0 () {

      V12::CRawRingItem item(generateSwappedRawData());
      EQMSG("type", uint32_t(0x1234), item.type());

  }

  void constructFromSwappedBuffer_1 () {

      V12::CRawRingItem item(generateSwappedRawData());
      EQMSG("source id", uint32_t(0x345), item.getSourceId());

  }

  void constructFromSwappedBuffer_2 () {

      V12::CRawRingItem item(generateSwappedRawData());
      EQMSG("size", uint32_t(21), item.size());

  }

  void constructFromSwappedBuffer_3 () {

      V12::CRawRingItem item(generateSwappedRawData());
      EQMSG("body", Buffer::ByteBuffer({9}), item.getBody());

  }

  void constructFromSwappedBuffer_4 () {

      V12::CRawRingItem item(generateSwappedRawData());
      EQMSG("tstamp", uint64_t(0x56781234), item.getEventTimestamp());

  }

  void constructFromSwappedBuffer_5 () {

      V12::CRawRingItem item(generateSwappedRawData());
      EQMSG("swap", true, item.mustSwap());

  }


  void isComposite_0() {
      V12::CRawRingItem item(V12::BEGIN_RUN);
      EQMSG("not composite", false, item.isComposite());
  }

  void isComposite_1() {
      V12::CRawRingItem item(V12::COMP_BEGIN_RUN);
      EQMSG("composite", true, item.isComposite());
  }

  void toString_0() {

      V12::CRawRingItem item(1, 23, 1234, {2,3});
      std::string msg;
      msg += "Size (bytes): 22\n";
      msg += "Type:         RawRingItem\n";
      msg += "Timestamp:    23\n";
      msg += "Source Id:    1234\n";
      msg += "0302 \n";

      EQMSG("toString (even byte count)", msg, item.toString());
  }

  void toString_1() {
      V12::CRawRingItem item(1, 23, 1234, {2});
      std::string msg;
      msg += "Size (bytes): 21\n";
      msg += "Type:         RawRingItem\n";
      msg += "Timestamp:    23\n";
      msg += "Source Id:    1234\n";
      msg += "02 \n";

      EQMSG("toString (odd byte count)", msg, item.toString());
  }

  void compare_0() {
    V12::CRawRingItem item1(1, 23, 1234, {2});

    CPPUNIT_ASSERT_MESSAGE("comparison", item1 == item1);
  }

  void compare_1() {
    V12::CRawRingItem item1(1, 23, 1234, {2});
    V12::CRawRingItem item2(2, 45, 5678, {3});

    CPPUNIT_ASSERT_MESSAGE("different", item1 != item2);
  }

  void compare_2() {
    V12::CRawRingItem item1(1, 23, 1234, {2});
    V12::CRawRingItem item2(2, 23, 1234, {2});

    CPPUNIT_ASSERT_MESSAGE("different", item1 != item2);
  }

  void compare_3() {
    V12::CRawRingItem item1(1, 23, 1234, {2});
    V12::CRawRingItem item2(1, 34, 1234, {2});

    CPPUNIT_ASSERT_MESSAGE("different", item1 != item2);
  }

  void compare_4() {
    V12::CRawRingItem item1(1, 23, 1234, {2});
    V12::CRawRingItem item2(1, 23, 5678, {2});

    CPPUNIT_ASSERT_MESSAGE("different", item1 != item2);
  }

  void compare_5() {
    V12::CRawRingItem item1(1, 23, 1234, {2});
    V12::CRawRingItem item2(1, 23, 1234, {3});

    CPPUNIT_ASSERT_MESSAGE("different", item1 != item2);
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(CRawRingItemTests);
