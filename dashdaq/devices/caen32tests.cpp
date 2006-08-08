// Template for a test suite.

#include <config.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include <CVMESubsystem.h>
#include <CSBSVMEInterface.h>
#include <CVMEAddressRange.h>
#include <CAEN32.h>
#include <stdint.h>
#include <iostream>
#include <stdlib.h>
using namespace std;

// Change the lines below if the module moves:

static const int      vme(0);
static const uint32_t base(0x80000000);
static Warning msg("caen32tests needs a CAEN32module at base address 0x80000000");

class caen32test : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(caen32test);
  CPPUNIT_TEST(construct);
  CPPUNIT_TEST(config);
  CPPUNIT_TEST(init);
  CPPUNIT_TEST_SUITE_END();


private:
  CVMESubsystem*     m_pSubsystem;
  CSBSVMEInterface*  m_pVmeCrate;
  CAEN32*            m_pModule;
public:
  void setUp() {
    m_pSubsystem = new CVMESubsystem;
    m_pVmeCrate  = new CSBSVMEInterface(vme);
    m_pSubsystem->installInterface(*m_pVmeCrate, true);
    m_pModule    = new CAEN32((*m_pSubsystem)[0], base);
  }
  void tearDown() {
    delete m_pModule;
    delete m_pSubsystem;
    usleep(1000);
  }
protected:
  void construct();
  void config();
  void init();
};

CPPUNIT_TEST_SUITE_REGISTRATION(caen32test);

// Construction should : 
//  set up the configuration to the default
//  values... a rather long test here.

void caen32test::construct() {
  // can't test firmware, cardtype or serial number... this will vary with
  // the test setup.  We can out put them though:

  cerr << endl;
  cerr << "Testing CAEN V" << m_pModule->getCardType() << endl;
  cerr << "Firmware rev: 0x" << hex << m_pModule->getFirmware() << dec << endl;
  cerr << "Serial num:   " << m_pModule->getSerial()   << endl;

  // Check the configuration against the default.
  
  EQMSG("crate", (uint8_t)0, m_pModule->cgetCrate());
  EQMSG("geo",   (uint8_t)1, m_pModule->cgetGeo());
  EQMSG("enabled", true,     m_pModule->cgetCardEnable());
  for (int i=0;  i < 32; i++) {
    char msg[100];
    sprintf(msg, "enable %d", i);
    EQMSG(msg, true, m_pModule->cgetChannelEnable(i));
  }
  EQMSG("fast clear window", (uint16_t)0, m_pModule->cgetFastClearInterval());
  
  EQMSG("threshold range", CAEN32::small, m_pModule->cgetThresholdMeaning());
  EQMSG("cbltmembership",  CAEN32::Unchained, m_pModule->cgetCBLTPosition());
  EQMSG("cblt base",       (uint32_t)(0xaa),  m_pModule->cgetCBLTBase());
  
  EQMSG("clear on read", true, m_pModule->cgetClearAfterRead());

  vector<uint16_t> thresholds = m_pModule->cgetThresholds();
  for (int i =0; i < 32; i++) {
    char msg[100];
    sprintf(msg, "Threshold channel %d", i);
    EQMSG(msg, (uint16_t)0x10, thresholds[i]);
  }

  // Cannot do the 775/862 specific ones.
}
// Another gi-humongous test... the alternative is many many
// smaller tests.. you make the call.
// What we wll do is exercise each config/cget pair
// of functions.
void
caen32test::config()
{
  m_pModule->configCrate(0xa);
  EQMSG("crate", (uint8_t)0xa, m_pModule->cgetCrate());

  m_pModule->configGeo(12);
  EQMSG("geo", (uint8_t)12, m_pModule->cgetGeo());
  
  m_pModule->configCardEnable(false);
  EQMSG("card enable", false, m_pModule->cgetCardEnable());

  for (int i = 0; i < 32; i++) {
    char msg[100];
    sprintf(msg, "Channel %d", i);
    m_pModule->configChannelEnable(i, false);
    EQMSG(msg, false, m_pModule->cgetChannelEnable(i));
  }

  m_pModule->configFastClearInterval(123);
  EQMSG("fast clear", (uint16_t)123, m_pModule->cgetFastClearInterval());

  m_pModule->configThresholdMeaning(CAEN32::large);
  EQMSG("threshold meaning", CAEN32::large, m_pModule->cgetThresholdMeaning());

  m_pModule->configCBLTMembership(CAEN32::First, 0xbb);
  EQMSG("membership base", (uint32_t)0xbb, m_pModule->cgetCBLTBase());
  EQMSG("membership position", CAEN32::First, m_pModule->cgetCBLTPosition());

  m_pModule->configClearAfterRead(false);
  EQMSG("clear after read", false, m_pModule->cgetClearAfterRead());
}

// YAHT  Yet Another Huge Test.  The module's initialization function
// is called and then we go through comprehensively figuring out if the
// registers were correctly set up.
//
void 
caen32test::init()
{
  m_pModule->initialize();

  CVMEAddressRange& module(*(m_pVmeCrate->createAddressRange(0xe, base, 0x9000)));
  
  // Crate select: 

  uint16_t crate = module.peekw(0x103c/sizeof(uint16_t)) & 0xff;

  EQMSG("Crate select", (unsigned short)m_pModule->cgetCrate(), crate);
  
  // keep bits in bitset 2 are off.

  uint16_t bs2 = module.peekw(0x1032/sizeof(uint16_t));
  EQMSG("keeps", (int)0x38, bs2 & 0x38);
  EQMSG("online", (int)0, bs2 & 2);
  EQMSG("threshold range", (int)0x100, bs2 & 0x100);

  // The Kill bits must all be 0; and the thresholds must be the default 0x10.

  for (int i=0; i < 32; i++) {
    char msg[100];
    sprintf(msg, "Kill bit channel %d", i);
    uint16_t threshold = module.peekw(0x1080/sizeof(uint16_t) + i);
    EQMSG(msg, 0, threshold & 0x100);
    sprintf(msg, "Threshold for channel %d", i);
    EQMSG(msg, 0x10, threshold & 0xff);
  }

  //  Fast clear window is 0:

  uint16_t fclear = module.peekw(0x102e/sizeof(uint16_t));
  EQMSG("fclear", 0, fclear & 0x3ff);


  // Chained block configurattion:

  uint16_t cblt = module.peekw(0x101a/sizeof(uint16_t));
  EQMSG("mcstcbltcontrol", (int)0, cblt & 0x3);
  cblt = module.peekw(0x1004/sizeof(uint16_t));
  EQMSG("mcstaddr", 0xaa, cblt & 0xff);

  delete &module;
}
