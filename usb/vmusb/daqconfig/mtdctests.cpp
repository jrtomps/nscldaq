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

class MtdcTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(MtdcTests);
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

CPPUNIT_TEST_SUITE_REGISTRATION(MtdcTests);

void MtdcTests::create() {
  CTCLInterpreter* pInterp = ::Globals::pConfig->getInterpreter();
  pInterp->GlobalEval("mtdc create testing");
  std::string configString = pInterp->GlobalEval("mtdc cget testing");
  
  EQ(std::string("0"), getConfigVal(*pInterp, "-base", configString)); 

}

void MtdcTests::config() {
  CTCLInterpreter* pInterp = ::Globals::pConfig->getInterpreter();
  pInterp->GlobalEval("mtdc create testing -base 0x12340000");
  pInterp->GlobalEval("mtdc config testing -maxtransfers 42");
  std::string configString = pInterp->GlobalEval("mtdc cget testing");
  
  EQ(std::string("0x12340000"), getConfigVal(*pInterp, "-base", configString)); 
  EQ(std::string("42"), getConfigVal(*pInterp, "-maxtransfers", configString));
}