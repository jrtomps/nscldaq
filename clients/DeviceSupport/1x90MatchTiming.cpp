// Template for a test suite.

#include <config.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include "CCAENV1x90.h"
#include "DesignByContract.h"
#include <Iostream.h>

#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif

using namespace DesignByContract;

extern long ModuleBase;

class MatchTiming : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(MatchTiming);
  CPPUNIT_TEST(MatchWindowTest);
  CPPUNIT_TEST(WindowOffsetTest);
  CPPUNIT_TEST(ExtraSearchTest);
  CPPUNIT_TEST(RejectMarginTest);
  CPPUNIT_TEST(TriggerSubtractTest);
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
  void MatchWindowTest();
  void WindowOffsetTest();
  void ExtraSearchTest();
  void RejectMarginTest();
  void TriggerSubtractTest();
};

CPPUNIT_TEST_SUITE_REGISTRATION(MatchTiming);


// Test that the match window can be set:

void
MatchTiming::MatchWindowTest()
{

  // Test legal values that check all bits.

  for(unsigned int i =1; i <= 0xfff; i = (i << 1) | 1) {
    m_pModule->SetWindowWidth(i);
    CCAENV1x90::TriggerConfiguration current = 
                                    m_pModule->GetTriggerConfiguration();
    EQ(i, m_pModule->GetMatchWindow(current));
  }

  // Now Test  illegal values:

  EXCEPTION((m_pModule->SetWindowWidth(0x1000)), Require);
  EXCEPTION((m_pModule->SetWindowWidth(0)),      Require);

}

// Test the window offset can be set.

void MatchTiming::WindowOffsetTest()
{
  for(int i = -2048; i < 41; i += 83) {
    m_pModule->SetWindowOffset(i);
    CCAENV1x90::TriggerConfiguration current = 
      m_pModule->GetTriggerConfiguration();
    EQ(i, m_pModule->GetWindowOffset(current));
  }

  // Test illegal values:

  EXCEPTION((m_pModule->SetWindowOffset(-2049)), Require);
  EXCEPTION((m_pModule->SetWindowOffset(41)),    Require);
}
//  Test extra search margin:

void
MatchTiming::ExtraSearchTest()
{
  for(unsigned int i = 0; i < 2048; i += 73) {
    m_pModule->SetExtraSearchMargin(i);
    CCAENV1x90::TriggerConfiguration current = 
      m_pModule->GetTriggerConfiguration();

    EQ(i, m_pModule->GetExtraSearchMargin(current));
  }

  EXCEPTION(m_pModule->SetExtraSearchMargin(2048), Require);

}
// Test reject margin:

void
MatchTiming::RejectMarginTest()
{
  for(unsigned int i = 0; i <= 0x3ff; i = (i << 1) | 1) {
    m_pModule->SetRejectMargin(i);
    CCAENV1x90::TriggerConfiguration current = 
      m_pModule->GetTriggerConfiguration();
    EQ(i, m_pModule->GetRejectMargin(current));
  }
  EXCEPTION(m_pModule->SetRejectMargin(2048), Require);

}

//  Test ability to toggle in/out of trigger subtraction mode.

void
MatchTiming::TriggerSubtractTest()
{
  CCAENV1x90::TriggerConfiguration  current;

  m_pModule->EnableTriggerTimeSubtraction();
  current = m_pModule->GetTriggerConfiguration();
  EQ(true, m_pModule->isTriggerTimeSubtracted(current));

  m_pModule->DisableTriggerTimeSubtraction();
  current = m_pModule->GetTriggerConfiguration();
  EQ(false, m_pModule->isTriggerTimeSubtracted(current));

  
}
