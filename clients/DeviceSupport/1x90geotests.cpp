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


extern long ModuleBase;

class GeoTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(GeoTests);
  CPPUNIT_TEST(SlotMatch);
  CPPUNIT_TEST(SlotSet);
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
  void SlotMatch();
  void SlotSet();
};

CPPUNIT_TEST_SUITE_REGISTRATION(GeoTests);


// After enabling 
// The trigger time tag enable bit should be set in the control register.

void GeoTests::SlotMatch() {
  EQMSG("Initial slot ", 1, (int)m_pModule->GetGeographicalID());
}


// Should be able to turn off trigger time tag.

void
GeoTests::SlotSet()
{
  // The legal ones:

  for(unsigned short nSlot = 0; nSlot < 32; nSlot++) {
    m_pModule->SetGeographicalID(nSlot);
    EQ(nSlot, m_pModule->GetGeographicalID());
  }

  // Try 256, the exact limit:

  EXCEPTION((m_pModule->SetGeographicalID(32)),
	    DesignByContract::Require);


}
