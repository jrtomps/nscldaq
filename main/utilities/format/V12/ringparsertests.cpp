
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
#include "V12/CRingItemParser.h"
#include "Deserializer.h"
#include "ByteBuffer.h"

#include <iostream>
#include <sstream>

// Tests for glom parameter ring item class:

using namespace DAQ;
using namespace DAQ::V12;

class CRingItemParserTests : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CRingItemParserTests);
    CPPUNIT_TEST(parse_0);
    CPPUNIT_TEST(parse_1);
    CPPUNIT_TEST(parse_2);
    CPPUNIT_TEST(parse_3);
    CPPUNIT_TEST(parse_4);
    CPPUNIT_TEST(parse_5);
    CPPUNIT_TEST(parseSwapped_0);
    CPPUNIT_TEST(peekHeader_0);
    CPPUNIT_TEST_SUITE_END();
private:

public:
  void setUp() {
  }
  void tearDown() {
  }
protected:

  void parse_0() {
    Buffer::ByteBuffer body;
    body << uint32_t(41) << COMP_PHYSICS_EVENT << uint64_t(12) << uint32_t(23);
    body << uint32_t(21) << PHYSICS_EVENT << uint64_t(12) << uint32_t(23);
    body << uint8_t(1);

    CRingItemPtr pItem;
    Buffer::ByteBuffer::iterator ptr;
    std::tie(pItem, ptr) = Parser::parse(body.begin(), body.end());
    EQMSG("size", uint32_t(41), pItem->size());
    EQMSG("type", COMP_PHYSICS_EVENT, pItem->type());
    EQMSG("tstamp", uint64_t(12), pItem->getEventTimestamp());
    EQMSG("source id", uint32_t(23), pItem->getSourceId());

    CCompositeRingItem& Composite0 = dynamic_cast<CCompositeRingItem&>(*pItem);

    EQMSG("n children", size_t(1), Composite0.getChildren().size());
    EQMSG("child size", uint32_t(21), Composite0.getChildren()[0]->size());
    EQMSG("child type", PHYSICS_EVENT, Composite0.getChildren()[0]->type());
    EQMSG("child evt tstamp", uint64_t(12), Composite0.getChildren()[0]->getEventTimestamp());
    EQMSG("child source id", uint32_t(23), Composite0.getChildren()[0]->getSourceId());

    auto& physEvent = dynamic_cast<CPhysicsEventItem&>(*Composite0.getChildren()[0]);
    CPPUNIT_ASSERT_MESSAGE("child body", Buffer::ByteBuffer({1}) == physEvent.getBody());
  }

  void parse_1() {
      Buffer::ByteBuffer body;
      body << uint32_t(83) << COMP_PHYSICS_EVENT << uint64_t(12) << uint32_t(23);
      body << uint32_t(41) << COMP_PHYSICS_EVENT << uint64_t(12) << uint32_t(23);
      body << uint32_t(21) << PHYSICS_EVENT << uint64_t(12) << uint32_t(23);
      body << uint8_t(1);
      body << uint32_t(22) << PHYSICS_EVENT << uint64_t(21) << uint32_t(32);
      body << uint8_t(1) << uint8_t(2);

      CRingItemPtr pItem;
      Buffer::ByteBuffer::iterator ptr;
      std::tie(pItem, ptr) = Parser::parse(body.begin(), body.end());
      EQMSG("size", uint32_t(83), pItem->size());
      EQMSG("type", COMP_PHYSICS_EVENT, pItem->type());
      EQMSG("tstamp", uint64_t(12), pItem->getEventTimestamp());
      EQMSG("source id", uint32_t(23), pItem->getSourceId());

      CCompositeRingItem& Composite0 = dynamic_cast<CCompositeRingItem&>(*pItem);

      EQMSG("n children", size_t(2), Composite0.getChildren().size());

      CPPUNIT_ASSERT_NO_THROW(dynamic_cast<CCompositeRingItem&>(*Composite0.getChildren()[0]));
      CPPUNIT_ASSERT_NO_THROW(dynamic_cast<CPhysicsEventItem&>(*Composite0.getChildren()[1]));

      auto& Composite = dynamic_cast<CCompositeRingItem&>(*Composite0.getChildren()[0]);
      EQMSG("nchildren of subitem", size_t(1), Composite.getChildren().size());
      CPPUNIT_ASSERT_NO_THROW(dynamic_cast<CPhysicsEventItem&>(*Composite.getChildren()[0]));

      pItem = Composite.getChildren()[0];
      EQMSG("size of child", uint32_t(21), pItem->size());
  }

  void parse_2() {
    Buffer::ByteBuffer body;
    body << uint32_t(44) << COMP_PHYSICS_EVENT << uint64_t(12) << uint32_t(23);
    body << uint32_t(24) << RING_FORMAT << uint64_t(12) << uint32_t(23);
    body << uint16_t(1) << uint16_t(2);

    CPPUNIT_ASSERT_THROW_MESSAGE( "inconsistent types should fail",
                                  Parser::parse(body.begin(), body.end()),
                                  std::runtime_error);
  }


  void parse_3() {
    Buffer::ByteBuffer body;
    body << uint32_t(82) << COMP_PHYSICS_EVENT << uint64_t(12) << uint32_t(23);
    body << uint32_t(41) << COMP_PHYSICS_EVENT_COUNT << uint64_t(12) << uint32_t(23);
    body << uint32_t(21) << PHYSICS_EVENT << uint64_t(12) << uint32_t(23);
    body << uint8_t(1);

    CPPUNIT_ASSERT_THROW_MESSAGE( "inconsistent types should fail",
                                  Parser::parse(body.begin(), body.end()),
                                  std::runtime_error);
  }


  void parse_4() {
      Buffer::ByteBuffer body;
      body << uint32_t(82) << COMP_PHYSICS_EVENT << uint64_t(12) << uint32_t(23);
      body << uint32_t(41) << COMP_PHYSICS_EVENT_COUNT << uint64_t(12) << uint32_t(23);

      CPPUNIT_ASSERT_THROW_MESSAGE("throw on insufficient data",
                                   Parser::parse(body.begin(), body.end()),
                                   std::runtime_error);

  }

  void parse_5() {
      Buffer::ByteBuffer body; body << 1;
      CPPUNIT_ASSERT_THROW_MESSAGE("throw on insufficient data for header",
                                   Parser::parse(body.begin(), body.end()),
                                   std::runtime_error);
  }

  void parseSwapped_0() {

      // This is the exact same data buffer as in the parse_0 test except that it has
      // a different endianness.
      Buffer::ByteBuffer body;
      body << uint32_t(0x53000000) << uint32_t(0x1e800000) << uint64_t(0x0c00000000000000)
           << uint32_t(0x17000000);
      body << uint32_t(0x29000000) << uint32_t(0x1e800000) << uint64_t(0x0c00000000000000)
           << uint32_t(0x17000000);
      body << uint32_t(0x15000000) << uint32_t(0x1e000000) << uint64_t(0x0c00000000000000)
           << uint32_t(0x17000000);
      body << uint8_t(0x1);
      body << uint32_t(0x16000000) << uint32_t(0x1e000000) << uint64_t(0x1500000000000000)
           << uint32_t(0x20000000);
      body << uint8_t(1) << uint8_t(2);

      CRingItemPtr pItem;
      Buffer::ByteBuffer::iterator ptr;
      std::tie(pItem, ptr) = Parser::parse(body.begin(), body.end());
      EQMSG("size", uint32_t(83), pItem->size());
      EQMSG("type", COMP_PHYSICS_EVENT, pItem->type());
      EQMSG("tstamp", uint64_t(12), pItem->getEventTimestamp());
      EQMSG("source id", uint32_t(23), pItem->getSourceId());

      CCompositeRingItem& Composite0 = dynamic_cast<CCompositeRingItem&>(*pItem);

      EQMSG("n children", size_t(2), Composite0.getChildren().size());

      CPPUNIT_ASSERT_NO_THROW(dynamic_cast<CCompositeRingItem&>(*Composite0.getChildren()[0]));
      CPPUNIT_ASSERT_NO_THROW(dynamic_cast<CPhysicsEventItem&>(*Composite0.getChildren()[1]));

      auto& Composite = dynamic_cast<CCompositeRingItem&>(*Composite0.getChildren()[0]);
      EQMSG("nchildren of subitem", size_t(1), Composite.getChildren().size());
      CPPUNIT_ASSERT_NO_THROW(dynamic_cast<CPhysicsEventItem&>(*Composite.getChildren()[0]));

      pItem = Composite.getChildren()[0];
      EQMSG("size of child", uint32_t(21), pItem->size());
  }

  void peekHeader_0() {
      Buffer::ByteBuffer body;
      body << uint32_t(20) << PHYSICS_EVENT;

      uint32_t size, type;
      bool swapNeeded;
      Parser::peekHeaderType(body.begin(), body.end(), size, type, swapNeeded);

      EQMSG("size", uint32_t(20), size);
      EQMSG("type", PHYSICS_EVENT, type);
      EQMSG("swap", false, swapNeeded);
  }

};


CPPUNIT_TEST_SUITE_REGISTRATION(CRingItemParserTests);

// Test parameters if not building:

