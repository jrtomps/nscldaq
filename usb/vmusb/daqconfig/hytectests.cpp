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

class HytecTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(HytecTests);
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

CPPUNIT_TEST_SUITE_REGISTRATION(HytecTests);

void HytecTests::create() {
  CTCLInterpreter* pInterp = ::Globals::pConfig->getInterpreter();
  pInterp->GlobalEval("hytec create testing");
  std::string configString = pInterp->GlobalEval("hytec cget testing");

  EQ(std::string("0"), getConfigVal(*pInterp, "-csr", configString)); 
}

void HytecTests::config() {
  CTCLInterpreter* pInterp = ::Globals::pConfig->getInterpreter();
  pInterp->GlobalEval("hytec create testing -csr 0x123400");
  pInterp->GlobalEval("hytec config testing -memory 0x66660000");
  std::string configString = pInterp->GlobalEval("hytec cget testing");

  EQ(std::string("0x123400"), getConfigVal(*pInterp, "-csr", configString)); 
  EQ(std::string("0x66660000"), getConfigVal(*pInterp, "-memory", configString));
}
