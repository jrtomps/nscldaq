// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include <CVMUSB.h>
#include <vector>
#include <usb.h>

using namespace std;

Warning(string("conTests requires exactly 1 VM-USB plugged into the system"));

class conTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(conTests);
  CPPUNIT_TEST(enumerate);
  CPPUNIT_TEST_SUITE_END();


private:

public:
  void setUp() {
  }
  void tearDown() {
  }
protected:
  void enumerate();
};

CPPUNIT_TEST_SUITE_REGISTRATION(conTests);


// Should find a single usb device.

void conTests::enumerate() {
  vector<struct usb_device*> devlist = CVMUSB::enumerate();
  
  EQMSG("size", (size_t)1, devlist.size());
}
