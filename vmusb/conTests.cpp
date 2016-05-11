// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include <CVMUSB.h>
#include <vector>
#include <usb.h>

using namespace std;

static Warning msg(string("conTests requires exactly 1 VM-USB plugged into the system"));

class conTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(conTests);
  CPPUNIT_TEST(enumerate);
  CPPUNIT_TEST(construct);
  CPPUNIT_TEST_SUITE_END();


private:

public:
  void setUp() {
  }
  void tearDown() {
  }
protected:
  void enumerate();
  void construct();
};

CPPUNIT_TEST_SUITE_REGISTRATION(conTests);


// Should find a single usb device.

void conTests::enumerate() {
  vector<struct usb_device*> devlist = CVMUSB::enumerate();
  
  EQMSG("size", (size_t)1, devlist.size());
}

// Should be able to construct on device 0:
// Can only test that we don't toss an exception.
//
void conTests::construct() {
  vector<struct usb_device*> devlist = CVMUSB::enumerate();
  CVMUSB* interface = new CVMUSB(devlist[0]);

  delete interface;
}
