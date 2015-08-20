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


class MarkerTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(MarkerTests);
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

CPPUNIT_TEST_SUITE_REGISTRATION(MarkerTests);

void MarkerTests::create()
{

  CTCLInterpreter* pInterp = ::Globals::pConfig->getInterpreter();
  pInterp->GlobalEval("marker create testing 0x1234");
  std::string configString = pInterp->GlobalEval("marker cget testing");
  
  EQ(std::string("0x1234"), getConfigVal(*pInterp, "-value", configString) );
}

void MarkerTests::config()
{
  CTCLInterpreter* pInterp = ::Globals::pConfig->getInterpreter();
  pInterp->GlobalEval("marker create testing 0x1234");
  pInterp->GlobalEval("marker config testing -value 0x4321");
  std::string configString = pInterp->GlobalEval("marker cget testing");
  
  EQ(std::string("0x4321"), getConfigVal(*pInterp, "-value", configString) );
  
}
