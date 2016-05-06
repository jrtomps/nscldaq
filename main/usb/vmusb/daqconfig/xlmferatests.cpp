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



class XlmFeraTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(XlmFeraTests);
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

CPPUNIT_TEST_SUITE_REGISTRATION(XlmFeraTests);

void XlmFeraTests::create() {
  CTCLInterpreter* pInterp = ::Globals::pConfig->getInterpreter();
  pInterp->GlobalEval("XLMFERA create testing");
  std::string configString = pInterp->GlobalEval("XLMFERA cget testing");
  
  EQ(std::string("0"), getConfigVal(*pInterp, "-base", configString)); 
}
void XlmFeraTests::config() {
  CTCLInterpreter* pInterp = ::Globals::pConfig->getInterpreter();
  pInterp->GlobalEval("XLMFERA create testing -base 0x12340000");
  pInterp->GlobalEval("XLMFERA config testing -configurationID 666");
  std::string configString = pInterp->GlobalEval("XLMFERA cget testing");
  
  EQ(std::string("0x12340000"), getConfigVal(*pInterp, "-base", configString));
  EQ(std::string("666"), getConfigVal(*pInterp, "-configurationID", configString));
}
