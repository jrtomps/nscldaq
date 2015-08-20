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

class UlmTriggerTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(UlmTriggerTests);
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

CPPUNIT_TEST_SUITE_REGISTRATION(UlmTriggerTests);

void UlmTriggerTests::create() {
  CTCLInterpreter* pInterp = ::Globals::pConfig->getInterpreter();
  pInterp->GlobalEval("CBDULMTrigger create testing");
  std::string configString = pInterp->GlobalEval("CBDULMTrigger cget testing");
  
  EQ(std::string("1"), getConfigVal(*pInterp, "-slot", configString)); 
}

void UlmTriggerTests::config() {
  CTCLInterpreter* pInterp = ::Globals::pConfig->getInterpreter();
  pInterp->GlobalEval("CBDULMTrigger create testing -slot 3");
  pInterp->GlobalEval("CBDULMTrigger config testing -pcDelay 123");
  std::string configString = pInterp->GlobalEval("CBDULMTrigger cget testing");
  
  EQ(std::string("3"), getConfigVal(*pInterp, "-slot", configString));
  EQ(std::string("123"), getConfigVal(*pInterp, "-pcDelay", configString));
}
