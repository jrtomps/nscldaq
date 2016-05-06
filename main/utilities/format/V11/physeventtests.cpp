// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"

#include <V11/DataFormatV11.h>
#include <V11/CPhysicsEventItem.h>

#include <string.h>
#include <time.h>

using namespace DAQ::V11;

class physeventtests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(physeventtests);
  CPPUNIT_TEST( ringitemcopy );
  CPPUNIT_TEST( badcast );
  CPPUNIT_TEST_SUITE_END();

private:

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
  CRingItem item(30,8192);
  uint16_t* p = reinterpret_cast<uint16_t*>(item.getBodyCursor());
  for (int i = 0; i < 10; i++) {
    *p++ = i;
  }

  item.setBodyCursor(p);
  item.updateSize();
 
  CPhysicsEventItem phys_item(item);
  ASSERT( item == phys_item );

}

void physeventtests::badcast()
{
  CRingItem myitem(PHYSICS_EVENT_COUNT,8192);
  EQ (PHYSICS_EVENT_COUNT , myitem.type());
  
  CPhysicsEventItem newitem;
  CPPUNIT_ASSERT_THROW( newitem = CPhysicsEventItem(myitem), std::bad_cast );
}
