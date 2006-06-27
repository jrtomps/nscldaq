// Template for a test suite.
#include <config.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include <CDuplicateDevice.h>
using namespace std;

class dupDevTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(dupDevTest);
  CPPUNIT_TEST(aTest);
  CPPUNIT_TEST_SUITE_END();


private:

public:
  void setUp() {
  }
  void tearDown() {
  }
protected:
  void aTest();
};

CPPUNIT_TEST_SUITE_REGISTRATION(dupDevTest);

void dupDevTest::aTest() {
  CDuplicateDevice e("george", "nothing");

  EQMSG("code", 0, e.ReasonCode());
  EQMSG("message", string("Attempt to install an existing device: george while : nothing"),
	string(e.ReasonText()));
}
