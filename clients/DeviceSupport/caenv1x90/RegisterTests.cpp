// Template for a test suite.
// Test register driven accesses:

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include "CCAENV1x90.h"
#include "DesignByContract.h"

extern unsigned long ModuleBase;

using namespace DesignByContract;

// Test a bunch of stuff that is handled via register
// addressing (non micro sequencer stuff).

class Registertests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(Registertests);
  CPPUNIT_TEST(AlmostFullLevel);
  CPPUNIT_TEST(ECLOutputSelect);
  CPPUNIT_TEST_SUITE_END();


private:
  CCAENV1x90* m_pModule;

public:
  void setUp() { 
    m_pModule = new CCAENV1x90(1, 0, ModuleBase);
    m_pModule->Reset();
  
  }
  void tearDown() {
    delete m_pModule;
  }
protected:
  void AlmostFullLevel();
  void ECLOutputSelect();
};

CPPUNIT_TEST_SUITE_REGISTRATION(Registertests);


// Check Set/GetAlmostFullLevel.

void Registertests::AlmostFullLevel() {


  // Check the legal range:

  for(unsigned int i = 1; i <= 32735; i++) {
    m_pModule->SetAlmostFullLevel(i);
    EQ(i, (unsigned int)m_pModule->GetAlmostFullLevel());
  }

  // Check Illegal range.

  EXCEPTION((m_pModule->SetAlmostFullLevel(0)),
	    Require);
  EXCEPTION((m_pModule->SetAlmostFullLevel(32736)),
	    Require);

}
//  Check the Define/Get ECLOtutputDefinition functions:
//
void
Registertests::ECLOutputSelect()
{
  m_pModule->DefineECLOutput(CCAENV1x90::DATA_READY);
  EQMSG("Data Ready",   CCAENV1x90::DATA_READY, 
	m_pModule->GetECLOutputDefinition());

  m_pModule->DefineECLOutput(CCAENV1x90::FULL);
  EQMSG("Full ", CCAENV1x90::FULL,
	m_pModule->GetECLOutputDefinition());

  m_pModule->DefineECLOutput(CCAENV1x90::ALMOST_FULL);
  EQMSG("Almost FUll", CCAENV1x90::ALMOST_FULL,
	m_pModule->GetECLOutputDefinition());

  m_pModule->DefineECLOutput(CCAENV1x90::ERROR);
  EQMSG("Error ", CCAENV1x90::ERROR,
	m_pModule->GetECLOutputDefinition());

}
