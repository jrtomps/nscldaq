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

class V830Tests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(V830Tests);
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

CPPUNIT_TEST_SUITE_REGISTRATION(V830Tests);

void V830Tests::create() {
  CTCLInterpreter* pInterp = ::Globals::pConfig->getInterpreter();
  pInterp->GlobalEval("v830 create testing 0x12340000");
  std::string configString = pInterp->GlobalEval("v830 cget testing");

  EQ(std::string("0x12340000"), getConfigVal(*pInterp, "-base", configString));  
}

void V830Tests::config() {
  CTCLInterpreter* pInterp = ::Globals::pConfig->getInterpreter();
  pInterp->GlobalEval("v830 create testing 0x12340000");
  pInterp->GlobalEval("v830 config testing -dwelltime 10");
  std::string configString = pInterp->GlobalEval("v830 cget testing");
  
  EQ(std::string("10"), getConfigVal(*pInterp, "-dwelltime", configString));
}