// Template for a test suite.

#include <config.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include "CSIS3600.h"
#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif

class sis3600tests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(sis3600tests);
  CPPUNIT_TEST(Construction);
  CPPUNIT_TEST(LED);
  CPPUNIT_TEST(LatchMode);
  CPPUNIT_TEST(ExternalClear);
  CPPUNIT_TEST(FClearWindow);
  CPPUNIT_TEST(LatchTest);
  CPPUNIT_TEST(FIFOClear);
  CPPUNIT_TEST(Read1);
  CPPUNIT_TEST(Read);
  CPPUNIT_TEST_SUITE_END();


private:
  CSIS3600* m_pModule;
public:
  void setUp() {
    m_pModule = new CSIS3600(0x38383800);
  }
  void tearDown() {
    delete m_pModule;
  }
protected:
  void Construction();
  void LED();
  void LatchMode();
  void ExternalClear();
  void FClearWindow();
  void LatchTest();
  void Read1();
  void Read();
  void FIFOClear();

private:
  void LatchEvent();
};

CPPUNIT_TEST_SUITE_REGISTRATION(sis3600tests);

// Test construction:
//    Constructing a module at 0x38383800 (default address)
//    should work but constructing one at 0x10000000 should fail.
//
void sis3600tests::Construction()
{
  // Should work without exceptions:

  try {
    CSIS3600 module(0x38383800); // Crate = 0
  }
  catch (...) {			// Should not catch here.
    FAIL("Exception on construction of valid module!!");
  }

  // This should throw a string exception:

  bool failed = true;
  try {
    CSIS3600 module(0x10000000);
  }
  catch (string msg) {		//  Correct exception.
    failed = false;
  }
  catch (...) {			// Incorrect exception.
    FAIL("Bad construction threw wrong type of exception.");
  }
  if(failed) {
    FAIL("Construction of bad moudule did not throw an exception");
  }
  // Success.

}
/*
  Test that the led can be turned on and off and that
  the status register reflects this (assume that if this is consistent
  the led actually will go on/off)
*/
void
sis3600tests::LED()
{
  m_pModule->LightLed();
  ASSERT(m_pModule->isLedLit());


  m_pModule->ClearLed();
  ASSERT(!m_pModule->isLedLit());
}
/*
   Test that the module can be put in latch (not coincidence
   mode.
*/
void
sis3600tests::LatchMode()
{
  m_pModule->SetLatchMode();
  ASSERT(m_pModule->isLatchMode());

  m_pModule->SetCoincidenceMode();
  ASSERT(!m_pModule->isLatchMode());
}
/*
   Test the ability to enable external clears:
*/
void
sis3600tests::ExternalClear()
{
  m_pModule->EnableExternalClear();
  ASSERT(m_pModule->ExternalClearEnabled());
  
  m_pModule->DisableExternalClear();
  ASSERT(!m_pModule->ExternalClearEnabled());
  
}
/*
  Test the fast clear window setting:
*/
void 
sis3600tests::FClearWindow()
{
  // First test the legal window values:
  // these should:
  // - Not throw exceptions.
  // - Return values identical to what was set.
  // Note valid values are in the range [220 - 25720 ns].

  int i;
  for(i = 220; i <= 25720; i+=100 ) {
    try {
      m_pModule->SetFastClearWindow(i);
      EQ(i, m_pModule->GetFastClearWindow());
    }
    catch (string msg) {
      FAIL("Valid fast clear window set -> Exception");
    }
  }
  // Now test illegal window valuues:
  //  - too low
  //  - too high.
  //
  bool failed;
  try {
    failed = true;
    m_pModule->SetFastClearWindow(169);	// Above this rounds to ok.
  }
  catch (string msg) {
    failed = false;
  }
  catch (...) {
    FAIL("Invalid fast clear window threw wrong exception type");
  }
  if(failed) {
    FAIL("Invalid fast clear window set succeeded !!!");
  }

  try {
    failed = true; 
    m_pModule->SetFastClearWindow(25771); // Lower rounds down to ok.

  }
  catch(string msg) {
    failed = false;
  }
  catch(...) {
    FAIL("Invalid fast clear window set threw wrong exception type");
  }
  if(failed) {
    FAIL("Invalid fast clear window set succeeded!!");
  }
  // Resetting the module should reset the fast clear window to 220:

  m_pModule->Reset();
  EQ(220, m_pModule->GetFastClearWindow());

}
/* Utility to latch an event:
 */
void sis3600tests::LatchEvent()
{
  m_pModule->StartLatch();
  m_pModule->EndLatch();
  m_pModule->Clock();
}
/*
  Ensure that the software latch can make the fifo not empty.
*/
void
sis3600tests::LatchTest()
{
  ASSERT(!(m_pModule->DataReady())); // I think reset should clear

  m_pModule->SetCoincidenceMode();	        // Take all transitions.
  m_pModule->Enable();
  LatchEvent();

  ASSERT(m_pModule->DataReady());       // Now there should be data. 
}
/*  
  Ensure that clearing the fifo leaves the module with
  a fifo empty configuration.
*/

void
sis3600tests::FIFOClear()
{
  LatchTest();			// There's now data in the fifo.

  m_pModule->ClearData();	// Clear the fifo.

  ASSERT(!(m_pModule->DataReady()));
}
/*
  Test ability to read a single event.  Note
  that a single event consists of a longword of data
  containing the bits of a single latch transfer.
  
*/
void
sis3600tests::Read1()
{
  // First latch two words of test data:

  m_pModule->Reset();
  m_pModule->SetCoincidenceMode();
  m_pModule->Enable();

  for(int i =0; i < 2; i++) {	// The body of this loop
    LatchEvent();
  }

  unsigned long data;
  try {
    ASSERT(m_pModule->DataReady()); // data should be ready.
    
    data = m_pModule->Read();	// Read an event..
    
    ASSERT(m_pModule->DataReady()); // Should be another event..
    
    data = m_pModule->Read();
    ASSERT(!m_pModule->DataReady()); // no more data.
  }
  catch (string msg) {		// Should not fire.
    FAIL("Read threw exception even with data!");
  }

  // The next read should throw an exception:

  bool failed = true;
  try {
    data = m_pModule->Read();
  }
  catch (string msg) {
    failed = false;
  }
  catch (...) {
    FAIL("Empty read threw wrong exception type");
  }
  if(failed) {
    FAIL("Empty read did not throw an exception");
  }
  // passed!

}
/*
   Test multievent read
*/
void
sis3600tests::Read()
{
  m_pModule->Reset();
  m_pModule->SetCoincidenceMode();
  m_pModule->Enable();

  // Now 100 events:

  for(int i = 0; i < 100; i ++) {
    LatchEvent();
  }
  unsigned long data[30];
  unsigned int  nread;

  nread = m_pModule->Read(data, 30);	// Read 30 events.
  EQMSG("First 30", 30U, nread);

  nread = m_pModule->Read(data, 30);    // 30 more -> 60...
  EQMSG("Second 30", 30U, nread);

  nread = m_pModule->Read(data, 30);    // 30 more -> 90...
  EQMSG("Last full 30", 30U, nread);

  nread = m_pModule->Read(data, 30);    // can only read 10
  EQMSG("Partial read", 10U, nread);
}
