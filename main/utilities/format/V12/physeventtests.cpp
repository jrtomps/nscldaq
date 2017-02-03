// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"

#include <V12/DataFormat.h>
#include <V12/CPhysicsEventItem.h>
#include <V12/CRawRingItem.h>

#include <string.h>
#include <time.h>

using namespace DAQ;

class physeventtests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(physeventtests);
  CPPUNIT_TEST( ringitemcopy );
  CPPUNIT_TEST( ringitemassign_0 );
  CPPUNIT_TEST( badcast );
  CPPUNIT_TEST( setType_0);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp() {
  }
  void tearDown() {
  }
protected:
  void ringitemcopy();
  void badcast();
  void comparison_0();
  void comparison_1();
  void ringitemassign_0();
  void setType_0();
};

CPPUNIT_TEST_SUITE_REGISTRATION(physeventtests);

void physeventtests::ringitemcopy()
{
  V12::CRawRingItem item;
  item.setType(V12::PHYSICS_EVENT);

  auto& body = item.getBody();
  for (uint16_t i = 0; i < 10; i++) {
    body << i;
  }

  V12::CPhysicsEventItem phys_item(item);
  ASSERT( item == phys_item );

}

void physeventtests::badcast()
{
  V12::CRawRingItem myitem;
  EQ (V12::VOID, myitem.type());
  
  V12::CPhysicsEventItem newitem;
  CPPUNIT_ASSERT_THROW( newitem = V12::CPhysicsEventItem(myitem), std::bad_cast );
}


void physeventtests::comparison_0()
{
    V12::CPhysicsEventItem newItm;
    CPPUNIT_ASSERT_MESSAGE("identity comparison", newItm == newItm);
}

void physeventtests::comparison_1()
{
    V12::CPhysicsEventItem newItem;

    V12::CPhysicsEventItem phys_item;

    auto& body = phys_item.getBody();
    for (uint16_t i = 0; i < 10; i++) {
      body << i;
    }

    CPPUNIT_ASSERT_MESSAGE("compare different", newItem != phys_item);

}


void physeventtests::ringitemassign_0()
{
    V12::CPhysicsEventItem item(0x1234, 0x45, {1,2,3,4,5,6});

    V12::CPhysicsEventItem item2;

    item2 = item;
    CPPUNIT_ASSERT_MESSAGE("assigment creates same", item == item2 );

}

void physeventtests::setType_0() {
    V12::CPhysicsEventItem item;
    CPPUNIT_ASSERT_THROW_MESSAGE("physics event type must be PHYSICS_EVENT",
                                 item.setType(V12::VOID),
                                 std::invalid_argument);
}
