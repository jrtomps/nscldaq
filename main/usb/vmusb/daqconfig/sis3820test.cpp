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

class Sis3820Tests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(Sis3820Tests);
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

CPPUNIT_TEST_SUITE_REGISTRATION(Sis3820Tests);

void Sis3820Tests::create() {
  CTCLInterpreter* pInterp = ::Globals::pConfig->getInterpreter();
  pInterp->GlobalEval("sis3820 create testing 0x12340000");
  std::string configString = pInterp->GlobalEval("sis3820 cget testing");

  EQ(std::string("0x12340000"), getConfigVal(*pInterp, "-base", configString));  
}
void Sis3820Tests::config() {
CTCLInterpreter* pInterp = ::Globals::pConfig->getInterpreter();
  pInterp->GlobalEval("sis3820 create testing 0x12340000");
  pInterp->GlobalEval("sis3820 config testing -timestamp on");
  std::string configString = pInterp->GlobalEval("sis3820 cget testing");
  
  EQ(std::string("on"), getConfigVal(*pInterp, "-timestamp", configString));
}
