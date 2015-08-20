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

class LRS4434Tests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(LRS4434Tests);
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

CPPUNIT_TEST_SUITE_REGISTRATION(LRS4434Tests);

void LRS4434Tests::create() {
  CTCLInterpreter* pInterp = ::Globals::pConfig->getInterpreter();
  pInterp->GlobalEval("CBDLeCroy4434 create testing");
  std::string configString = pInterp->GlobalEval("CBDLeCroy4434 cget testing");
  
  EQ(std::string("1"), getConfigVal(*pInterp, "-slot", configString)); 
}
void LRS4434Tests::config() {
  CTCLInterpreter* pInterp = ::Globals::pConfig->getInterpreter();
  pInterp->GlobalEval("CBDLeCroy4434 create testing -slot 2");
  pInterp->GlobalEval("CBDLeCroy4434 config testing -incremental true");
  std::string configString = pInterp->GlobalEval("CBDLeCroy4434 cget testing");
  
  EQ(std::string("2"), getConfigVal(*pInterp, "-slot", configString));
  EQ(std::string("true"), getConfigVal(*pInterp, "-incremental", configString));
}
