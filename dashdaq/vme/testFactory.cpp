// Template for a test suite.

#include <config.h>
#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"

#include "CVMEInterfaceFactory.h"
#include "nullVMEInterfaceCreator.h"
#include "nullVMEInterface.h"
#include "CVMEInterface.h"


class testFactory : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(testFactory);
  CPPUNIT_TEST(registerCreator);
  CPPUNIT_TEST(distinguishCreators);
  CPPUNIT_TEST(create);
  CPPUNIT_TEST_SUITE_END();


private:

public:
  void setUp() {
  }
  void tearDown() {
  }
protected:
  void registerCreator();
  void distinguishCreators();
  void create();
};

CPPUNIT_TEST_SUITE_REGISTRATION(testFactory);

void testFactory::registerCreator() {
  nullVMEInterfaceCreator* c = new nullVMEInterfaceCreator;
  CVMEInterfaceFactory::addCreator(string("null"), *c);

  // Should be able to find the matching creator
  
  CVMEInterfaceCreator* f = CVMEInterfaceFactory::findCreator(string("null"));
  EQMSG("Pointer match", c, static_cast<nullVMEInterfaceCreator*>(f));


  CVMEInterfaceFactory::clearRegistry();
  delete c;
}
// ensure we can tell 2 creators apart:

void testFactory::distinguishCreators()
{
  nullVMEInterfaceCreator c1,c2;
  
  CVMEInterfaceFactory::addCreator(string("null1"), c1);
  CVMEInterfaceFactory::addCreator(string("null2"), c2);

  CVMEInterfaceCreator* f1 = CVMEInterfaceFactory::findCreator(string("null1"));
  CVMEInterfaceCreator* f2 = CVMEInterfaceFactory::findCreator(string("null2"));
  CVMEInterfaceCreator* nu = CVMEInterfaceFactory::findCreator(string("nosuch"));

  EQMSG("c1", static_cast<CVMEInterfaceCreator*>(&c1), f1);
  EQMSG("c2", static_cast<CVMEInterfaceCreator*>(&c2), f2);
  EQMSG("NULL", static_cast<CVMEInterfaceCreator*>(NULL), nu);

  CVMEInterfaceFactory::clearRegistry();
}

void testFactory::create()
{
  nullVMEInterfaceCreator c;
  CVMEInterfaceFactory::addCreator(string("null"), c);

  CVMEInterface* pInterface = 
    CVMEInterfaceFactory::create(string("null some configuration"));

  ASSERT(pInterface);		// Should make one.
  EQMSG("type", string("NULL testing only"), pInterface->deviceType());

  nullVMEInterface* pNull = dynamic_cast<nullVMEInterface*>(pInterface);

  string config = pNull->getConfiguration();
  string type   = pNull->getType();

  EQMSG("ctype", string("null"), type);
  EQMSG("config", string("some configuration"), config);

  delete pNull;

  CVMEInterfaceFactory::clearRegistry();
}
