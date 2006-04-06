// Template for a test suite.

#include <config.h>
#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"

#include "nullVMEInterfaceCreator.h"
#include "nullVMEInterface.h"


class testCreator : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(testCreator);
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

CPPUNIT_TEST_SUITE_REGISTRATION(testCreator);

// just create an interface:
// - no null return.
// - configuration correct
// - type correct.

void testCreator::aTest() {
  string type("testing");
  string config("some configuration string");
  nullVMEInterfaceCreator creator;
  CVMEInterface* pI  = creator(type, config);

  ASSERT(pI);

  nullVMEInterface* pInterface = dynamic_cast<nullVMEInterface*>(pI);

  EQMSG("type", type, pInterface->getType());
  EQMSG("config", config, pInterface->getConfiguration());

  EQMSG("Type String", string("NULL testing only"), pInterface->deviceType());
	
}
