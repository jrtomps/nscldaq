// Template for a test suite.
#include <config.h>
using namespace std;

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include <CInvalidInterfaceType.h>


class interfaceType : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(interfaceType);
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

CPPUNIT_TEST_SUITE_REGISTRATION(interfaceType);

void interfaceType::aTest() {
  CInvalidInterfaceType e(string("type invalid"), string("doing test"));
  string type  = e.ReasonText();
  string doing = e.WasDoing();

  EQ(string("type invalid"), type);
  EQ(string("doing test"), doing);

}
