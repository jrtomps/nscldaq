// Template for a test suite.
#include <config.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include <CNoSuchDevice.h>
using namespace std;



class nosuchDev : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(nosuchDev);
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

CPPUNIT_TEST_SUITE_REGISTRATION(nosuchDev);

void nosuchDev::aTest() {
  CNoSuchDevice e("george", 3, "nothing");

  EQMSG("code", 3, e.ReasonCode());
  EQMSG("message", string("No such george device 3 while: nothing"),
	string(e.ReasonText()));
}
