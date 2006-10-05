// Template for a test suite.

#include <config.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include "CBiRA3251.h"

#include <CVMESubsystem.h>
#include <CSBSVMEInterface.h>
#include <CWienerVC32.h>
#include <CVC32CC32.h>

#include <stdint.h>

using namespace std;

/// Use the stuff below to configure the test... change the warning too please

static const uint32_t vmecrate = 0;
static const uint32_t camaccontroller(0);
static const uint32_t vc32base = 0xc00000;
static const size_t   biRaSlot = 9;
static Warning msg("bira3251tests needs a BiRA 3251 in slot 9  of crate 0@sbsvme0, a vc32 @ 0xc00000");

class bira3251tests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(bira3251tests);
  CPPUNIT_TEST(write);
  CPPUNIT_TEST_SUITE_END();


private:
  CVMESubsystem*    m_pSubsystem;
  CSBSVMEInterface* m_pVmeCrate;
  CWienerVC32*      m_pController;
  CBiRA3251*        m_pModule;

public:
  void setUp() {
    m_pSubsystem = new CVMESubsystem;
    m_pVmeCrate  = new CSBSVMEInterface(vmecrate);
    m_pSubsystem->installInterface(*m_pVmeCrate, true);
    m_pController= new CWienerVC32(0, vc32base);
    m_pController->addCrate(*(new CVC32CC32(*m_pController)),0);
    m_pModule    = new CBiRA3251((*m_pController)[0], 
				 biRaSlot);
  }
  void tearDown() {
    delete m_pModule;
    delete m_pController;
    delete m_pSubsystem;
  }
protected:
  void write();
};

CPPUNIT_TEST_SUITE_REGISTRATION(bira3251tests);

void bira3251tests::write() {
  for (int i =0; i < 0x1000; i++) {
    m_pModule->write(i);
  }
}
