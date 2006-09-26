// Template for a test suite.

#include <config.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"

#include <CVMESubsystem.h>
#include <CSBSVMEInterface.h>
#include <CVMEAddressRange.h>

#include <CSIS3300.h>
#include <stdint.h>
#include <iostream>
#include <stdlib.h>
#include <unistd.h>

using namespace std;

//
// Edit below this line if needed to change the test module location.
//
static const uint32_t baseAddress=(0x20000000);
static const int      vmeCrate(0);
static Warning msg("sis3300tests requires an SIS 3300/1 @ 0x20000000");



class sis3300tests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(sis3300tests);
  CPPUNIT_TEST(construct);
  CPPUNIT_TEST(config);
  CPPUNIT_TEST(init);
  CPPUNIT_TEST_SUITE_END();


private:
  CVMESubsystem*    m_pSubsystem;
  CSBSVMEInterface* m_pVmeCrate;
  CSIS3300*         m_pModule;
public:
  void setUp() {
    m_pSubsystem = new CVMESubsystem;
    m_pVmeCrate  = new CSBSVMEInterface(vmeCrate);
    m_pSubsystem->installInterface(*m_pVmeCrate, true);
    m_pModule    = new CSIS3300(*m_pVmeCrate, baseAddress);
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

CPPUNIT_TEST_SUITE_REGISTRATION(sis3300tests);

void sis3300tests::construct() {
  EQMSG("Triggout enabled", true, m_pModule->cgetTriggerOutputIsEnabled());
  EQMSG("ouput inverted?", false, m_pModule->cgetTriggerOutputIsInverted());
  EQMSG("active when armed", true, m_pModule->cgetTriggerIsActiveWhenArmed());
  EQMSG("outmask", (uint32_t)0, m_pModule->cgetBankFullOutputMask());
  EQMSG("bank1clock", true, m_pModule->cgetBank1ClockIsEnabled());
  EQMSG("bank2clock", false, m_pModule->cgetBank2ClockIsEnabled());
  EQMSG("autoswitch", false, m_pModule->cgetAutoBankSwitchIsEnabled());
  EQMSG("rcm", false, m_pModule->cgetRandomClockIsEnabled());
  EQMSG("autostart", false, m_pModule->cgetAutoStartIsEnabled());
  EQMSG("multievent", false, m_pModule->cgetIsMultiEventMode());
  EQMSG("startdelayon", false, m_pModule->cgetStartDelayIsEnabled());
  EQMSG("stopdelayon", false, m_pModule->cgetStopDelayIsEnabled());
  EQMSG("fpstartstop", true, m_pModule->cgetFrontPanelStartStopIsEnabled());
  EQMSG("fpgatemode",  false, m_pModule->cgetFPGateModeIsEnabled());
  EQMSG("extrcm", false, m_pModule->cgetIsExternRandomClockMode());
  EQMSG("clksrc", CSIS3300::internal80_100MHz, m_pModule->cgetClockSource());
  EQMSG("mux", false, m_pModule->cgetMultiplexMode());
  EQMSG("startdelay", (uint16_t)0,  m_pModule->cgetStartDelay());
  EQMSG("stopdelay", (uint16_t)0, m_pModule->cgetStopDelay());
  EQMSG("tsprediv", (uint16_t)0, m_pModule->cgetTimestampPredivider());
  EQMSG("pagesize", CSIS3300::page1K, m_pModule->cgetPageSize());
  EQMSG("wrap",   false, m_pModule->cgetWrapIsEnabled());
  EQMSG("gatechaining", false, m_pModule->cgetIsGateChaining());
  for (int i =0; i < 8; i++) {
    CSIS3300::Threshold t = m_pModule->cgetChannelThreshold(i);
    EQMSG("Threshold value", (uint16_t)0x3fff, t.s_value);
    EQMSG("Threhosld le", false, t.s_le);
  }
  EQMSG("clockprediv", (uint8_t)0, m_pModule->cgetClockPredivider());
  EQMSG("Muxsamples", (uint8_t)0, m_pModule->cgetMuxModeSamples());
  EQMSG("CFDNumerator", (uint8_t)0, m_pModule->cgetCFDNumerator());
  EQMSG("CFDDenom.", (uint8_t)0, m_pModule->cgetCFDDenominator());
  EQMSG("CFDWidth", (uint8_t)0, m_pModule->cgetCFDWidth());
  EQMSG("CFDEnable", false, m_pModule->cgetCFDIsEnabled());
  EQMSG("chainmax", (uint16_t)0, m_pModule->cgetChainMaxEvents());
  EQMSG("groupread", (uint8_t)0xf, m_pModule->cgetGroupEnables());
  EQMSG("tag groups", true, m_pModule->cgetTagGroups());
  EQMSG("tag data", false, m_pModule->cgetTaggingData());
  EQMSG("data tag", (uint16_t)0x1234, m_pModule->cgetDataTag());

}

// Should be able to configure and read the changed config for all of those

void sis3300tests::config()
{
  m_pModule->configTriggerOutput(false);
  EQMSG("trig out off", false, m_pModule->cgetTriggerOutputIsEnabled());
  
  m_pModule->configTriggerOutputInverted(true);
  EQMSG("trig invert", true, m_pModule->cgetTriggerOutputIsInverted());

  m_pModule->configTriggerActiveWhenArmed(false);
  EQMSG("active when armed", false, m_pModule->cgetTriggerIsActiveWhenArmed());

  m_pModule->configBankFullOutputMask(0xf);
  EQMSG("bank full output mask", (uint32_t)0xf, m_pModule->cgetBankFullOutputMask());

  m_pModule->configEnableBank1Clock(false);
  m_pModule->configEnableBank2Clock(true);
  EQMSG("bank 1 clock disabled", false, m_pModule->cgetBank1ClockIsEnabled());
  EQMSG("bank 2 clock enabled", true,  m_pModule->cgetBank2ClockIsEnabled());

  m_pModule->configAutoBankSwitchEnabled(true);
  EQMSG("autoswitch on", true, m_pModule->cgetAutoBankSwitchIsEnabled());

  m_pModule->configRandomClockEnable(true);
  EQMSG("rcm", true, m_pModule->cgetRandomClockIsEnabled());

  m_pModule->configAutoStartEnable(true);
  EQMSG("autostarting", true, m_pModule->cgetAutoStartIsEnabled());

  m_pModule->configMultiEventMode(true);
  EQMSG("multievent mode", true, m_pModule->cgetIsMultiEventMode());

  m_pModule->configEnableStartDelay(true);
  EQMSG("start delay on", true, m_pModule->cgetStartDelayIsEnabled());
  
  m_pModule->configEnableStopDelay(true);
  EQMSG("stop delay on", true, m_pModule->cgetStopDelayIsEnabled());

  m_pModule->configEnableFrontPanelStartStop(false);
  EQMSG("no front panel start/stop", false,
	m_pModule->cgetFrontPanelStartStopIsEnabled());

  m_pModule->configEnableFPGateMode(true);
  EQMSG("Now in gate mode", true,
	m_pModule->cgetFPGateModeIsEnabled());
  
  m_pModule->configExternRandomClockMode(true);
  EQMSG("External rcm", true, m_pModule->cgetIsExternRandomClockMode());

  m_pModule->configMultiplexMode(true);
  EQMSG("muxmode", true, m_pModule->cgetMultiplexMode());

  m_pModule->configStartDelay(123);
  EQMSG("startdelay", (uint16_t)123, m_pModule->cgetStartDelay());
  
  m_pModule->configStopDelay(321);
  EQMSG("stopdelay", (uint16_t)321, m_pModule->cgetStopDelay());

  m_pModule->configTimestampPredivider(0x5555);
  EQMSG("timestamp prediv", (uint16_t)0x5555, m_pModule->cgetTimestampPredivider());

  m_pModule->configPageSize(CSIS3300::page512);
  EQMSG("page size", CSIS3300::page512, m_pModule->cgetPageSize());

  m_pModule->configWrapEnable(true);
  EQMSG("wrapenable", true, m_pModule->cgetWrapIsEnabled());

  m_pModule->configGateChaining(true);
  EQMSG("gatechaining", true, m_pModule->cgetIsGateChaining());

  for (int i =0; i < 8; i++) {
    CSIS3300::Threshold t; 
    t.s_le = ((i % 2) == 0);
    t.s_value = i;
    m_pModule->configChannelThreshold(i, t);
 
    
    CSIS3300::Threshold a = m_pModule->cgetChannelThreshold(i);
    EQMSG("le", t.s_le, a.s_le);
    EQMSG("thresh", t.s_value, a.s_value);
  }

  m_pModule->configClockPredivider(111);
  EQMSG("clock prediv", (uint8_t)111, m_pModule->cgetClockPredivider());

  m_pModule->configMuxModeSamples(2);
  EQMSG("mux samples", (uint8_t)2, m_pModule->cgetMuxModeSamples());
  
  m_pModule->configCFDNumerator(50);
  EQMSG("cfd numerator", (uint8_t)50, m_pModule->cgetCFDNumerator());

  m_pModule->configCFDDenominator(75);
  EQMSG("cfd denominator", (uint8_t)75, m_pModule->cgetCFDDenominator());

  m_pModule->configCFDWidth(128);
  EQMSG("cfd width", (uint8_t)128, m_pModule->cgetCFDWidth());

  m_pModule->configCFDEnable(true);
  EQMSG("cfd enable", true, m_pModule->cgetCFDIsEnabled());

  m_pModule->configChainMaxEvents(5);
  EQMSG("max chain events", (uint16_t)5, m_pModule->cgetChainMaxEvents());

  m_pModule->configGroupEnables(0xaa);
  EQMSG("groupenables", (uint8_t)0xaa, m_pModule->cgetGroupEnables());


  
}
// Test the init.
//
void
sis3300tests::init()
{

  uint32_t fwid  = m_pModule->readFirmware();
  uint32_t major = (fwid >> 8) & 0xff;
  uint32_t minor = (fwid & 0xff);

  ASSERT(major != 10);		// We don't want amanda firmware in our lab.

  // Using the firmware rev level figure out some of the capabilities of the
  // device:

  bool hasGateMode(false);	// Gate mode for trigger.
  bool hasDisableMode(false);	// Disable timestampe clear mode.
  bool hasCFD(false);		// CFD based trigger.
  bool hasMuxMode(false);
  bool hasClockPredivider(false);
  bool ecrHasGroupId(false);
  bool ecrHasEnableTriggerEventDir(false);

  if (major >= 3) {
    hasGateMode = true;
    hasCFD      = true;

    if ((major == 3) && (minor >= 7)) {
      hasDisableMode = true;
    }
    // These are guesses about when they come in:
    hasMuxMode         = true;		
    hasClockPredivider = true;
    ecrHasGroupId      = true;
    ecrHasEnableTriggerEventDir = true;
  }

  m_pModule->configMuxModeSamples(5);
  m_pModule->configClockPredivider(0xf);
  for (int chan =0; chan < 8; chan++) {
    CSIS3300::Threshold t; 
    t.s_value  = chan*10;
    t.s_le     = ((chan%2) == 0);
    m_pModule->configChannelThreshold(chan,t);
  }
  m_pModule->configPageSize(CSIS3300::page512);

  m_pModule->initialize();

  CVMEAddressRange& all(m_pModule->getGroupRegisters(1));
  CVMEAddressRange& csr(m_pModule->getControlRegisters());
		
  // Check out the value of control registers we can check.
  // note that what we can check will be somewhat modified by the
  // firmware id as not all modules are created equal.

  // If the module has a CFD trigger capability check out its register.

  if (hasCFD) {
    uint32_t tsetup = 0;
    if (m_pModule->cgetCFDIsEnabled())             tsetup |= 0x01000000;
    if (m_pModule->cgetPulseTriggerIsEnabled()) tsetup |= 0x10000000;
    tsetup |= (m_pModule->cgetCFDNumerator()   >> 8) & 0xf;;
    tsetup |=  m_pModule->cgetCFDDenominator() & 0xf;
    tsetup |= (m_pModule->cgetCFDWidth() >> 16) & 0xf;
    EQMSG("TriggerSetupReg", tsetup, m_pModule->readReg(all, 0x28));

  }
  // Multiplexor sample count.

  if (hasMuxMode) {
    EQMSG("mux mode samples", 
	  (uint32_t)m_pModule->cgetMuxModeSamples(), 
	  m_pModule->readReg(all, 0x24));
  }
  // Clock predivider.

  if (hasClockPredivider) {
    uint32_t cpd = m_pModule->cgetClockPredivider();
    EQMSG("clock prediv.", cpd, m_pModule->readReg(all, 0x20));
  }
  // CSIS3300::Threshold registers (I think those were in rev 1).

  CSIS3300::Threshold ts[8];
  for (int i=0; i < 8; i++) {
    ts[i] = m_pModule->cgetChannelThreshold(i);
  }
  for (int g = 0; g < 4; g++) {
    int first = g*2;		// first channel in group.
    CVMEAddressRange& grp(m_pModule->getGroupRegisters(g+1));
    uint32_t expected  = 0;
    expected |= ts[first].s_value << 16;
    expected |= ts[first+1].s_value;
    expected |= ts[first].s_le ? 1 << 31 : 0;
    expected |= ts[first+1].s_le ? 1 << 15 : 0;
    EQMSG("threshold", expected, m_pModule->readReg(grp, 0x4));
  }
  // Now the event configuration register.  That existed from t=0,
  // but I'm not sure all the modern bits are supported...we'llsee.
  //
  uint32_t econf = 0;
  if (ecrHasGroupId) econf |= 0x100;
  if (ecrHasEnableTriggerEventDir) econf |= 0x1000;

  switch (m_pModule->cgetPageSize()) {
  case CSIS3300::page128K:
    econf |= 0;			// for completeness.
    break;
  case CSIS3300::page16K:
    econf |= 1;
    break;
  case CSIS3300::page4K:
    econf |= 2;
    break;
  case CSIS3300::page2K:
    econf |= 3;
    break;
  case CSIS3300::page1K:
    econf |= 4;
    break;
  case CSIS3300::page512:
    econf |= 5;
    break;
  case CSIS3300::page256:
    econf |= 6;
    break;
  case CSIS3300::page128:
    econf |= 7;
    break;
  }
  if (m_pModule->cgetWrapIsEnabled()) econf  |= 8;
  if (m_pModule->cgetIsGateChaining()) econf |= 0x10;
  if (m_pModule->cgetIsExternRandomClockMode()) econf |= 0x800;
  if (hasMuxMode & m_pModule->cgetMultiplexMode()) econf |= 0x8000;
  EQMSG("Event config reg", econf,  m_pModule->readReg(all, 0));

  // Predividers etc:

  EQMSG("time stamp predivider:", (uint32_t)m_pModule->cgetTimestampPredivider(),
	m_pModule->readReg(csr, 0x1c));
  EQMSG("stop delay", (uint32_t)m_pModule->cgetStartDelay(),
	m_pModule->readReg(csr, 0x14));
  EQMSG("start delay", (uint32_t)m_pModule->cgetStopDelay(),
	m_pModule->readReg(csr, 0x18));

  uint32_t acqcontrol = m_pModule->readReg(csr, 0x10);

  uint32_t expectedacq(0);
  if (m_pModule->cgetBank1ClockIsEnabled()) expectedacq |= 1;
  if (m_pModule->cgetBank2ClockIsEnabled()) expectedacq |= 2;
  if (m_pModule->cgetAutoBankSwitchIsEnabled()) expectedacq |= 4;
  if (m_pModule->cgetAutoStartIsEnabled()) expectedacq |= 0x10;
  if (m_pModule->cgetIsMultiEventMode())   expectedacq |= 0x20;
  if (m_pModule->cgetStartDelayIsEnabled()) expectedacq |= 0x40;
  if (m_pModule->cgetStopDelayIsEnabled())  expectedacq |= 0x80;
  if (m_pModule->cgetFrontPanelStartStopIsEnabled()) expectedacq |= 0x100;
  if (m_pModule->cgetFPGateModeIsEnabled()) expectedacq |= 0x400;
  if (m_pModule->cgetIsExternRandomClockMode()) expectedacq |= 0x800;

  switch (m_pModule->cgetClockSource()) {
  case CSIS3300::external:
    expectedacq |= 0x6000;
    break;
  case CSIS3300::internal3pt125KHz:
    expectedacq |= 0x5000;
    break;
  case CSIS3300::internal6pt250KHz:
    expectedacq |= 0x4000;
    break;
  case CSIS3300::internal12500KHz:
    expectedacq |= 0x3000;
    break;
  case CSIS3300::internal20_25MHz:
    expectedacq |= 0x2000;
    break;
  case CSIS3300::internal40_50MHz:
    expectedacq |= 0x1000;
    break;
  case CSIS3300::internal80_100MHz:
    expectedacq |= 0x0000;	// I know, not needed but it's illustrative.
    break;

  }
  if (hasMuxMode) {
    if (m_pModule->cgetMultiplexMode()) {
      expectedacq |= 0x8000;
    }
  }
  EQMSG("ACQ register", expectedacq, m_pModule->readReg(csr, 0x10) & 0xffff);

  // I'm not quite sure what the bits in the csr will be so we'll ignore that
  // for now.

  

}
