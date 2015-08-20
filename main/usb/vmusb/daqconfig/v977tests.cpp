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

class V977Tests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(V977Tests);
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

CPPUNIT_TEST_SUITE_REGISTRATION(V977Tests);

void V977Tests::create() {
  CTCLInterpreter* pInterp = ::Globals::pConfig->getInterpreter();
  pInterp->GlobalEval("v977 create testing");
  std::string configString = pInterp->GlobalEval("v977 cget testing");
  
  EQ(std::string("0"), getConfigVal(*pInterp, "-base", configString)); 
}

void V977Tests::config() {
  CTCLInterpreter* pInterp = ::Globals::pConfig->getInterpreter();
  pInterp->GlobalEval("v977 create testing -base 0x12340000");
  pInterp->GlobalEval("v977 config testing -readmode multihit");
  std::string configString = pInterp->GlobalEval("v977 cget testing");
  
  EQ(std::string("0x12340000"), getConfigVal(*pInterp, "-base", configString)); 
  EQ(std::string("multihit"), getConfigVal(*pInterp, "-readmode", configString));
}
