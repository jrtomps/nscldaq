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

class V1495Tests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(V1495Tests);
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

CPPUNIT_TEST_SUITE_REGISTRATION(V1495Tests);

void V1495Tests::create() {
  CTCLInterpreter* pInterp = ::Globals::pConfig->getInterpreter();
  pInterp->GlobalEval("v1495sc create testing 0x12340000");
  std::string configString = pInterp->GlobalEval("v1495sc cget testing");

  EQ(std::string("0x12340000"), getConfigVal(*pInterp, "-base", configString));  

}
void V1495Tests::config() {
  CTCLInterpreter* pInterp = ::Globals::pConfig->getInterpreter();
  pInterp->GlobalEval("v1495sc create testing 0x12340000");
  pInterp->GlobalEval("v1495sc config testing -g1_mode reset");
  std::string configString = pInterp->GlobalEval("v1495sc cget testing");

  EQ(std::string("reset"), getConfigVal(*pInterp, "-g1_mode", configString));  
  
}
