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
  EQMSG("clksrc", CSIS3300::internal80_100Mhz, m_pModule->cgetClockSource());
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

  m_pModule->initialize();

  CVMEAddressRange& all(m_pModule->getGroupRegisters(1));
			


  
}
