// Template for a test suite.

#include <config.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"

#include <CBadValue.h>

using namespace std;


class badValue : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(badValue);
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

CPPUNIT_TEST_SUITE_REGISTRATION(badValue);

void badValue::aTest() {
  CBadValue e("a b c", "d", "nothing");
  EQMSG("ReasonCode", 0, e.ReasonCode());
  EQMSG("ReasonText", string("Invalid value : d valid values are: a b c while : nothing"),
	string(e.ReasonText()));
}
