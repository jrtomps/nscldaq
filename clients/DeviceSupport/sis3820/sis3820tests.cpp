// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include "CSIS3820.h"
#include <iostream.h>
#include <unistd.h>
#include <stdio.h>

#define SISBase 0x38000000
const double FWlevel(1.1);

class sis3820tests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(sis3820tests);
  CPPUNIT_TEST(MapTestFail);
  CPPUNIT_TEST(MapTestOk);
  CPPUNIT_TEST(TestFirmware);
  CPPUNIT_TEST(TestCSRRead);
  CPPUNIT_TEST(TestReset);
  CPPUNIT_TEST(LatchMode);
  CPPUNIT_TEST(InvalidMode);
  CPPUNIT_TEST(TestInputModes);
  CPPUNIT_TEST(TestLNESource);
  CPPUNIT_TEST(Arm);
  CPPUNIT_TEST(Enable);
  CPPUNIT_TEST(ReferencePulser);
  CPPUNIT_TEST(CounterTestMode);
  CPPUNIT_TEST(Read1Channel);
  CPPUNIT_TEST(ReadAllChannels);
  CPPUNIT_TEST(AllChannelsCount);
  CPPUNIT_TEST(LatchTest);
  CPPUNIT_TEST(ClearChannels);
  CPPUNIT_TEST(ClearOnLatch);
  CPPUNIT_TEST_SUITE_END();


private:
  CSIS3820* m_pModule;
public:
  void setUp() {
    m_pModule = new CSIS3820(SISBase);
  }
  void tearDown() {
    delete m_pModule;
  }
protected:
  void MapTestFail();
  void MapTestOk();
  void TestFirmware();
  void TestCSRRead();
  void TestReset();
  void LatchMode();
  void InvalidMode();
  void TestInputModes();
  void TestLNESource();
  void Arm();
  void Enable();
  void ReferencePulser();
  void CounterTestMode();
  void Read1Channel();
  void ReadAllChannels();
  void AllChannelsCount();
  void LatchTest();
  void ClearChannels();
  void ClearOnLatch();

  // Utilities:

  void ArmInLatchMode();
  void SetupReferencePulser();
  void Setup25MhzCounter();
  void Count2sec();
};


CPPUNIT_TEST_SUITE_REGISTRATION(sis3820tests);


//
// Test failing map of module.
//  The constructor should throw a string exception:
//  
void
sis3820tests::MapTestFail()
{
  try {
    CSIS3820 module(SISBase+4);	// Shift a bit to bad base address.
  }
  catch (string msg) {
    return;
  }
  catch (...) {
    cerr << "Weird exception type!!\n";
    return;
  }
  FAIL("Created a module where none is in the crate");
}


//  Test successful map.. should not throw!!
void
sis3820tests::MapTestOk()
{
  try {
    CSIS3820 module(SISBase);
  }
  catch (string msg) {
    FAIL("Unable to create module at correct base address");
    return;
  }
}
//  Test the firmware rev fetch.
// 
void
sis3820tests::TestFirmware()
{
  double fw;
  fw   = m_pModule->getRevision();
  EQMSG("Firmware compare", FWlevel,fw);
}

//  Test Access to the CSR  this is a bit complex  we need
//  to set a bit there (done by turning on the led
//  we check then that the LED thinks it's on.
//  then we turn it off and check that it's off
//
void
sis3820tests::TestCSRRead()
{
  m_pModule->LightOn();		// Turn on the LED.
  unsigned int csr;
  csr = m_pModule->getCsr();
  EQMSG("Led not on", static_cast<unsigned int>(1), (csr & 1));

  m_pModule->LightOff();	// Turn off the LED.
  csr = m_pModule->getCsr();
  EQMSG("Led not off", static_cast<unsigned int>(0), (csr & 1));
}
//
//  Test the reset function of the module.
/// After a rest, the module user led should be off.
//
void
sis3820tests::TestReset()
{

  m_pModule->LightOn();		// give reset something to do.

  m_pModule->Reset();
 
  unsigned int csr = m_pModule->getCsr();
  EQMSG("Led not off", static_cast<unsigned int>(0), (csr & 1));
}

//  Test the ability to set the scaler mode to latching mode.
//
void
sis3820tests::LatchMode()
{
  m_pModule->setOperatingMode(CSIS3820::LatchingScaler);

  EQMSG("Incorrect operating mode", CSIS3820::LatchingScaler, 
	m_pModule->getOperatingMode());

}
//  Test that attempts to put the scaler into an invalid mode
//  throw a string exception.
void
sis3820tests::InvalidMode()
{
  try {
    m_pModule->setOperatingMode(CSIS3820::ReservedScaler1);
  } 
  catch(string text) {
    return;

  }
  catch(...) {
    FAIL("Invalid mode test - wrong exception thrown");
    return;
  }
  FAIL("Invalid mode test - no exception thrown");
}
//
// Test the ability to set all supported input modes:
//
void
sis3820tests::TestInputModes()
{
  CSIS3820::InputMode Valid[] = {
    CSIS3820::NoInputs,
    CSIS3820::InputLatchInhibitLatch,
    CSIS3820::InputLatchInhibitAllAndLatch,
    CSIS3820::InputLatchInhibitAll,
    CSIS3820::InhibitGroups,
  };
  int validmodecount = sizeof(Valid)/sizeof(CSIS3820::InputMode);

  CSIS3820::InputMode Invalid[] = {
    CSIS3820::InputReserved5,
    CSIS3820::InputReserved6,
    CSIS3820::InputReserved7
  };
  int invalidmodecount = sizeof(Invalid)/sizeof(CSIS3820::InputMode);

  // For all valid modes:
  //  - The mode must set without an exception.
  //  - The mode set must be the same as the one we tried to set.

  for(int i = 0; i < validmodecount; i++) {
    CSIS3820::InputMode mode = Valid[i];
    try {
      m_pModule->setInputMode(mode);
    }
    catch (...) {
      FAIL("Set of valid mode threw exception");
    }
    mode = m_pModule->getInputMode();
    EQMSG("Mode set/get mismatch", Valid[i], mode);

  }
  // For all the invalid modes
  //  - The mode set must throw an exception.
  //  - The exception must be a string exception.
  //  - The mode must not be altered.

  bool failed;
  CSIS3820::InputMode initialMode = m_pModule->getInputMode();
  for(int i =0; i < invalidmodecount; i++) {
    CSIS3820::InputMode mode = Invalid[i];
    try {
      failed = true;
      m_pModule->setInputMode(mode);
    }
    catch (string msg) {
      failed = false;
    }
    catch (...) {
    }
    if(failed) {
      FAIL("Exception not thrown on invalid input mode set");
    }
    EQMSG("Invalid mode changed mode!!", initialMode,
	  m_pModule->getInputMode());
  }
  
}
//  Test the ability to set the latch source.  We support:
//
void
sis3820tests::TestLNESource()
{
  CSIS3820::LNESource SupportedSources [] = {
    CSIS3820::LatchVMEOnly,
    CSIS3820::LatchFP,
    CSIS3820::Latch10Mhz 
  };
  int nSupported = sizeof(SupportedSources)/sizeof(CSIS3820::LNESource);
  
  CSIS3820::LNESource UnSupportedSources [] = {
    CSIS3820::LatchChannelN,
    CSIS3820::LatchPresetN,
    CSIS3820::LatchReserved5,
    CSIS3820::LatchReserved6,
    CSIS3820::LatchReserved7
  };
  int nUnsupported = sizeof(UnSupportedSources)/
                     sizeof(CSIS3820::LNESource);
			 
  // Each supported latch source:
  // - should set without an exception.
  // - should actually set the requested source.

  
  for(int i = 0; i < nSupported; i++ ) {
    CSIS3820::LNESource mode = SupportedSources[i];
    try {
      m_pModule->setLatchSource(mode);
    }
    catch (...) {
      FAIL("Supported latch source set failed");
    }
    EQMSG("Supported latch source did not read as set",
	  mode, m_pModule->getLatchSource());
  }
  
  // Each unsupported latch source:
  // - should throw an exception if set.
  // - the exception should be a string exception.
  // - The latch source should not change.

  bool failed;
  CSIS3820::LNESource CurrentMode = m_pModule->getLatchSource();
  for(int i= 0; i < nUnsupported; i++) {
    CSIS3820::LNESource mode = UnSupportedSources[i];
    failed = true;
    try {
      m_pModule->setLatchSource(mode);
    }
    catch (string msg) {
      failed = false;		// This is the correct result.
    }
    catch (...) {
      // Wrong exception type.

      FAIL("Incorrect exception type for unsupported latch source");
    }
    if(failed) {
      FAIL("No exception thrown for unsupported latch source");
    }

    EQMSG("Mode changed on unsupported latch source ",
	  CurrentMode, m_pModule->getLatchSource());

  }


    
}
//  Test the ability to arm the scaler module to count.
//  To count, if I understand correctly, the module must be
//  armed and enabled.
void
sis3820tests::Arm()
{
  m_pModule->Arm();

  ASSERT(m_pModule->isArmed());


}
//   Test ability to enable/disable the module.
//
void
sis3820tests::Enable()
{
  m_pModule->Enable();
  ASSERT(m_pModule->isEnabled());

  m_pModule->Disable();
  ASSERT(!m_pModule->isEnabled());
}
//  Test ability to turn on/off the reference pulser.
void
sis3820tests::ReferencePulser()
{
  m_pModule->EnableReferencePulser();
  ASSERT(m_pModule->isReferencePulserEnabled());

  m_pModule->DisableReferencePulser();
  ASSERT(!m_pModule->isReferencePulserEnabled());
}
//  Test the ability to turn on and off the counter
//  Test mode (25mHz counter into all channels).
void
sis3820tests::CounterTestMode()
{
  m_pModule->EnableTestCounter();
  ASSERT(m_pModule->isTestCounterOn());
 
  m_pModule->DisableTestCounter();
  ASSERT(!m_pModule->isTestCounterOn());
}
// Set module in latch mode and arm:
void
sis3820tests::ArmInLatchMode()
{
  m_pModule->setOperatingMode(CSIS3820::LatchingScaler);
  m_pModule->Arm();
}
// 
// Function to put the module in latching mode and setup the
// reference pulser.  The scaler is left armed and ready
//  This can only be used within a test as it assumes that
// m_pModule is valid.
void
sis3820tests::SetupReferencePulser()
{
  ArmInLatchMode();
  m_pModule->EnableReferencePulser();


}

// Enable the scaler count 2 secs, disable.

void 
sis3820tests::Count2sec()
{
  m_pModule->Enable();		// Scaler is now counting.  
  sleep(2);			// Let it count...


}

// Test ability to read a single channel of the scaler in
// on the fly mode.
//  We:
//    enable the reference pulser, arm/enable the module
//    read once, sleep, read again and ensure that the values
//    differ.
//    Note that an unclocked channel should not change!!.
void
sis3820tests::Read1Channel()
{
  // test range checkingon ReadChannel:

  bool failed = true;
  try {
    m_pModule->ReadChannel(32);	// boundary condition.
  }
  catch (string msg) {
    failed = false;		// should throw a string.
  }
  catch (...) {
    FAIL("Bad channel number threw wrong exception type");
  }
  if(failed) {
    FAIL("Bad channel number did not throw an exception!!");
  }

  // Test that the channels count.

  SetupReferencePulser();
  

  // The scaler is not yet counting... read it then start it..
  
  unsigned long initial = 
    m_pModule->ReadChannel(0); // Read first channel.
  unsigned long uninitial = 
    m_pModule->ReadChannel(1);	// This channel won't change.

  Count2sec();

  unsigned long final = 
    m_pModule->ReadChannel(0);

  if(initial == final) {
    FAIL("Read1Channel - initial value == final value");
  }
  EQMSG("Unclocked channel channged :-(",
	uninitial, m_pModule->ReadChannel(1));

}
//  Test that all channels read. THis test is essentially the same
//  as the previous one, however all channels are read.. .the first
//  channel should be a changing channel, the rest unchanging.
//
void
sis3820tests::ReadAllChannels()
{
  // Test that the channels count.

  SetupReferencePulser();

  // The scaler is not yet counting... read it then start it..
  
  unsigned long initial[32];
  m_pModule->ReadAllChannels(initial);

  Count2sec();

  unsigned long final[32];
  m_pModule->ReadAllChannels(final);

  for (int i =0; i < 32; i++) {
    if((i != 0)) {
      EQ(initial[i], final[i]);
    }
    
  }
}

void
sis3820tests::Setup25MhzCounter()
{
  ArmInLatchMode();
  m_pModule->EnableTestCounter();

}
// Test that all channels can be made to count by
// turning on the TestCounter, counting for 2sec
// and ensuring that all channel values have changed.
//
void sis3820tests::AllChannelsCount()
{
  Setup25MhzCounter();

  unsigned long initial[32];
  m_pModule->ReadAllChannels(initial);

  Count2sec();


  unsigned long final[32];
  m_pModule->ReadAllChannels(final);

  for(int i =0; i < 32; i++) {
    char msg[100];
    sprintf(msg, "Channel %d is not counting", i);
    if(initial[i] == final[i]) {
      FAIL(msg);
    }

 }


}
//  Test the latch into the shadow registers.
//  1. Latch - read the latched registers.
//  2. start all channels counting on the 25mhz test counter
//     for 2 sec.
//  3. Read the latched registers - they should not have changed.
//  4. Latch
//     Read the latched registers - they should have changed now.
void sis3820tests::LatchTest()
{

  // Start the 25mhz test counter:

  Setup25MhzCounter();

  // Latch the counters before we count.

  unsigned long initial[32];
  unsigned long final[32];

  m_pModule->LatchAndRead(initial);

  Count2sec();



  // Simon didn't say latch so the latched registers should
  // be unchanged!!

  m_pModule->ReadAllLatchedChannels(final);
  for(int i =0; i < 32; i++) {
    char msg[100];
    sprintf(msg, "Latched channel %d changed without latch", i);
    EQMSG(msg, initial[i], final[i]);
  }

  m_pModule->LatchAndRead(final); // Now the latched registers change
  for(int i = 0; i < 32; i++) {
    char msg[100];
    if(initial[i] == final[i]) {
      sprintf(msg, "Initial = final channel %d values %d %d",
	      i, initial[i], final[i]);
      FAIL(msg);
    }
  }
    
}
//   Test ability to clear all channels.  What we do is
//   first run the LatchTest... that should leave us with
//   channels having values and the 25Mhz test counter enabled.
//   We then:
//   - stop the 25mhz counter.
//   - Disable counting for good measure.
//   - Clear all channels.
//   - Latch and verify that all shadow registers now contain 0.
//
void
sis3820tests::ClearChannels()
{
  LatchTest();			// Get the channels 'dirty'.
  m_pModule->Disable();
  m_pModule->DisableTestCounter();

  // Clear the channels.

  m_pModule->ClearChannels();

  // Latch and read the channels:

  unsigned long buf[32];
  m_pModule->LatchAndRead(buf);
  for(int i = 0; i < 32; i++) {
    char msg[100];
    sprintf(msg, "Channel %d is non zero", i);
    EQMSG(msg, static_cast<unsigned long>(0), buf[i]);
  }

}
//  Test the functionality of the noclear on latch.
//  By default the module clears the counters on a latch
//  Therefore with the 25mhz input in, a read of 2 sec followed
//  by a read of 1 sec should result in fewer counts the second
//  read.
//  If we then turn off this feature (DisableClearOnLatch)
//  this will reverse
//  If we turn it back on (EnbaleClearOnLatch) once more...
//
void
sis3820tests::ClearOnLatch()
{
  Setup25MhzCounter();		// Armed and ready...
  m_pModule->ClearChannels();	// All channels zeroed...

  unsigned long buf2sec[32];
  unsigned long buf1sec[32];

  Count2sec();
  m_pModule->LatchAndRead(buf2sec);
  sleep(1);
  m_pModule->LatchAndRead(buf1sec);
  
  for(int i=0; i < 32; i++) {
    if(buf1sec[i] >= buf2sec[i]) { // only if didn't clear!!
      char msg[100];
      sprintf(msg, "Channel %d looks like it didn't clear %d %d",
	      i, buf1sec[i], buf2sec[i]);
      FAIL(msg);
    }
  }

  // Now:
  // - Turn off the clear
  // - Disable,
  // - Clear
  // - Enable 
  //  and count...

  m_pModule->DisableClearOnLatch();
  m_pModule->Disable();
  m_pModule->ClearChannels();		// start from zero...
  Count2sec();
  m_pModule->LatchAndRead(buf2sec);
  sleep(1);
  m_pModule->LatchAndRead(buf1sec);

  // the second interval should have started with the values
  // from the first interval.

  for(int i =0; i < 32; i++) {
    if(buf1sec[i] <= buf2sec[i]) {
      char msg[100];
      sprintf(msg, "Channel %d looks like it did clear %d %d",
	      i, buf1sec[i], buf2sec[i]);
      FAIL(msg);
    }
  }

  // Go the other way:
  //
  m_pModule->EnableClearOnLatch();
  m_pModule->Disable();
  m_pModule->ClearChannels();
  Count2sec();
  m_pModule->LatchAndRead(buf2sec);
  sleep(1);
  m_pModule->LatchAndRead(buf1sec);

  for(int i=0; i < 32; i++) {
    if(buf1sec[i] >= buf2sec[i]) { // only if didn't clear!!
      char msg[100];
      sprintf(msg, "Channel %d looks like it didn't clear %d %d",
	      i, buf1sec[i], buf2sec[i]);
      FAIL(msg);
    }
  }
}
