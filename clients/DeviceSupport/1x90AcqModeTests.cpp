// Template for a test suite.

#include <config.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include "CCAENV1x90.h"
#include "DesignByContract.h"

#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif

using namespace DesignByContract;

extern unsigned long ModuleBase;


class AcqModeTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(AcqModeTests);
  CPPUNIT_TEST(TriggerMatchTest);
  CPPUNIT_TEST(ContStoreTest);
  CPPUNIT_TEST(LoadDefaults);
  CPPUNIT_TEST(SaveLoadUserConfig);
  CPPUNIT_TEST(AutoLoadTest);
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
  void TriggerMatchTest();
  void ContStoreTest();
  void LoadDefaults();
  void SaveLoadUserConfig();
  void AutoLoadTest();
};

CPPUNIT_TEST_SUITE_REGISTRATION(AcqModeTests);

// Can module be put in trigger match mode?

void AcqModeTests::TriggerMatchTest() {
  m_pModule->TriggerMatchMode(); // Put into trigger match mode.
  //  sleep(1);

  ASSERT(m_pModule->isTriggerMatching());
}
// Can module be put in continuous storage mode.
// Flip flop since power up is cont storage mode.

void
AcqModeTests::ContStoreTest()
{
  m_pModule->TriggerMatchMode();
  m_pModule->ContinuousStorageMode();

  ASSERT(!m_pModule->isTriggerMatching());
}

// Load defaults: 

void
AcqModeTests::LoadDefaults()
{
  m_pModule->TriggerMatchMode();

  m_pModule->LoadDefaultConfig(); // -> Cont storage.

  ASSERT(!m_pModule->isTriggerMatching());

}

// Check save/load user config.

void
AcqModeTests::SaveLoadUserConfig()
{
  m_pModule->TriggerMatchMode();
  m_pModule->SaveUserConfig();

  m_pModule->Reset();
  ASSERT(!m_pModule->isTriggerMatching());

  m_pModule->LoadUserConfig();
  ASSERT(m_pModule->isTriggerMatching());
}
//  Check auto load functions:
void
AcqModeTests::AutoLoadTest()
{
  m_pModule->TriggerMatchMode();
  m_pModule->SaveUserConfig();
  m_pModule->AutoLoadUserConfig();
  m_pModule->Reset();		// Should load user config.

  ASSERT(m_pModule->isTriggerMatching());

  m_pModule->AutoLoadDefaultConfig();
  m_pModule->Reset();		// Should load default config..
  ASSERT(!m_pModule->isTriggerMatching());
}

