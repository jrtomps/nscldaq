

// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include <string>

#include "Asserts.h"
#include "V12/DataFormat.h"
#include "V12/format_cast.h"
#include "V12/CRawRingItem.h"
#include "V12/CDataFormatItem.h"
#include "ByteBuffer.h"
#include "Deserializer.h"

#include <iostream>
#include <sstream>

// Tests for glom parameter ring item class:

using namespace DAQ;
using namespace DAQ::V12;

class format_castTests : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(format_castTests);
    CPPUNIT_TEST(toRawRingItem_0);
    CPPUNIT_TEST(fromRawRingItem_0);
    CPPUNIT_TEST_SUITE_END();

private:

public:
  void setUp() {
  }
  void tearDown() {
  }
protected:

  void toRawRingItem_0() {

      V12::CDataFormatItem item;

      auto rawItem = V12::format_cast<V12::CRawRingItem>(item);

      uint16_t major, minor;
      Buffer::ContainerDeserializer<Buffer::ByteBuffer> stream(rawItem.getBody());
      stream >> major >> minor;

      EQMSG("size", uint32_t(24), rawItem.size());
      EQMSG("type", RING_FORMAT, rawItem.type());
      EQMSG("evt tstamp", NULL_TIMESTAMP, rawItem.getEventTimestamp());
      EQMSG("source id", uint32_t(0), rawItem.getSourceId());

      EQMSG("major", uint16_t(12), major);
      EQMSG("minor", uint16_t(0), minor);
  }

  void fromRawRingItem_0()
  {
      V12::CRawRingItem rawItem;
      rawItem.getBody() << uint16_t(12) << uint16_t(0);

      auto item = V12::format_cast<V12::CDataFormatItem>(rawItem);

      EQMSG("size", uint32_t(24), item.size());
      EQMSG("type", RING_FORMAT, item.type());
      EQMSG("evt tstamp", NULL_TIMESTAMP, item.getEventTimestamp());
      EQMSG("source id", uint32_t(0), item.getSourceId());

      EQMSG("major", uint16_t(12), item.getMajor());
      EQMSG("minor", uint16_t(0), item.getMinor());
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(format_castTests);

// Test parameters if not building:

