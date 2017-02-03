
// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include <string>

#include "Asserts.h"
#include "V12/DataFormat.h"
#include "V12/CDataFormatItem.h"
#include "V12/CRawRingItem.h"
#include "ByteBuffer.h"

#include <iostream>
#include <sstream>

// Tests for glom parameter ring item class:

using namespace DAQ;
using namespace DAQ::V12;

class CDataFormatItemTests : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CDataFormatItemTests);
    CPPUNIT_TEST(defaultConstructor_0);
    CPPUNIT_TEST(constructor_0);
    CPPUNIT_TEST(fromRawRingItem_0);
    CPPUNIT_TEST(fromRawRingItem_1);
    CPPUNIT_TEST(fromRawRingItem_2);
    CPPUNIT_TEST(fromSwappedRawRingItem_0);
    CPPUNIT_TEST(setType_0);
    CPPUNIT_TEST(setType_1);
    CPPUNIT_TEST(comparison_0);
    CPPUNIT_TEST(comparison_1);
    CPPUNIT_TEST(comparison_2);
    CPPUNIT_TEST(comparison_3);
    CPPUNIT_TEST(comparison_4);
    CPPUNIT_TEST(toRawRingItem_0);
    CPPUNIT_TEST(toRawRingItem_1);
    CPPUNIT_TEST(toString_0);
    CPPUNIT_TEST_SUITE_END();

private:

public:
  void setUp() {
  }
  void tearDown() {
  }
protected:

  void defaultConstructor_0() {
      CDataFormatItem item;
      EQMSG("size", uint32_t(24), item.size());
      EQMSG("type", RING_FORMAT, item.type());
      EQMSG("evt tstamp", NULL_TIMESTAMP, item.getEventTimestamp());
      EQMSG("source id", uint32_t(0), item.getSourceId());
      EQMSG("major", uint16_t(12), item.getMajor());
      EQMSG("minor", uint16_t(0), item.getMinor());
  }

  void constructor_0() {
      CDataFormatItem item(234, 12357, 34, 12);
      EQMSG("size", uint32_t(24), item.size());
      EQMSG("type", RING_FORMAT, item.type());
      EQMSG("evt tstamp", uint64_t(234), item.getEventTimestamp());
      EQMSG("source id", uint32_t(12357), item.getSourceId());
      EQMSG("major", uint16_t(34), item.getMajor());
      EQMSG("minor", uint16_t(12), item.getMinor());
  }

  void fromRawRingItem_0() {
      CRawRingItem rawItem(RING_FORMAT, 234, 23457);
      rawItem.getBody() << uint16_t(12) << uint16_t(234);


      CDataFormatItem item(rawItem);
      EQMSG("size", uint32_t(24), item.size());
      EQMSG("type", RING_FORMAT, item.type());
      EQMSG("evt tstamp", uint64_t(234), item.getEventTimestamp());
      EQMSG("source id", uint32_t(23457), item.getSourceId());
      EQMSG("major", uint16_t(12), item.getMajor());
      EQMSG("minor", uint16_t(234), item.getMinor());
  }

  void fromRawRingItem_1() {
      CRawRingItem rawItem(RING_FORMAT, 234, 23457);

      CPPUNIT_ASSERT_THROW_MESSAGE("construction from incomplete buffer fails",
                                   CDataFormatItem item(rawItem),
                                   std::runtime_error);
  }

  void fromRawRingItem_2() {
      CRawRingItem rawItem(PHYSICS_EVENT, 234, 23457);

      CPPUNIT_ASSERT_THROW_MESSAGE("construction from non RING_FORMAT fails",
                                   CDataFormatItem item(rawItem),
                                   std::runtime_error);
  }



  void fromSwappedRawRingItem_0() {

      // same content as for fromRawRingItem_0 except that it is byte swapped
      Buffer::ByteBuffer buffer;
      buffer << uint32_t(0x18000000) << uint32_t(0x0c000000);
      buffer << uint64_t(0xea00000000000000);
      buffer << uint32_t(0xa15b0000);
      buffer << uint16_t(0x0c00) << uint16_t(0xea00);

      CRawRingItem rawItem(buffer);
      CDataFormatItem item(rawItem);
      EQMSG("size", uint32_t(24), item.size());
      EQMSG("type", RING_FORMAT, item.type());
      EQMSG("evt tstamp", uint64_t(234), item.getEventTimestamp());
      EQMSG("source id", uint32_t(23457), item.getSourceId());
      EQMSG("major", uint16_t(12), item.getMajor());
      EQMSG("minor", uint16_t(234), item.getMinor());
  }


  void setType_0() {
      CDataFormatItem item;
      CPPUNIT_ASSERT_NO_THROW_MESSAGE("set type to RING_FORMAT is fine",
                                      item.setType(RING_FORMAT));
  }

  void setType_1() {
      CDataFormatItem item;
      CPPUNIT_ASSERT_THROW_MESSAGE("set type to non-RING_FORMAT is bad!",
                                   item.setType(VOID),
                                   std::invalid_argument);
  }

  void comparison_0() {
      CDataFormatItem item;
      CPPUNIT_ASSERT_MESSAGE("identity comparison", item == item);
  }

  void comparison_1() {
      CDataFormatItem item1(12, 23, 34, 45);
      CDataFormatItem item2(22, 23, 34, 45);

      CPPUNIT_ASSERT_MESSAGE("different evt timestamp", item1 != item2);
  }

  void comparison_2() {
      CDataFormatItem item1(12, 23, 34, 45);
      CDataFormatItem item2(12, 33, 34, 45);

      CPPUNIT_ASSERT_MESSAGE("different source id", item1 != item2);
  }

  void comparison_3() {
      CDataFormatItem item1(12, 23, 34, 45);
      CDataFormatItem item2(12, 23, 44, 45);

      CPPUNIT_ASSERT_MESSAGE("different major version", item1 != item2);
  }

  void comparison_4() {
      CDataFormatItem item1(12, 23, 34, 45);
      CDataFormatItem item2(12, 23, 34, 55);

      CPPUNIT_ASSERT_MESSAGE("different minor version", item1 != item2);
  }


  void toRawRingItem_0() {
      CRawRingItem rawItem;

      CDataFormatItem item(23, 45, 67, 89);
      item.toRawRingItem(rawItem);

      EQMSG("size", uint32_t(24), rawItem.size());
      EQMSG("type", RING_FORMAT, rawItem.type());
      EQMSG("evt tstamp", uint64_t(23), rawItem.getEventTimestamp());
      EQMSG("source id", uint32_t(45), rawItem.getSourceId());
      EQMSG("major", uint16_t(67), item.getMajor());
      EQMSG("minor", uint16_t(89), item.getMinor());
  }


  void toRawRingItem_1() {
      // fill the body of the raw ring item with data prior to calling toRawRingItem
      // it should still succeed
      CRawRingItem rawItem;
      rawItem.getBody() << 23 << 23;

      CDataFormatItem item(23, 45, 67, 89);
      item.toRawRingItem(rawItem);

      EQMSG("size", uint32_t(24), rawItem.size());
      EQMSG("type", RING_FORMAT, rawItem.type());
      EQMSG("evt tstamp", uint64_t(23), rawItem.getEventTimestamp());
      EQMSG("source id", uint32_t(45), rawItem.getSourceId());
      EQMSG("major", uint16_t(67), item.getMajor());
      EQMSG("minor", uint16_t(89), item.getMinor());
  }


  void toString_0() {
      CDataFormatItem item;
      std::stringstream asString;
      asString << "Size (bytes) : 24" << std::endl;
      asString << "Type         : Data Format" << std::endl;
      asString << "Timestamp    : NULL_TIMESTAMP" << std::endl;
      asString << "Source Id    : 0" << std::endl;
      asString << "Data Version : 12.0" << std::endl;

      EQMSG("empty compound item", asString.str(), item.toString());
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(CDataFormatItemTests);

// Test parameters if not building:

