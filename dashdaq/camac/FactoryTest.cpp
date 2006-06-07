// Template for a test suite.

#include <config.h>
#include <CCAMACInterfaceFactory.h>
#include "CFakeInterfaceCreator.h"
#include "CFakeInterface.h"
#include <CCAMACInterface.h>
#include <string>

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"

using namespace std;


class factoryTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(factoryTests);
  CPPUNIT_TEST(creator);
  CPPUNIT_TEST(createFail);
  CPPUNIT_TEST(configure);
  CPPUNIT_TEST_SUITE_END();


private:

public:
  void setUp() {
  }
  void tearDown() {
  }
protected:
  void creator();
  void createFail();
  void configure();
};

CPPUNIT_TEST_SUITE_REGISTRATION(factoryTests);

// working creation.

void factoryTests::creator() {
  CCAMACInterfaceFactory* pFactory = CCAMACInterfaceFactory::getInstance();
  pFactory->addCreator("fake", new CFakeInterfaceCreator);

  CCAMACInterface* pI = pFactory->createInterface(string("fake"), 
						  string("some test stuff"));

  EQMSG("crate count", (size_t)3, pI->maxCrates());
  EQMSG("last crate",  (size_t)0, pI->lastCrate());

  delete pFactory;		// We can do this since we're a test.
  
}

// failing creation.

void factoryTests::createFail() {
  CCAMACInterfaceFactory* pFactory = CCAMACInterfaceFactory::getInstance();
  pFactory->addCreator("fake", new CFakeInterfaceCreator);

  // Mis spelled:
  CCAMACInterface* pI = pFactory->createInterface(string("fak"), 
						  string("some test stuff"));
  EQ((CCAMACInterface*)NULL, pI);

  delete pFactory;
}

// Can configure?

void factoryTests::configure()
{
  CCAMACInterfaceFactory* pFactory = CCAMACInterfaceFactory::getInstance();
  pFactory->addCreator("fake", new CFakeInterfaceCreator);

  CCAMACInterface* pI = pFactory->createInterface(string("fake"), "configuration");
  
  CFakeInterface* pf = dynamic_cast<CFakeInterface*>(pI);

 EQ(string("configuration"), string(pf->getConfiguration()));
 

  delete pFactory;
}
