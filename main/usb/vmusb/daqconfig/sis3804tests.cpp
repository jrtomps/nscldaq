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


class Sis3804Tests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(Sis3804Tests);
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

CPPUNIT_TEST_SUITE_REGISTRATION(Sis3804Tests);

void Sis3804Tests::create() {
  CTCLInterpreter* pInterp = ::Globals::pConfig->getInterpreter();
  pInterp->GlobalEval("sis3804 create testing");
  std::string configString = pInterp->GlobalEval("sis3804 cget testing");

  EQ(std::string("0"), getConfigVal(*pInterp, "-base", configString)); 
}

void Sis3804Tests::config() {
  CTCLInterpreter* pInterp = ::Globals::pConfig->getInterpreter();
  pInterp->GlobalEval("sis3804 create testing -base 0x12340000");
  pInterp->GlobalEval("sis3804 config testing -refpulser on");
  std::string configString = pInterp->GlobalEval("sis3804 cget testing");
  
  EQ(std::string("0x12340000"), getConfigVal(*pInterp, "-base", configString));
  EQ(std::string("on"),         getConfigVal(*pInterp, "-refpulser", configString));
}
