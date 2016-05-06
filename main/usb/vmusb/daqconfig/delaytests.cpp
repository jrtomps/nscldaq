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



class DelayTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(DelayTests);
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

CPPUNIT_TEST_SUITE_REGISTRATION(DelayTests);

void DelayTests::create() {
  CTCLInterpreter* pInterp = ::Globals::pConfig->getInterpreter();
  pInterp->GlobalEval("delay create testing");
  std::string configString = pInterp->GlobalEval("delay cget testing");
  
  EQ(std::string("0"), getConfigVal(*pInterp, "-value", configString)); 
}
void DelayTests::config() {
  CTCLInterpreter* pInterp = ::Globals::pConfig->getInterpreter();
  
  pInterp->GlobalEval("delay create testing -value 12");
  std::string configString = pInterp->GlobalEval("delay cget testing");
  EQ(std::string("12"), getConfigVal(*pInterp, "-value", configString));
  
  pInterp->GlobalEval("delay config testing -value 34");
  configString = pInterp->GlobalEval("delay cget testing");
  EQ(std::string("34"), getConfigVal(*pInterp, "-value", configString));
}
