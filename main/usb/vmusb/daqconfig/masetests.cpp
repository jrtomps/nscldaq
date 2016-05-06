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

class MaseTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(MaseTests);
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

CPPUNIT_TEST_SUITE_REGISTRATION(MaseTests);

void MaseTests::create() {
  CTCLInterpreter* pInterp = ::Globals::pConfig->getInterpreter();
  pInterp->GlobalEval("mase create testing");
  std::string configString = pInterp->GlobalEval("mase cget testing");
  
  EQ(std::string("0"), getConfigVal(*pInterp, "-base", configString)); 
}

void MaseTests::config() {
  CTCLInterpreter* pInterp = ::Globals::pConfig->getInterpreter();
  pInterp->GlobalEval("mase create testing -base 0x12340000");
  std::string configString = pInterp->GlobalEval("mase cget testing");
  
  EQ(std::string("0x12340000"), getConfigVal(*pInterp, "-base", configString));
  
  pInterp->GlobalEval("mase config testing -base 0x43210000");
  configString = pInterp->GlobalEval("mase cget testing");
  EQ(std::string("0x43210000"), getConfigVal(*pInterp, "-base", configString));
}
