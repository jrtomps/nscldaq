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

class LRS2551tests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(LRS2551tests);
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

CPPUNIT_TEST_SUITE_REGISTRATION(LRS2551tests);

void LRS2551tests::create() {
  CTCLInterpreter* pInterp = ::Globals::pConfig->getInterpreter();
  pInterp->GlobalEval("CBDLeCroy2551 create testing");
  std::string configString = pInterp->GlobalEval("CBDLeCroy2551 cget testing");
  
  EQ(std::string("1"), getConfigVal(*pInterp, "-slot", configString)); 
}

void LRS2551tests::config() {
  CTCLInterpreter* pInterp = ::Globals::pConfig->getInterpreter();
  pInterp->GlobalEval("CBDLeCroy2551 create testing -slot 2");
  std::string configString = pInterp->GlobalEval("CBDLeCroy2551 cget testing");
  
  EQ(std::string("2"), getConfigVal(*pInterp, "-slot", configString));
  
  pInterp->GlobalEval("CBDLeCroy2551 config testing -slot 3");
  configString = pInterp->GlobalEval("CBDLeCroy2551 cget testing");
  
  EQ(std::string("3"), getConfigVal(*pInterp, "-slot", configString));
}
