// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include "CCAENV1x90.h"
#include "CCAENV1x90Data.h"
#include "DesignByContract.h"
#include <unistd.h>

using namespace CCAENV1x90Data;

extern long ModuleBase;

class ReadTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(ReadTests);
  CPPUNIT_TEST(FIFOTest);
  CPPUNIT_TEST(TestReadNoHeaders);
  CPPUNIT_TEST(TestReadHeaders);
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
  void FIFOTest();
  void TestReadNoHeaders();
  void TestReadHeaders();
};

CPPUNIT_TEST_SUITE_REGISTRATION(ReadTests);

/*

Test procedure:

1.	Limit event size to 1 hit.
2.	Set triggger matching mode
3.	Set test mode with appropriate data word.\
4.	Do 16 SW triggers.
5.	Check FIFO for correctness
*/

void 
ReadTests::FIFOTest()
{
  m_pModule->SetMaxHitsPerEvent(CCAENV1x90::HITS_1);
  m_pModule->TriggerMatchMode();
  m_pModule->EnableTestMode( 0x123456U ); // Nice easy pattern.

  for(int i =0; i < 16; i++) {
    m_pModule->Trigger();	// Trigger the module for test data.
    usleep(100);
  }
  /*
    1.	FIFO status shows fifo ready , fifo not full.
    2.	FIFO count shows 16 events stored.
    3.	Event fifo can be read 16 times, 
        and event count field will increment between each read.
    4.	After 16 reads, fifo ready is clear, fifo count is zero.
   */
  ASSERT(m_pModule->isEventFIFOReady());
  ASSERT(!m_pModule->isEventFIFOFull());

  EQMSG("FIFO COunt", static_cast<unsigned short>(16), 
	              m_pModule->EventFIFOCount());

  // Get the first value:

  unsigned long  fifo       = m_pModule->ReadEventFIFO();
  unsigned short lastevent  = m_pModule->FIFOEventNumber(fifo);
  for(int i = 0; i < 15; i++) {	// only 15 entries left now:
    fifo = m_pModule->ReadEventFIFO();
    EQMSG("Event Number", ++lastevent, m_pModule->FIFOEventNumber(fifo));
  }
  ASSERT(!m_pModule->isEventFIFOReady());
}
// See if we can read an event in test mode.

static const CCAENV1x90::HitMax hitcode[] = {
  CCAENV1x90::HITS_1,
  CCAENV1x90::HITS_2,
  CCAENV1x90::HITS_4,
  CCAENV1x90::HITS_8,
  CCAENV1x90::HITS_16,
  CCAENV1x90::HITS_32,
  CCAENV1x90::HITS_64,
  CCAENV1x90::HITS_128
};
static const int hitwords[] = {
  1,2,4,8,16,32,64,128
};

static const int nHitvalues = sizeof(hitwords)/sizeof(int);

void 
ReadTests::TestReadNoHeaders()
{
  unsigned short nChips = m_pModule->getChipCount();

  m_pModule->TriggerMatchMode();
  m_pModule->EnableTestMode( 0x123456U ); // Nice easy pattern.
  m_pModule->DisableTDCEncapsulation();

  for(int i =0; i < nHitvalues; i++) {

    m_pModule->SetMaxHitsPerEvent(hitcode[i]);
    int nWords    = hitwords[i];
    unsigned long 
      nEventSize = nWords * nChips + 2; // hitwords/chip + header + trailer


    m_pModule->Trigger();	// Trigger the module for test data.
    usleep(nWords);		// Allow time for buffer transfer.

    // The fifo should be ready, but not full.
    
    ASSERT(m_pModule->isEventFIFOReady());
    ASSERT(!m_pModule->isEventFIFOFull());
    
  // The fifo should indicate an event with # hits * chips.+2.
    
    unsigned long fifo = m_pModule->ReadEventFIFO();
    EQMSG("event size",
	  nEventSize, 
	  static_cast<unsigned long>(m_pModule->FIFOWordCount(fifo)));
    ASSERT(m_pModule->DataReady()); // Data must be present.

    unsigned long *words = new unsigned long[nEventSize];
    m_pModule->ReadData(words, nEventSize);

    // Now check the format:


    // Global header for GEO 1.
    
    ASSERT(isGlobalHeader(words[0]));
    EQMSG("Slot h", 1U, BoardNumber(words[0]));
    

    // For each chip...

    unsigned long*p = &(words[1]);
    for(unsigned long chip =0; chip < nChips; chip++) {
      for(unsigned long word = 0; word < nWords; word++) { 
	ASSERT((*p & 0xf8000000) == 0x20000000); // Test output.
	EQMSG("chip", chip,
	      (*p >> 24) & 0x3);
	EQMSG("Data", 0x123456UL, (*p & 0xffffff));
	p++;
      } 
    }
      

    
    // Global trailer.

    ASSERT(isGlobalTrailer(words[nEventSize-1]));
    EQMSG("Size ",  nEventSize, EventSize(words[nEventSize-1]));
    EQMSG("Slot t", 1U, BoardNumber(words[nEventSize-1]));

    delete []words;
  }
}

// Same as above, but TDC headers are on.

void
ReadTests::TestReadHeaders()
{
  unsigned short nChips = m_pModule->getChipCount();

  m_pModule->TriggerMatchMode();
  m_pModule->EnableTestMode( 0x123456U ); // Nice easy pattern.
  m_pModule->EnableTDCEncapsulation();

  for(int i =0; i < nHitvalues; i++) {

    m_pModule->SetMaxHitsPerEvent(hitcode[i]);
    int nWords    = hitwords[i];

    // (Hitwords+chiphead+chiptrail)*nchips + global header + global trailer.
    unsigned long 
      nEventSize = (nWords+2) * nChips + 2;

    m_pModule->Trigger();	// Trigger the module for test data.
    usleep(nWords);		// Allow time for buffer transfer.

    // The fifo should be ready, but not full.
    
    ASSERT(m_pModule->isEventFIFOReady());
    ASSERT(!m_pModule->isEventFIFOFull());
    
  // The fifo should indicate an event with # hits * chips.+2.
    
    unsigned long fifo = m_pModule->ReadEventFIFO();
    EQMSG("event size",
	  nEventSize, 
	  static_cast<unsigned long>(m_pModule->FIFOWordCount(fifo)));
    ASSERT(m_pModule->DataReady()); // Data must be present.

    unsigned long *words = new unsigned long[nEventSize];
    m_pModule->ReadData(words, nEventSize);

    // Now check the format:


    // Global header for GEO 1.
    
    ASSERT(isGlobalHeader(words[0]));
    EQMSG("Slot h", 1U, BoardNumber(words[0]));
    

    // For each chip...

    unsigned long*p = &(words[1]);


    for(unsigned long chip =0; chip < nChips; chip++) {
      // TDC Header
      
      ASSERT(isTDCHeader(*p));
      EQMSG("Chip#", static_cast<unsigned int>(chip), TDCChip(*p));
      p++;
      
      // Test data

      for(unsigned long word = 0; word < nWords; word++) { 
	ASSERT((*p & 0xf8000000) == 0x20000000); // Test output.
	EQMSG("chip", chip,
	      (*p >> 24) & 0x3);
	EQMSG("Data", 0x123456UL, (*p & 0xffffff));
	p++;
      } 
      // TDC Trailer.

      ASSERT(isTDCTrailer(*p));
      EQMSG("Chip#", static_cast<unsigned int>(chip), TDCChip(*p));
      EQMSG("Size ", static_cast<short>(nWords+2), TDCWordCount(*p));
      p++;

    }

    
    // Global trailer.

    ASSERT(isGlobalTrailer(words[nEventSize-1]));
    EQMSG("Size ",  nEventSize, EventSize(words[nEventSize-1]));
    EQMSG("Slot t", 1U, BoardNumber(words[nEventSize-1]));

    delete []words;
  }
}
