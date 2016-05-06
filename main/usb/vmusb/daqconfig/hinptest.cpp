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


class HinpTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(HinpTests);
  CPPUNIT_TEST(create);
  CPPUNIT_TEST(config);
  CPPUNIT_TEST(psdcreate);
  CPPUNIT_TEST(psdconfig);
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
  void psdcreate();
  void psdconfig();
};

CPPUNIT_TEST_SUITE_REGISTRATION(HinpTests);

void HinpTests::create() {
  CTCLInterpreter* pInterp = ::Globals::pConfig->getInterpreter();
  pInterp->GlobalEval("hinp create testing 0x12340000");
  std::string configString = pInterp->GlobalEval("hinp cget testing");

  EQ(std::string("0x12340000"), getConfigVal(*pInterp, "-base", configString));  
}
void HinpTests::config() {
  CTCLInterpreter* pInterp = ::Globals::pConfig->getInterpreter();
  pInterp->GlobalEval("hinp create testing 0x12340000");
  pInterp->GlobalEval("hinp config testing -readsramb yes");
  std::string configString = pInterp->GlobalEval("hinp cget testing");
  
  EQ(std::string("yes"), getConfigVal(*pInterp, "-readsramb", configString));
}


void HinpTests::psdcreate() {
  CTCLInterpreter* pInterp = ::Globals::pConfig->getInterpreter();
  pInterp->GlobalEval("psd create testing 0x12340000");
  std::string configString = pInterp->GlobalEval("psd cget testing");

  EQ(std::string("0x12340000"), getConfigVal(*pInterp, "-base", configString));  
  
}

void HinpTests::psdconfig()
{
  CTCLInterpreter* pInterp = ::Globals::pConfig->getInterpreter();
  pInterp->GlobalEval("psd create testing 0x12340000");
  pInterp->GlobalEval("psd config testing -base 0x43210000");
  std::string configString = pInterp->GlobalEval("psd cget testing");
 
  EQ(std::string("0x43210000"), getConfigVal(*pInterp, "-base", configString));  
}