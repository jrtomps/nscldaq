#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include <CVMEInterface.h>
#include <SBSBit3API.h>		// assums sbsbit3 interface.
#include "v890.h"
#include <stdio.h>

#define slot 2
#define crate 0

static const int us(1000);	// Ns in a microsecond

// Requires a module in slot slot  with base set ot 0xeeee0000!!

class V890Test : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(V890Test);
  CPPUNIT_TEST(testConstruction);
  CPPUNIT_TEST(testMapping);
  CPPUNIT_TEST(testModuleId);
  CPPUNIT_TEST(testBadModule);
  CPPUNIT_TEST(testResets);
  CPPUNIT_TEST(testReadEmpty);
  CPPUNIT_TEST(testReadTest);
  CPPUNIT_TEST(testLoadDefaultConfig);
  CPPUNIT_TEST(testIndividualChannelEnables);
  CPPUNIT_TEST(testStorageMode);
  CPPUNIT_TEST(testWindowWidth);
  CPPUNIT_TEST(testWindowOffset);
  CPPUNIT_TEST(testSubtractionOnOff);
  CPPUNIT_TEST(testEdgeDetection);
  CPPUNIT_TEST(testResolution);
  CPPUNIT_TEST(testDelimeters);
  CPPUNIT_TEST(testMaxHits);
  CPPUNIT_TEST(testDataReady);
  CPPUNIT_TEST_SUITE_END();


private:
  CAENV890* m_pModule;
public:
  void setUp() {
    try {
      m_pModule = new CAENV890(slot, crate); // Module in slot 6.
    }
    catch(string& fail) {
      cerr << "Setup caught exception: " << fail << endl;
      throw;
    }
  }
  void tearDown() {
    delete m_pModule;
    m_pModule = (CAENV890*)NULL;

  }
protected:
  void testConstruction();
  void testMapping();
  void testModuleId();
  void testBadModule();
  void testSlotProgramming();
  void testResets();
  void testReadEmpty();
  void testReadTest();
  void testLoadDefaultConfig();
  void testIndividualChannelEnables();
  void testStorageMode();
  void CheckEnables(const char*, u_short mask);
  void testWindowWidth();
  void testWindowOffset();
  void testSubtractionOnOff();
  void testEdgeDetection();
  void testResolution();
  void testDelimeters();
  void testMaxHits();
  void testDataReady();
};

CPPUNIT_TEST_SUITE_REGISTRATION(V890Test);

// Utility function to check that the enables mask values are all correct.

void 
V890Test::CheckEnables(const char* pTest, u_short mask)
{
  u_short Enables[8];
  m_pModule->ReadChannelEnables(Enables);
  for(int i = 0; i < 8; i++) {
    char message[100];
    sprintf(message,"%s Checking enables for channels %d-%d", 
	    pTest, (i-1)*16, i*16-1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(message, mask, Enables[i]);
  }
}

/*!
   Test that construction has initialized all members correctly:
*/
void 
V890Test::testConstruction()
{
  CAENV890& module(*m_pModule);

  CPPUNIT_ASSERT(module.getSlot()  == slot);
  CPPUNIT_ASSERT(module.getCrate() == crate);
  CPPUNIT_ASSERT(module.getGeo()   == true);
  CPPUNIT_ASSERT(module.getBase()  == 0l);
}
/*!
   Test that we got a base address for the module:
*/
void
V890Test::testMapping() {
  CAENV890& module(*m_pModule);
  CPPUNIT_ASSERT(module.getRegisters() != (unsigned short*)0);
  CPPUNIT_ASSERT(module.getVmeDevice() != (void*)0);
}

/*!
   Test that we have the correct module type:
*/
void
V890Test::testModuleId()
{
  CPPUNIT_ASSERT(m_pModule->getModuleType() == 890);
}


/*!
   Attempt to map a module in slot 5.. this should fail.
*/
void
V890Test::testBadModule()
{
  try{
    CAENV890 module(5);
    CPPUNIT_FAIL("Creating V890 in slot 5 should have failed!!");
  }
  catch (...) {
  }
}

/*!
   Check the resets work.
*/
void
V890Test::testResets()
{
  m_pModule->EmptyEvent(true);	// This bit gets reset on module reset:
  CPPUNIT_ASSERT(m_pModule->EmptyEventOn());

  m_pModule->Reset();		// Full module reset.
  CPPUNIT_ASSERT(!m_pModule->EmptyEventOn()); // Reset should kill it.
  
  m_pModule->EmptyEvent(true);	// Module clears don't reset this.
  m_pModule->Clear();
  CPPUNIT_ASSERT(m_pModule->EmptyEventOn());

}
/*!
   Test that reading a module with no data gives the right stuff.
*/
void
V890Test::testReadEmpty()
{
  u_long buffer[100];
  u_int  nWords =  m_pModule->Read(sizeof(buffer)/sizeof(u_long), 
				   buffer);

  CPPUNIT_ASSERT_EQUAL_MESSAGE("Reading empty buffer memory", 0, (int)nWords);
}
/*!
  Put the module into test mode and read it.
*/
void
V890Test::testReadTest()
{
  u_long buffer[100];
  u_int  nWords;
  u_long pattern(0x01020304);

  m_pModule->TestMode(true, pattern);
  
  nWords = m_pModule->Read(sizeof(buffer)/sizeof(u_long),
			   buffer);
  CPPUNIT_ASSERT_EQUAL(1,(int)nWords);
  CPPUNIT_ASSERT_EQUAL((int)pattern, (int)buffer[0]);

  m_pModule->TestMode(false);
  testReadEmpty();
  
  
}

/*!
  This does a simple test of the micro sequencer by asking it to load
  the default configuration and then checking to see that it was loaded
  correctly.
  Trigger info is 5 words:
  - Match window width
  - Trigger offset
  - Extra search window
  - Reject margin
  - Trigger time subtraction enable.
*/
void
V890Test::testLoadDefaultConfig()
{
  CAENV890::TriggerInfo  TriggerConfig;

  m_pModule->LoadDefaultConfig(); // Load the module default configuration 
                                  // which should be:
  CPPUNIT_ASSERT(!m_pModule->isTriggerMatching()); // Trigger matching off.

  CPPUNIT_ASSERT_EQUAL(500, m_pModule->GetWindowWidth());

  m_pModule->ReadTriggerConfig(&TriggerConfig);

  CPPUNIT_ASSERT_EQUAL(-us, 
		       (int)TriggerConfig.Offset*CAENV890::m_nTdcClock); // Window offset 
  CPPUNIT_ASSERT_EQUAL(200, 
		       (int)TriggerConfig.ExtraSearchWindow*CAENV890::m_nTdcClock); // Extra search margin.
  CPPUNIT_ASSERT_EQUAL(100, 
		       (int)TriggerConfig.RejectMargin*CAENV890::m_nTdcClock); // Reject margin 100ns.

  // They don't say what the value of the trigger time subtraction enable is.
  
  // All channels should be enabled:

  u_short Enables[8];
  m_pModule->ReadChannelEnables(Enables);

  CheckEnables("testLoadDefaultConfig - all enables", 0xffff);
  
}

/*!
   Test the enables of individual channels.  First we enable
   the even channels check and then the odd channels.
   Then we enable all channels, disable all channels and 
   finally write a rotating bit pattern.
*/
void
V890Test::testIndividualChannelEnables()
{
  CAENV890& Module(*m_pModule);

  // Enable evens and disable odds:

  for(int i  =0; i < 128; i ++) {
    if((i % 2) == 0) {
      Module.EnableChannel(i);
    } 
    else {
      Module.DisableChannel(i);
    }
  }
  // Check that the evens are on and the odds are off:


  CheckEnables("testIndividualChannelEnables - Even bits", 0x5555);

  // Enable odds and disable events:
  for(int i = 0; i < 128; i++) {
    if((i%2) == 0) {
      Module.DisableChannel(i);
    }
    else {
      Module.EnableChannel(i);
    }
  }
  CheckEnables("testIndividualChannelEnables - Odd bits", 0xaaaa);

  // Enable all channels:

  Module.EnableAllChannels();
  CheckEnables("testIndividualChannelEnables - All on", 0xffff);

  // Disable all channels:

  Module.DisableAllChannels();
  CheckEnables("testIndividualChannelEnables - All off", 0);

  // Rotating bit pattern:

  u_short inPattern[8];
  u_short outPattern[8];

  for(int i =0; i < 8; i++) {
    inPattern[i] = 1 << i;
  }
  Module.SetChannelMask(inPattern);
  Module.ReadChannelEnables(outPattern);
  for(int i =0; i < 8; i ++) {
    char message[100];
    sprintf(message, "Rotating bit check channels %d-%d", (i-1)*16, i*16-1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(message, inPattern[i], outPattern[i]);
  }

}
/*!
   Puts the module into continuous storage mode checks and then
   puts it into trigger match mode.
*/
void
V890Test::testStorageMode()
{
  m_pModule->SetTriggerMatchingMode();
  CPPUNIT_ASSERT(m_pModule->isTriggerMatchingMode());

  m_pModule->SetContinuousStorageMode();
  CPPUNIT_ASSERT(!m_pModule->isTriggerMatchingMode());

}

/*!   
   - Check that on power up the window width is 500ns.
   - Set the window width to a series of widths from 
     0 - 100usec in 10 usec steps, and ensures that this
     happens.  Note the window width is set in ns.
*/
void
V890Test::testWindowWidth()
{
  CPPUNIT_ASSERT_EQUAL_MESSAGE("Default width check",
			       500, m_pModule->GetWindowWidth());
  for(int i =0; i < 100; i += 10) {
    m_pModule->SetWindowWidth(i*us); // width in ns.
    CPPUNIT_ASSERT_EQUAL(i*us, m_pModule->GetWindowWidth());
  }
}
/*!
    - Check that on power up the window offset is -1us.
    - Check that the offset can be set from -50usec - 0 usec 
      in 5usec units.
*/
void 
V890Test::testWindowOffset()
{
  CPPUNIT_ASSERT_EQUAL_MESSAGE("Default window check",
			       -us, m_pModule->GetWindowOffset());
  for(int i = -50; i <= 0; i += 5) {
    m_pModule->SetWindowOffset(i*us);
    CPPUNIT_ASSERT_EQUAL(i*us, m_pModule->GetWindowOffset());
  }
}
/*!
   Test the ability to turn on/off the subtraction mode of the module.
*/
void
V890Test::testSubtractionOnOff()
{
  m_pModule->EnableTriggerSubtraction();
  CPPUNIT_ASSERT(m_pModule->TriggerSubtractionEnabled());
  
  m_pModule->DisableTriggerSubtraction();
  CPPUNIT_ASSERT(!m_pModule->TriggerSubtractionEnabled());
}
/*!
  Test the ability to set all possible edge detection modes, and read them
  back
*/
void
V890Test::testEdgeDetection()
{
  m_pModule->SetEdgeDetection(CAENV890::EdgePairs);
  CPPUNIT_ASSERT_EQUAL(CAENV890::EdgePairs,
			m_pModule->GetEdgeDetection());

  m_pModule->SetEdgeDetection(CAENV890::LeadingEdge);
  CPPUNIT_ASSERT_EQUAL(CAENV890::LeadingEdge,
			m_pModule->GetEdgeDetection());

  m_pModule->SetEdgeDetection(CAENV890::TrailingEdge);
  CPPUNIT_ASSERT_EQUAL(CAENV890::TrailingEdge,
			m_pModule->GetEdgeDetection());

  m_pModule->SetEdgeDetection(CAENV890::EitherEdge);
  CPPUNIT_ASSERT_EQUAL(CAENV890::EitherEdge,
			m_pModule->GetEdgeDetection());

}

/*!
   Test the ability to set/get the tdc resolution.
*/
void
V890Test::testResolution()
{
  m_pModule->SetResolution(CAENV890::ps800);
  CPPUNIT_ASSERT_EQUAL(CAENV890::ps800,
		       m_pModule->GetResolution());

  m_pModule->SetResolution(CAENV890::ps200);
  CPPUNIT_ASSERT_EQUAL(CAENV890::ps200,
		       m_pModule->GetResolution());

  m_pModule->SetResolution(CAENV890::ps100);
  CPPUNIT_ASSERT_EQUAL(CAENV890::ps100,
		       m_pModule->GetResolution());

}
/*!
   Test the ability to turn on/off the header/eob which I decided to
   collectively call delimeters.
*/
void
V890Test::testDelimeters()
{
  m_pModule->EnableDelimeters();
  CPPUNIT_ASSERT(m_pModule->DelimetersEnabled());

  m_pModule->DisableDelimeters();
  CPPUNIT_ASSERT(!m_pModule->DelimetersEnabled());
}
/*!
   Test the ability to set/get the maximum hit count.
*/
void
V890Test::testMaxHits()
{
  m_pModule->SetMaxHits(CAENV890::HitMax0);
  CPPUNIT_ASSERT_EQUAL(CAENV890::HitMax0,
		       m_pModule->GetMaxHits());

  m_pModule->SetMaxHits(CAENV890::HitMax1);
  CPPUNIT_ASSERT_EQUAL(CAENV890::HitMax1,
		       m_pModule->GetMaxHits());

  m_pModule->SetMaxHits(CAENV890::HitMax2);
  CPPUNIT_ASSERT_EQUAL(CAENV890::HitMax2,
		       m_pModule->GetMaxHits());

  m_pModule->SetMaxHits(CAENV890::HitMax4);
  CPPUNIT_ASSERT_EQUAL(CAENV890::HitMax4,
		       m_pModule->GetMaxHits());

  m_pModule->SetMaxHits(CAENV890::HitMax8);
  CPPUNIT_ASSERT_EQUAL(CAENV890::HitMax8,
		       m_pModule->GetMaxHits());

  m_pModule->SetMaxHits(CAENV890::HitMax16);
  CPPUNIT_ASSERT_EQUAL(CAENV890::HitMax16,
		       m_pModule->GetMaxHits());

  m_pModule->SetMaxHits(CAENV890::HitMax32);
  CPPUNIT_ASSERT_EQUAL(CAENV890::HitMax32,
		       m_pModule->GetMaxHits());

  m_pModule->SetMaxHits(CAENV890::HitMax64);
  CPPUNIT_ASSERT_EQUAL(CAENV890::HitMax64,
		       m_pModule->GetMaxHits());

  m_pModule->SetMaxHits(CAENV890::HitMax128);
  CPPUNIT_ASSERT_EQUAL(CAENV890::HitMax128,
		       m_pModule->GetMaxHits());

  m_pModule->SetMaxHits(CAENV890::HitMaxNoLimit);
  CPPUNIT_ASSERT_EQUAL(CAENV890::HitMaxNoLimit,
		       m_pModule->GetMaxHits());



}
/*!
   Test ability to check for data ready.
*/
void V890Test::testDataReady()
{
  u_long pattern(0x01020304);
  
  m_pModule->TestMode(true, pattern);
  
  CPPUNIT_ASSERT(m_pModule->DataReady());

  u_long buffer[100];
  u_int nWords;

  nWords = m_pModule->Read(sizeof(buffer)/sizeof(u_long),
			   buffer);
  CPPUNIT_ASSERT_EQUAL(1, (int)nWords);
  CPPUNIT_ASSERT_EQUAL((int)pattern, (int)buffer[0]);

  m_pModule->TestMode(false);
  testReadEmpty();
  
}
