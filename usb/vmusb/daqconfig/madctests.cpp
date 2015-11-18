// Template for a test suite.
#include <iostream>
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


class MadcTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(MadcTests);
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

CPPUNIT_TEST_SUITE_REGISTRATION(MadcTests);

void MadcTests::create() {
  CTCLInterpreter* pInterp = ::Globals::pConfig->getInterpreter();
  pInterp->GlobalEval("madc create testing");
  std::string configString = pInterp->GlobalEval("madc cget testing");
  
  EQ(std::string("0"), getConfigVal(*pInterp, "-base", configString)); 

}

void MadcTests::config() {
  CTCLInterpreter* pInterp = ::Globals::pConfig->getInterpreter();
  pInterp->GlobalEval("madc create testing -base 0x12340000");
  pInterp->GlobalEval("madc config testing -id 12");
  std::string configString = pInterp->GlobalEval("madc cget testing");

  EQ(std::string("0x12340000"), getConfigVal(*pInterp, "-base", configString));
  EQ(std::string("12"), getConfigVal(*pInterp, "-id", configString));
}
