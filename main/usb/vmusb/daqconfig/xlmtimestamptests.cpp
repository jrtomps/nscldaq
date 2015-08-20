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


class XlmTimestampTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(XlmTimestampTests);
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

CPPUNIT_TEST_SUITE_REGISTRATION(XlmTimestampTests);

void XlmTimestampTests::create() {
  CTCLInterpreter* pInterp = ::Globals::pConfig->getInterpreter();
  pInterp->GlobalEval("XLMTimestamp create testing");
  std::string configString = pInterp->GlobalEval("XLMTimestamp cget testing");
  
  EQ(std::string("0"), getConfigVal(*pInterp, "-base", configString)); 
}
void XlmTimestampTests::config() {
  CTCLInterpreter* pInterp = ::Globals::pConfig->getInterpreter();
  pInterp->GlobalEval("XLMTimestamp create testing -base 0x12340000");
  
  std::string configString = pInterp->GlobalEval("XLMTimestamp cget testing");
  
  EQ(std::string("0x12340000"), getConfigVal(*pInterp, "-base", configString));
  
  pInterp->GlobalEval("XLMTimestamp config testing -base 0x43210000");
   configString = pInterp->GlobalEval("XLMTimestamp cget testing");
  
  EQ(std::string("0x43210000"), getConfigVal(*pInterp, "-base", configString));
}