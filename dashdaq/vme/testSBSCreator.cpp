// Template for a test suite.
#include <config.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"

#include <CSBSVMEInterfaceCreator.h>
#include <CVMEInterface.h>

#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif




class createTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(createTest);
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

CPPUNIT_TEST_SUITE_REGISTRATION(createTest);

void createTest::aTest() {
  CSBSVMEInterfaceCreator test;
  CVMEInterface* pInterface= test("", "0");
  ASSERT(pInterface);

  EQ(string("SBSBit3PCIVME"), pInterface->deviceType());
}
