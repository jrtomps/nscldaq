// Template for a test suite.
#include <config.h>
#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include <string>

#include "CDeviceIncapable.h"

class testIncapable : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(testIncapable);
  CPPUNIT_TEST(both);
  CPPUNIT_TEST(attemptOnly);
  CPPUNIT_TEST_SUITE_END();


private:

public:
  void setUp() {
  }
  void tearDown() {
  }
protected:
  void both();
  void attemptOnly();
};

CPPUNIT_TEST_SUITE_REGISTRATION(testIncapable);

// Both device request and capabilities.

void testIncapable::both() {
  CDeviceIncapable test("test", 
			"was testing", "lou, harry, george, not test");
  EQ(string("Device not capable of attempted operation 'test' Device capabilities: lou, harry, george, not test"),
     string(test.ReasonText()));
}

void testIncapable::attemptOnly()
{
  CDeviceIncapable test(string("test"), "was testing" );
  EQ(string("Device not capable of attempted operation 'test'"),
     string(test.ReasonText()));
}
