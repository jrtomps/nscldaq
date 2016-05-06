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



class CbdTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(CbdTests);
  CPPUNIT_TEST(create);
  CPPUNIT_TEST(config);
  CPPUNIT_TEST_SUITE_END();


private:

public:
  
  // NOTE: If we ever require the branch to validate modules/crates we need
  //       to modify this to use a CVMUSBHighLevelController with its own
  //       configuration since that's how things are done now.
  
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

CPPUNIT_TEST_SUITE_REGISTRATION(CbdTests);

void CbdTests::create() {
  CTCLInterpreter* pInterp = ::Globals::pConfig->getInterpreter();
  pInterp->GlobalEval("CBDCamacBranch create testing");
  std::string configString = pInterp->GlobalEval("CBDCamacBranch cget testing");
  
  EQ(std::string("0"), getConfigVal(*pInterp, "-branch", configString)); 
}

void CbdTests::config() {
  CTCLInterpreter* pInterp = ::Globals::pConfig->getInterpreter();
  pInterp->GlobalEval("CBDCamacBranch create testing -branch 1");
  std::string configString = pInterp->GlobalEval("CBDCamacBranch cget testing");
  EQ(std::string("1"), getConfigVal(*pInterp, "-branch", configString));
  
  pInterp->GlobalEval("CBDCamacBranch config testing -branch 5");
  configString = pInterp->GlobalEval("CBDCamacBranch cget testing");
  EQ(std::string("5"), getConfigVal(*pInterp, "-branch", configString));
}
