// Template for a test suite.

#include <config.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include "CCAENV1x90.h"

#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif


extern long ModuleBase;

class TriggerTimeTagTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(TriggerTimeTagTests);
  CPPUNIT_TEST(Enable);
  CPPUNIT_TEST(Disable);
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
  void Enable();
  void Disable();
};

CPPUNIT_TEST_SUITE_REGISTRATION(TriggerTimeTagTests);


// After enabling 
// The trigger time tag enable bit should be set in the control register.

void TriggerTimeTagTests::Enable() {
  m_pModule->EnableTriggerTagTime();
  unsigned short cr = m_pModule->ReadCR();
  ASSERT((cr & 0x200) != 0);
}


// Should be able to turn off trigger time tag.

void
TriggerTimeTagTests::Disable()
{
  m_pModule->DisableTriggerTagTime();
  unsigned short cr = m_pModule->ReadCR();
  ASSERT((cr & 0x200) == 0);

}
