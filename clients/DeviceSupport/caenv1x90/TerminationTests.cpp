// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include "CCAENV1x90.h"

extern long ModuleBase;

class TerminationTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(TerminationTests);
  CPPUNIT_TEST(Terminate);
  CPPUNIT_TEST(Unterminate);
  CPPUNIT_TEST(SwitchTerminate);
  CPPUNIT_TEST_SUITE_END();


private:
  CCAENV1x90* m_pModule;

public:
  // Construct a module in powered up condition:

  void setUp() {
    m_pModule = new CCAENV1x90(1, 0, ModuleBase);
    m_pModule->Reset();
  }
  // Delete the module to prevent resource leaks.

  void tearDown() {
    delete m_pModule;
  }
protected:
  void Terminate();
  void Unterminate();
  void SwitchTerminate();
};

CPPUNIT_TEST_SUITE_REGISTRATION(TerminationTests);


// After enabling termination, the module should
// be terminated.

void TerminationTests::Terminate() {
  m_pModule->Terminate();
  ASSERT(m_pModule->isTerminated());
}


// Should be able to turn off termination too..

void
TerminationTests::Unterminate()
{
  m_pModule->Unterminate();
  ASSERT(!(m_pModule->isTerminated()));

}
// Should be able to use switch termination and then
// ReadCR will have the sw term bit off.

void
TerminationTests::SwitchTerminate()
{
  m_pModule->TerminateWithSwitch();
  
  unsigned long cr = m_pModule->ReadCR();
  ASSERT((cr & 4) == 0);
}
