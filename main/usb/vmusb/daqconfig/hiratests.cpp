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

class HiraTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(HiraTests);
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

CPPUNIT_TEST_SUITE_REGISTRATION(HiraTests);

void HiraTests::create() {

  CTCLInterpreter* pInterp = ::Globals::pConfig->getInterpreter();
  pInterp->GlobalEval("hira create testing");
  std::string configString = pInterp->GlobalEval("hira cget testing");
  int id = atoi(getConfigVal(*pInterp, "-id", configString).c_str());
  EQ(0x618a, id);  
}
void HiraTests::config()
{
  CTCLInterpreter* pInterp = ::Globals::pConfig->getInterpreter();
  pInterp->GlobalEval("hira create testing");
  pInterp->GlobalEval("hira config testing -id 1234");
  std::string configString = pInterp->GlobalEval("hira cget testing");

  EQ(std::string("1234"), getConfigVal(*pInterp, "-id", configString));  
  
}
