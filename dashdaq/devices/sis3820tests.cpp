// Template for a test suite.

#include <config.h>

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"

#include <CVMESubsystem.h>
#include <CSBSVMEInterface.h>
#include <CVMEAddressRange.h>

#include <CSIS3820.h>
#include <stdint.h>
#include <iostream>
#include <stdlib.h>
#include <unistd.h>



// Change the three lines below to match the
// base address of your modules:


static const uint32_t baseAddress=(0x01000000);
static const int      vmeCrate(0);
static Warning msg("sis3820tests requires an SIS 3820 at base address 0x01000000");



//

static const uint32_t O(0);	// Variable named O!!

using namespace std;


class sis3820tests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(sis3820tests);
  CPPUNIT_TEST(construct);
  CPPUNIT_TEST(configure);
  CPPUNIT_TEST(initialize);
  CPPUNIT_TEST(refpulsetest);
  CPPUNIT_TEST(testpulsetest);
  CPPUNIT_TEST(mcstest);
  CPPUNIT_TEST(histotest);
  CPPUNIT_TEST_SUITE_END();


private:
  CVMESubsystem*    m_pSubsystem;
  CSBSVMEInterface* m_pVmeCrate;
  CSIS3820*         m_pModule;

public:
  void setUp() {
    m_pSubsystem = new CVMESubsystem;
    m_pVmeCrate  = new CSBSVMEInterface(vmeCrate);
    m_pSubsystem->installInterface(*m_pVmeCrate, true);
    m_pModule    = new CSIS3820(*m_pVmeCrate, baseAddress);
  }
  void tearDown() {
  }
protected:
  void construct();
  void configure();
  void initialize();
  void refpulsetest();
  void testpulsetest();
  void mcstest();
  void histotest();


};

CPPUNIT_TEST_SUITE_REGISTRATION(sis3820tests);


// check that intialization set the right defaults.

void sis3820tests::construct() {
  cerr << "Testing sis3820\n";
  EQMSG("mode", CSIS3820::Latching, m_pModule->cgetMode());
  EQMSG("refpulseron", false, m_pModule->cgetRefPulserEnabled());
  EQMSG("mcspreset",   O,     m_pModule->cgetMCSPreset());
  EQMSG("lneprescale", O,     m_pModule->cgetLNEPrescale());
  EQMSG("disableClear",true, m_pModule->cgetClearOnLNEEnabled());
  EQMSG("dataformat", CSIS3820::format32, m_pModule->cgetDataFormat());
  EQMSG("lnesource",  CSIS3820::LNEVME,   m_pModule->cgetLNESource());
  EQMSG("armenableSrc", CSIS3820::ArmFPLNE, m_pModule->cgetArmEnableSource());
  EQMSG("SDRAMisFIFO", true, m_pModule->cgetSDRAMisFIFO());
  EQMSG("inputsInverted", CSIS3820::normal, m_pModule->cgetInputPolarity());
  EQMSG("inputmode",  CSIS3820::LneInhibitLneAndAll, m_pModule->cgetInputMode());
  EQMSG("outputsinverted", CSIS3820::normal,  m_pModule->cgetOutputPolarity());
  EQMSG("outputmode", CSIS3820::Unused,       m_pModule->cgetOutputMode());
  EQMSG("copydisbales", O, m_pModule->cgetCopyDisableMask());
  EQMSG("channeld isables", O, m_pModule->cgetChannelDisableMask());


}
// Should be able to configure every thing in the module:
// Note that we are still not affecting the module state,
// We are just affeting what would be putinto the state on an Initialize().
void sis3820tests::configure() 
{
  m_pModule->configMode(CSIS3820::MCS);
  EQMSG("mode->mcs", CSIS3820::MCS, m_pModule->cgetMode());

  m_pModule->configEnableRefPulser();
  EQMSG("refpulser on", true, m_pModule->cgetRefPulserEnabled());

  m_pModule->configMCSPreset(100);
  EQMSG("mcs preset->100", (uint32_t)100, m_pModule->cgetMCSPreset());

  m_pModule->configLNEPrescale(255);
  EQMSG("lne prescale->255", (uint32_t)255, m_pModule->cgetLNEPrescale());

  m_pModule->configDataFormat(CSIS3820::format16);
  EQMSG("data format->16 bits", CSIS3820::format16, m_pModule->cgetDataFormat());

  m_pModule->configDisableClearOnLNE();
  EQMSG("LNE Clear disabled", false, m_pModule->cgetClearOnLNEEnabled());

  m_pModule->configLNESource(CSIS3820::LNE10MHz);
  EQMSG("LNE Source ->> 10Mhz", CSIS3820::LNE10MHz, m_pModule->cgetLNESource());

  m_pModule->configArmEnableSource(CSIS3820::ArmFPLNE);	// only option.
  EQMSG("Enable src->FPLNE", CSIS3820::ArmFPLNE, m_pModule->cgetArmEnableSource());

  m_pModule->configSDRAMasFIFO();
  EQMSG("SDRAM->FIFO MODE", true, m_pModule->cgetSDRAMisFIFO());

  m_pModule->configInputPolarity(CSIS3820::inverted);
  EQMSG("Input polarity inverted", CSIS3820::inverted, m_pModule->cgetInputPolarity());
  
  m_pModule->configInputMode(CSIS3820::NoInputs);
  EQMSG("Inputs disabled", CSIS3820::NoInputs, m_pModule->cgetInputMode());

  m_pModule->configOutputPolarity(CSIS3820::inverted);
  EQMSG("out polarity->inverted", CSIS3820::inverted, m_pModule->cgetOutputPolarity());

  m_pModule->configOutputMode(CSIS3820::Clock);
  EQMSG("out mode -> clock", CSIS3820::Clock, m_pModule->cgetOutputMode());

  m_pModule->configCopyDisableMask(0xaaaaaaaa);
  EQMSG("copydismask is as", (uint32_t)0xaaaaaaaa, m_pModule->cgetCopyDisableMask());
  
  m_pModule->configChannelDisableMask(0x55555555);
  EQMSG("chann disables->5's.", (uint32_t)0x55555555, m_pModule->cgetChannelDisableMask());
  
}

// After initialization are all the right status bit registers etc.
// set the way we want them.

void sis3820tests::initialize() {

  m_pModule->configCopyDisableMask(0xaaaaaaaa);	// every other channel not copied
  m_pModule->configChannelDisableMask(0x55555555); // copied channels not counting.
  m_pModule->configSDRAMasRAM();
  m_pModule->initialize(); 

  uint32_t csr = m_pModule->regRead(0);
  EQMSG("csr bits", (uint32_t)0x00010000, csr & 0x0fffffff);
  
  uint32_t id  = m_pModule->regRead(4);
  EQMSG("Type", (uint32_t)0x38200000, id & 0xffff0000); // Don't care about firmware rev 

  uint32_t acqcount = m_pModule->regRead(0x14);
  EQMSG("Acq count", O, acqcount);

  uint32_t mcsPreset = m_pModule->cgetMCSPreset();
  EQMSG("MCS Preset", mcsPreset, m_pModule->regRead(0x10));

  uint32_t lnePrescale = m_pModule->cgetLNEPrescale();
  EQMSG("LNE Prescale", lnePrescale, m_pModule->regRead(0x18));

  uint32_t  acqmode   = m_pModule->regRead(0x100);

  CSIS3820::mode mode = m_pModule->cgetMode();
  uint32_t modeBits;
  switch (mode) {
   case CSIS3820::Latching:
      modeBits  = 0x00000000;
      break;
  case CSIS3820::MCS:
      modeBits  = 0x20000000;
      break;
  case CSIS3820::Histogramming:
      modeBits  = 0x30000000;
      break;
  }

  EQMSG("mode", modeBits, (m_pModule->regRead(0x100) & 0x70000000));

  uint32_t outpolmask;
  if (m_pModule->cgetOutputPolarity() == CSIS3820::normal) {
    outpolmask = 0x00000000;
  }
  else {
    outpolmask = 0x00800000;
  }
  EQMSG("Out polarity", outpolmask, acqmode & 0x00800000);

  uint32_t outputmode;
  CSIS3820::OutputMode omode = m_pModule->cgetOutputMode();
  switch (omode) {
  case CSIS3820::SDRAM:
    outputmode = 0;
    break;
  case CSIS3820::Clock:
    outputmode = 0x001000000;
    break;
  case CSIS3820::Unused:
    outputmode = 0x00300000;
    break;
  }
  EQMSG("output mode", outputmode, acqmode & 0x00300000);


  bool fifo = m_pModule->cgetSDRAMisFIFO();
  uint32_t sdrambit;
  if (fifo) {
    sdrambit = 0x00000000;
  } 
  else {
    sdrambit = 0x1000;
  }
  EQMSG("sdramis fifo", sdrambit, acqmode & 0x00001000);
  EQMSG("armenable bits", O, acqmode & 0x00000300);

  CSIS3820::LNESource lnesrc = m_pModule->cgetLNESource();
  uint32_t lnesrcbits;
  switch (lnesrc) {
  case CSIS3820::LNEVME:
    lnesrcbits = 0;
    break;
  case CSIS3820::LNEFrontPanel:
    lnesrcbits = 0x10;
    break;
  case CSIS3820::LNE10MHz:
    lnesrcbits = 0x20;
    break;
    
  }
  EQMSG("LNE src", lnesrcbits, acqmode & 0x30);


  CSIS3820::DataFormat format = m_pModule->cgetDataFormat();
  uint32_t fmtbits;
  switch (format) {
  case CSIS3820::format32:
    fmtbits = 0;
    break;
  case CSIS3820::format24:
    fmtbits = 4;
    break;
  case CSIS3820::format16:
    fmtbits = 8;
    break;
  case CSIS3820::format08:
    fmtbits = 0xc;
    break;
  }
  EQMSG("data format", fmtbits , acqmode & 0xc);

  EQMSG("clearon lne", m_pModule->cgetClearOnLNEEnabled() ? O : (int32_t) 1,
	acqmode & 0x1);

  uint32_t cpyDisables = m_pModule->cgetCopyDisableMask();
  EQMSG("cpy disable", cpyDisables, m_pModule->regRead(0x104));
  
  uint32_t inhibits = m_pModule->cgetChannelDisableMask();
  EQMSG("inhibits", inhibits, m_pModule->regRead(0x200));


}

// Turn on the reference pulser so that a read will not be boring.
// clear the counters and wait for 1ms.  Then look at the counters.
// All but channel 0 should be zero...
// This  is a read test too.
//

void sis3820tests::refpulsetest() {
  m_pModule->configEnableRefPulser();
  m_pModule->initialize();

  usleep(1000);

  uint32_t buffer[32];
  m_pModule->read(buffer);
  ASSERT(buffer[0] != 0);
  for (int i=1; i<32; i++) {
    ASSERT(buffer[i] == 0);
  }
}
// Turn on the test pulser.. now all channels should be nonzero and equal too.
//
void sis3820tests::testpulsetest() {
  m_pModule->configEnableTestPulser();
  m_pModule->initialize();
  uint32_t csr = m_pModule->regRead(0);


  usleep(1000);

  uint32_t buffer[32];
  m_pModule->read(buffer);
  ASSERT(buffer[0] != 0);
  for (int i=1; i < 32; i++) {
    ASSERT(buffer[i] == buffer[0]);
  }
}
void sis3820tests::mcstest() {
  m_pModule->configMode(CSIS3820::MCS);
  m_pModule->configEnableRefPulser();
  m_pModule->configLNEPrescale(105000); // 0.1+ seconds.
  m_pModule->configMCSPreset(10); // 10 cycles of it.
  m_pModule->configLNESource(CSIS3820::LNE10MHz);
  m_pModule->configSDRAMasRAM();
  m_pModule->initialize();
  uint32_t csr = m_pModule->regRead(0);


  usleep(110000);			// at least one mCS period.

  uint32_t buffer[32];
  uint32_t initial=buffer[0];
  memset(buffer, 0, 32*sizeof(uint32_t));
  m_pModule->read(buffer);

  ASSERT(buffer[0] != 0);
  for (int i =1; i < 32; i++) {
    ASSERT(buffer[i] == 0);
  }

  usleep(110000);			// One more mcs period.
  uint32_t buf2[32];
  memset(buf2, 0, 32*sizeof(uint32_t));
  uint32_t in2=buf2[0];
  m_pModule->read(buf2);
  ASSERT(buf2[0] != 0);
  for (int i =1; i < 32; i++) {
    ASSERT(buf2[i] == 0);
  }
  ASSERT(buf2[0] - buffer[0]  < 2);

}

void sis3820tests::histotest()
{
  m_pModule->configMode(CSIS3820::Histogramming);
  m_pModule->configEnableRefPulser();
  m_pModule->configLNEPrescale(10500); // 0.1+ seconds.
  m_pModule->configMCSPreset(10); // 10 cycles of it.
  m_pModule->configLNESource(CSIS3820::LNE10MHz);
  m_pModule->configSDRAMasRAM();
  m_pModule->initialize();
  uint32_t csr = m_pModule->regRead(0);
  uint32_t amode = m_pModule->regRead(0x100);


  usleep(110000);			// at least one mCS period.

  uint32_t buffer[32];
  uint32_t initial=buffer[0];
  memset(buffer, 0, 32*sizeof(uint32_t));
  m_pModule->read(buffer);

  ASSERT(buffer[0] != 0);
  for (int i =1; i < 32; i++) {
    ASSERT(buffer[i] == 0);
  }

  usleep(110000);			// One more mcs period.
  uint32_t buf2[32];
  memset(buf2, 0, 32*sizeof(uint32_t));
  uint32_t in2=buf2[0];
  m_pModule->read(buf2);
  ASSERT(buf2[0] != 0);
  for (int i =1; i < 32; i++) {
    ASSERT(buf2[i] == 0);
  }

}
