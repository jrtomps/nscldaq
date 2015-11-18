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

class Tdc1x90Tests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(Tdc1x90Tests);
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

CPPUNIT_TEST_SUITE_REGISTRATION(Tdc1x90Tests);

void Tdc1x90Tests::create() {
  CTCLInterpreter* pInterp = ::Globals::pConfig->getInterpreter();
  pInterp->GlobalEval("tdc1x90 create testing");
  std::string configString = pInterp->GlobalEval("tdc1x90 cget testing");
  
  EQ(std::string("0"), getConfigVal(*pInterp, "-base", configString)); 
}

void Tdc1x90Tests::config() {
  CTCLInterpreter* pInterp = ::Globals::pConfig->getInterpreter();
  pInterp->GlobalEval("tdc1x90 create testing -base 0x12340000");
  pInterp->GlobalEval("tdc1x90 config testing -termination switch");
  std::string configString = pInterp->GlobalEval("tdc1x90 cget testing");
  
  EQ(std::string("0x12340000"), getConfigVal(*pInterp, "-base", configString));
  EQ(std::string("switch"), getConfigVal(*pInterp, "-termination", configString));
}
