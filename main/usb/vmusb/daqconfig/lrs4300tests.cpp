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


class LRS4300Tests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(LRS4300Tests);
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

CPPUNIT_TEST_SUITE_REGISTRATION(LRS4300Tests);

void LRS4300Tests::create() {
  CTCLInterpreter* pInterp = ::Globals::pConfig->getInterpreter();
  pInterp->GlobalEval("CBDLeCroy4300B create testing");
  std::string configString = pInterp->GlobalEval("CBDLeCroy4300B cget testing");
  
  EQ(std::string("1"), getConfigVal(*pInterp, "-slot", configString)); 
}
void LRS4300Tests::config() {
  CTCLInterpreter* pInterp = ::Globals::pConfig->getInterpreter();
  pInterp->GlobalEval("CBDLeCroy4300B create testing -slot 2");
  pInterp->GlobalEval("CBDLeCroy4300B config testing -vsn 2");
  std::string configString = pInterp->GlobalEval("CBDLeCroy4300B cget testing");
  
  EQ(std::string("2"), getConfigVal(*pInterp, "-slot", configString));
  EQ(std::string("2"), getConfigVal(*pInterp, "-vsn", configString));   
}
