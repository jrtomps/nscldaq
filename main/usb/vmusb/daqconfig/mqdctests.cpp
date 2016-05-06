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


class MqdcTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(MqdcTests);
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

CPPUNIT_TEST_SUITE_REGISTRATION(MqdcTests);

void MqdcTests::create() {
  CTCLInterpreter* pInterp = ::Globals::pConfig->getInterpreter();
  pInterp->GlobalEval("mqdc create testing");
  std::string configString = pInterp->GlobalEval("mqdc cget testing");
  
  EQ(std::string("0"), getConfigVal(*pInterp, "-base", configString)); 
}

void MqdcTests::config() {
  CTCLInterpreter* pInterp = ::Globals::pConfig->getInterpreter();
  pInterp->GlobalEval("mqdc create testing -base 0x12340000");
  pInterp->GlobalEval("mqdc config testing -timestamp on");
  std::string configString = pInterp->GlobalEval("mqdc cget testing");
  
  EQ(std::string("0x12340000"), getConfigVal(*pInterp, "-base", configString));
  EQ(std::string("on"), getConfigVal(*pInterp, "-timestamp", configString));
}