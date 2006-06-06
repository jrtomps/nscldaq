// Template for a test suite.

#include <config.h>
#include "CFakeInterface.h"
#include "CFakeInterfaceCreator.h"
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"

#include <string>
using namespace std;

class TestFake : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(TestFake);
  CPPUNIT_TEST(maxcrates);
  CPPUNIT_TEST(lastcrate);
  CPPUNIT_TEST(creator);
  CPPUNIT_TEST_SUITE_END();


private:

public:
  void setUp() {
  }
  void tearDown() {
  }
protected:
  void maxcrates();
  void lastcrate();
  void creator();
};

CPPUNIT_TEST_SUITE_REGISTRATION(TestFake);

void TestFake::maxcrates() {
  CFakeInterface interface;
  EQ((size_t)3, interface.maxCrates());
}


void TestFake::lastcrate()
{
  CFakeInterface interface;

  EQ((size_t)0, interface.lastCrate());
}

void TestFake::creator()
{
  CFakeInterfaceCreator c;
  CFakeInterface* i = static_cast<CFakeInterface*>(c(string("junk")));
  EQ((size_t)3, i->maxCrates());
  delete i;
}
