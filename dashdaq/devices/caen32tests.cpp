// Template for a test suite.


#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include <CAEN32.h>
#include <stdint.h>

using namespace std;

// Change the lines below if the module moves:
static const uint32_t base(0x80000000);
static Warning msg("caen32tests needs a CAEN32module at base address 0x80000000");

class Testname : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(Testname);
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

CPPUNIT_TEST_SUITE_REGISTRATION(Testname);

void Testname::aTest() {
}
