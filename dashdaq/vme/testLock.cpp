// Template for a test suite.

#include <config.h>
#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include "CVMELock.h"
#include "CVMESubsystem.h"
#include <string>

class lockTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(lockTests);
  CPPUNIT_TEST(lockTest);
  CPPUNIT_TEST_SUITE_END();


private:

public:
  void setUp() {
  }
  void tearDown() {
  }
protected:
  void lockTest();
};

CPPUNIT_TEST_SUITE_REGISTRATION(lockTests);

void lockTests::lockTest() {
  try {
    CVMELock  lock;
    ASSERT(CVMESubsystem::isLockHeld());
    throw string("AAAA");		// Should destroy lock.
  }
  catch (string junk) {
  }
  ASSERT(!CVMESubsystem::isLockHeld());
}
