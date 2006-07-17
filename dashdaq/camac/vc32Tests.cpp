// Template for a test suite.

#include <config.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include <CVMESubsystem.h>
#include <CSBSVMEInterface.h>
#include <CWienerVC32.h>
#include <CVC32CC32.h>

using namespace std;

static Warning msg("vc32Tests require a VC32 in VME crate 0, address 0xC00000");
static Warning msg2("vc32Tests require a CC32 to be plugged in and online");

static uint32_t vc32base = 0xc00000; // Don't forget to change the message too.

class vc32tests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(vc32tests);
  CPPUNIT_TEST(maxcrates);
  CPPUNIT_TEST(crate0);
  CPPUNIT_TEST(remove);
  CPPUNIT_TEST(vmesrtest);
  CPPUNIT_TEST(CZ);
  CPPUNIT_TEST(inhibit);
  CPPUNIT_TEST(lamflipflop);
  CPPUNIT_TEST(cycletune);
  CPPUNIT_TEST(reset);
  CPPUNIT_TEST_SUITE_END();


private:
  CVMESubsystem*    m_pSubsystem;
  CSBSVMEInterface* m_pVmeCrate;
  CWienerVC32*      m_pController;
public:
  void setUp() {
    m_pSubsystem = new CVMESubsystem;
    m_pVmeCrate  = new CSBSVMEInterface(0);
    m_pSubsystem->installInterface(*m_pVmeCrate, true);
    m_pController= new CWienerVC32(0, vc32base);
  }
  void tearDown() {
    delete m_pController;
    delete m_pSubsystem;
  }
protected:
  void maxcrates();
  void crate0();
  void remove();
  void vmesrtest();
  void CZ();
  void inhibit();
  void lamflipflop();
  void cycletune();
  void reset();
};

CPPUNIT_TEST_SUITE_REGISTRATION(vc32tests);

void vc32tests::maxcrates() {
  EQ((size_t)1, m_pController->maxCrates());
}
void vc32tests::crate0() {
  EQ(false, m_pController->haveCrate(0)); // Not installed.

  CVC32CC32* pCrate = new CVC32CC32(*m_pController);
  m_pController->addCrate(*pCrate, 0);

  EQ(true, m_pController->haveCrate(0));
}

void vc32tests::remove()
{
  CVC32CC32* pCrate = new CVC32CC32(*m_pController);
  m_pController->addCrate(*pCrate, 0);

  EQ(pCrate, 
     dynamic_cast<CVC32CC32*>(m_pController->removeCrate(0)));
  delete pCrate;
}

void vc32tests::vmesrtest()
{
  // Status register read should show the 32k window bit set.

  uint16_t sr = m_pController->readStatus();
  EQMSG("32kbit", (int)CWienerVC32::VC32Status::size32k,
	sr & CWienerVC32::VC32Status::size32k);
  
  // SHolkd be able to set the Interrupt vector to whatever I want:

  m_pController->writeStatus(0xff); // all bits on..
  sr = m_pController->readStatus();
  EQMSG("vector ff", 0xff, sr & 0xff);

  m_pController->writeStatus(0); // all bits off.
  sr = m_pController->readStatus();
  EQMSG("vector 00", 0, sr & 0xff);
  
}
// All we can do with C/Z is ensure that there are no exceptions.

void vc32tests::CZ()
{
  m_pController->C();
  m_pController->Z();
}
// Toggle the inhibit on /off and check it in the CC32status reg.

void vc32tests::inhibit()
{
  m_pController->unInhibit();
  uint16_t sr = m_pController->readCC32Status();
  EQMSG("uninhibited", 0, sr & CWienerVC32::CC32Status::Inhibit);

  m_pController->Inhibit();
  sr   = m_pController->readCC32Status();
  EQMSG("inhibited", (int)CWienerVC32::CC32Status::Inhibit,
	sr & CWienerVC32::CC32Status::Inhibit);
}
// Can't really get lams set with our setup, but we can check that
// there's consistency between the bit in vc32satus and cc32status and
// that it can be forced to 0.
//
void vc32tests::lamflipflop()
{
  uint16_t vcsr   = m_pController->readStatus();
  uint16_t ccsr   = m_pController->readCC32Status();

  EQMSG("initial ==", (vcsr & CWienerVC32::VC32Status::lamfifo) ? 1 : 0,
	(ccsr & CWienerVC32::CC32Status::LamFlipFlop) ? 1 : 0);

  // Resetting should make both 0 for sure:

  m_pController->resetLamFlipFlop();
  EQMSG("vcsr lamfifo == 0", 0, (vcsr & CWienerVC32::VC32Status::lamfifo) ? 1 : 0);
  EQMSG("ccsr lamfifo == 0", 0, 
	(ccsr & CWienerVC32::CC32Status::LamFlipFlop) ? 1 : 0);
	

}
// Should be able to do anything to the cycle tune registers.
void vc32tests::cycletune()
{
  uint16_t ctune;
  m_pController->writeCycleTuneA(0xffff);
  ctune = m_pController->readCycleTuneA();

  EQMSG("A 0xffff", (uint16_t)0xffff, ctune);

  m_pController->writeCycleTuneA(0);
  ctune = m_pController->readCycleTuneA();
  EQMSG("A 0", (uint16_t)0, ctune);

  m_pController->writeCycleTuneB(0xaaaa);
  ctune   = m_pController->readCycleTuneB();
  EQMSG("B 0xaaaa", (uint16_t)0xaaaa, ctune);

  m_pController->writeCycleTuneB(0);
  ctune   = m_pController->readCycleTuneB();
  EQMSG("B 0", (uint16_t)0, ctune);

  m_pController->writeCycleTuneC(0x5555);
  ctune = m_pController->readCycleTuneC();
  EQMSG("C 0x5555", (uint16_t)0x5555, ctune);

  m_pController->writeCycleTuneC(0);
  ctune = m_pController->readCycleTuneC();
  EQMSG("C 0", (uint16_t)0, ctune);
  
}
// reset should reset cycle tune registers to 0 (standard CAMAC timings).

void vc32tests::reset()
{
  m_pController->writeCycleTuneA(0xffff);
  m_pController->reset();

  uint16_t tune = m_pController->readCycleTuneA();
  EQ((uint16_t)0, tune);
}
