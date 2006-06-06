// Template for a test suite.

#include <config.h>
#include "CFakeInterface.h"
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"



class TestFake : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(TestFake);
  CPPUNIT_TEST(maxcrates);
  CPPUNIT_TEST(lastcrate);
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
