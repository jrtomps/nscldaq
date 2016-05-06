// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"


#include <string>


#include <TCLInterpreter.h>
#include <TCLObject.h>
#include "CConfiguration.h"


namespace Globals {
  extern CConfiguration* pConfig;
  extern unsigned        scalerPeriod;
};

class CbdCrateTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(CbdCrateTests);
  CPPUNIT_TEST(create);
  CPPUNIT_TEST(config);
  CPPUNIT_TEST_SUITE_END();


private:

public:
  void setUp() {
    ::Globals::pConfig = new CConfiguration;
  }
  void tearDown() {
    delete ::Globals::pConfig;
    ::Globals::pConfig = 0;
  }
protected:
  void create();
  void config();
};

CPPUNIT_TEST_SUITE_REGISTRATION(CbdCrateTests);

void CbdCrateTests::create() {
  CTCLInterpreter* pInterp = ::Globals::pConfig->getInterpreter();
  pInterp->GlobalEval("CBDCamacCrate create testing");
  std::string configString = pInterp->GlobalEval("CBDCamacCrate cget testing");
  
  EQ(std::string("0"), getConfigVal(*pInterp, "-crate", configString)); 
}

void CbdCrateTests::config() {
  CTCLInterpreter* pInterp = ::Globals::pConfig->getInterpreter();
  pInterp->GlobalEval("CBDCamacCrate create testing -crate 1");
  std::string configString = pInterp->GlobalEval("CBDCamacCrate cget testing");
  
  EQ(std::string("1"), getConfigVal(*pInterp, "-crate", configString));
  
  pInterp->GlobalEval("CBDCamacCrate config testing -crate 2");
  configString = pInterp->GlobalEval("CBDCamacCrate cget testing");
  
  EQ(std::string("2"), getConfigVal(*pInterp, "-crate", configString));
  
}
