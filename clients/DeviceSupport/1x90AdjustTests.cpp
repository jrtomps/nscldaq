#include <config.h>

// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include "CCAENV1x90.h"
#include "DesignByContract.h"
#include <Iostream.h>

extern long ModuleBase;

class AdjustTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(AdjustTests);
  CPPUNIT_TEST(GlobalOffsetTest);
  CPPUNIT_TEST(ChannelOffsetTest);
  CPPUNIT_TEST(DelayLineTest);
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
  void GlobalOffsetTest();
  void ChannelOffsetTest();
  void DelayLineTest();
};

CPPUNIT_TEST_SUITE_REGISTRATION(AdjustTests);

// test global offset:  Note limits:
//   Coarse <= 0x7ff
//   Vernier < = 0x1f

void
AdjustTests::GlobalOffsetTest()
{
  // Rolling bits in coarse and vernier.

  for(int nv = 0; nv < 5; nv++) {
    for(int nc = 0; nc < 11; nc++) {
      unsigned short int coarse, vernier;
      m_pModule->SetGlobalOffset((1 << nc) & 0x7ff, (1 << nv) & 0x1f);
      m_pModule->ReadGlobalOffset(coarse, vernier);
      EQMSG("Coarse", static_cast<unsigned short>((1 << nc) & 0x7ff), 
	    coarse);
      EQMSG("Vernier", static_cast<unsigned short>((1 << nv) & 0x1f), 
	    vernier);
    }
  }

  // Shifting bits in coarse and vernier:

  unsigned short coarsein  = 0;
  unsigned short vernierin = 0;
  for(int c = 0; c < 11; c++) {
    for(int v =0; v < 5; v++) {
      unsigned short coarseout, vernierout;
      coarsein   |= (1 << c) & 0x7ff;
      vernierin  |= (1 << v) & 0x1f;
      m_pModule->SetGlobalOffset(coarsein, vernierin);
      m_pModule->ReadGlobalOffset(coarseout, vernierout);

      EQMSG("Coarse", coarsein, coarseout);
      EQMSG("Vernier", vernierin, vernierout);

    } 
  }
  // patterns:  We'll be even more exhaustive:
  // Than the test spec because I'm lazy
  // and want to loop this.

  unsigned short patterns[] = {
    0xffff, 0, 0x5a5a, 0xa5a5};

  int nPatterns = sizeof(patterns)/sizeof(unsigned short);

  for(int c = 0; c < nPatterns; c++) {
    for(int v = 0; v < nPatterns; v++) {
      unsigned short coarseout;
      unsigned short vernierout;

      m_pModule->SetGlobalOffset(patterns[c] & 0x7ff, patterns[v] & 0x1f);
      m_pModule->ReadGlobalOffset(coarseout, vernierout);
      
      EQMSG("Coarse", static_cast<unsigned short>(patterns[c] & 0x7ff) , coarseout);
      EQMSG("Vernier", static_cast<unsigned short>(patterns[v] & 0x1f),  vernierout);
    }
  }
}
//  Test set/get of channel offsets:

void
AdjustTests::ChannelOffsetTest()
{
  unsigned int nChannels = m_pModule->getChannelCount();
  for(int chan =0; chan < nChannels; chan++) {
    for(int value =0; value<=0xff; value++) {
      m_pModule->SetChannelOffset(chan, value);
      EQ(static_cast<unsigned short>(value), m_pModule->GetChannelOffset(chan));
    }
  }
}
//  Test ability to adjust the delay line rc circuit timing parameters.
//  NOTE: We don't save the calibrations so that the factory settings won't
//        get messed up!!!
//
void
AdjustTests::DelayLineTest()
{
  unsigned short nChip = m_pModule->getChipCount();
  for(unsigned short i=0; i < nChip; i++) {
    for(unsigned short tap1 = 0; tap1 < 8; tap1++) { // Each tap has 7 combinations.
      for(unsigned short tap2 = 0; tap2 < 8; tap2++) {
	for(unsigned short tap3 = 0; tap3 < 8; tap3++) {
	  for(unsigned short tap4=0; tap4 < 8; tap4++) {
	    m_pModule->CalibrateDelayLine(i,
					  tap1, tap2, tap3, tap4);
	    unsigned short to1, to2, to3, to4;
	    m_pModule->GetDelayLineCalibration(i,
					       to1, to2, to3,to4);
	    EQMSG("Tap1", tap1, to1);
	    EQMSG("Tap2", tap2, to2);
	    EQMSG("Tap3", tap3, to3);
	    EQMSG("Tap4", tap4, to4);
	  }
	}
      }
    }
  }
    				 
}
