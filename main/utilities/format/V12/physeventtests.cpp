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
  CPPUNIT_TEST( badcast );
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp() {
  }
  void tearDown() {
  }
protected:
  void ringitemcopy();
  void badcast();
};

CPPUNIT_TEST_SUITE_REGISTRATION(physeventtests);

void physeventtests::ringitemcopy()
{
  V12::CRawRingItem item(V12::PHYSICS_EVENT);

  auto& body = item.getBody();
  for (uint16_t i = 0; i < 10; i++) {
    body << i;
  }

  V12::CPhysicsEventItem phys_item(item);
  ASSERT( item == phys_item );

}

void physeventtests::badcast()
{
  V12::CRawRingItem myitem(V12::PHYSICS_EVENT_COUNT);
  EQ (V12::PHYSICS_EVENT_COUNT, myitem.type());
  
  V12::CPhysicsEventItem newitem;
  CPPUNIT_ASSERT_THROW( newitem = V12::CPhysicsEventItem(myitem), std::bad_cast );
}
