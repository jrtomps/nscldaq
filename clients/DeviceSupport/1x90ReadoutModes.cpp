// Template for a test suite.

#include <config.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include "CCAENV1x90.h"
#include "DesignByContract.h"
#include <vector>

#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif


using namespace DesignByContract;

extern long ModuleBase;

class ReadoutModes : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(ReadoutModes);
  CPPUNIT_TEST(ChipEncapsulateTest);
  CPPUNIT_TEST(MaxHitTest);
  CPPUNIT_TEST(ErrorEnableTest);
  CPPUNIT_TEST(L1CacheTest);
  CPPUNIT_TEST(IndividualEnableTest);
  CPPUNIT_TEST(EnableAllTest);
  CPPUNIT_TEST(MaskEnableTest);
  CPPUNIT_TEST(ChipEnableTest);
  CPPUNIT_TEST_SUITE_END();


private:
  CCAENV1x90* m_pModule;

  bool isEnabled(unsigned int nChannel);
  void TestChip(unsigned int nChip); // Test 1 chip's chip enable.

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
  void ChipEncapsulateTest();
  void MaxHitTest();
  void ErrorEnableTest();
  void L1CacheTest();
  void IndividualEnableTest();
  void EnableAllTest();
  void MaskEnableTest();
  void ChipEnableTest();
};

CPPUNIT_TEST_SUITE_REGISTRATION(ReadoutModes);

// utility:

bool 
isSet(vector<unsigned short> masks,unsigned int bit)
{
  unsigned int nMask = bit / 16; // 16 bits per mask.
  unsigned int nBit  = bit % 16;

  if(nMask >= masks.size()) {
    throw string("isSet - bit number too big");
  }

  return (masks[nMask] & (1 << nBit)) != 0;
}

void
Set(vector<unsigned short>& masks, unsigned int bit)
{
  unsigned int nMask = bit / 16;
  unsigned int nBit  = bit % 16;

  if(nMask >= masks.size()) {
    throw string("isSet - bit number too big");
  }

  masks[nMask] |= 1 << nBit;
  
}
void
Clear(vector<unsigned short>& masks, unsigned int bit)
{
  unsigned int nMask = bit / 16;
  unsigned int nBit  = bit % 16;

  if(nMask >= masks.size()) {
    throw string("isSet - bit number too big");
  }
  masks[nMask] &= ~(1 << nBit);

}

bool
ReadoutModes::isEnabled(unsigned int nChannel)
{
  vector<unsigned short> masks;

  m_pModule->GetChannelEnables(masks);
  return isSet(masks, nChannel);
}

// Test ability to turn on/off chip encapsulation:

void
ReadoutModes::ChipEncapsulateTest()
{
  m_pModule->EnableTDCEncapsulation();
  ASSERT(m_pModule->isTDCEncapsulationOn());

  m_pModule->DisableTDCEncapsulation();
  ASSERT(!m_pModule->isTDCEncapsulationOn());
}
/// Test various max hit counts.


static const CCAENV1x90::HitMax hitvalues[] = {
  CCAENV1x90::HITS_0,
  CCAENV1x90::HITS_1,
  CCAENV1x90::HITS_2,
  CCAENV1x90::HITS_4,
  CCAENV1x90::HITS_8,
  CCAENV1x90::HITS_16,
  CCAENV1x90::HITS_32,
  CCAENV1x90::HITS_64,
  CCAENV1x90::HITS_128,
  CCAENV1x90::HITS_UNLIMITED };
static const int nHitMaxes = sizeof(hitvalues)/sizeof(CCAENV1x90::HitMax);

void
ReadoutModes::MaxHitTest()
{
  for(int i =0; i < nHitMaxes; i++) {
    m_pModule->SetMaxHitsPerEvent(hitvalues[i]);
    EQ(hitvalues[i], m_pModule->GetMaxHitsPerEvent());
  }
}

static const unsigned short ErrorPatterns [] = {
  CCAENV1x90::ERR_VERNIER,
  CCAENV1x90::ERR_SELECT,
  CCAENV1x90::ERR_L1PARITY,
  CCAENV1x90::ERR_TFIFOPARITY,
  CCAENV1x90::ERR_MATCHERROR,
  CCAENV1x90::ERR_RFIFOPARITY,
  CCAENV1x90::ERR_RDOSTATE,
  CCAENV1x90::ERR_SUPPARITY,
  CCAENV1x90::ERR_CTLPARITY,
  CCAENV1x90::ERR_JTAGPARITY,
  CCAENV1x90::ERR_VERNIER    | CCAENV1x90::ERR_L1PARITY    |
  CCAENV1x90::ERR_MATCHERROR | CCAENV1x90::ERR_RDOSTATE    |
  CCAENV1x90::ERR_CTLPARITY,
  CCAENV1x90::ERR_SELECT     | CCAENV1x90::ERR_TFIFOPARITY |
  CCAENV1x90::ERR_RFIFOPARITY| CCAENV1x90::ERR_SUPPARITY   |
  CCAENV1x90::ERR_JTAGPARITY
};
static const int nErrorPatterns = sizeof(ErrorPatterns)/sizeof(unsigned short);

void
ReadoutModes::ErrorEnableTest()
{
  for(int i =0; i < nErrorPatterns; i++) {
    m_pModule->SetErrorEnables(ErrorPatterns[i]);
    EQ(ErrorPatterns[i], m_pModule->GetErrorEnables());
  }
}
// Test the L1 cache size control functions::
//
static const CCAENV1x90::L1Size l1sizes[] = {
  CCAENV1x90::L1_2wds,
  CCAENV1x90::L1_4wds,
  CCAENV1x90::L1_8wds,
  CCAENV1x90::L1_16wds,
  CCAENV1x90::L1_32wds,
  CCAENV1x90::L1_64wds,
  CCAENV1x90::L1_128wds,
  CCAENV1x90::L1_256wds };
static const int nL1Sizes = sizeof(l1sizes)/sizeof(CCAENV1x90::L1Size);

void
ReadoutModes::L1CacheTest() 
{
  for(int i = 0; i < nL1Sizes; i++) {
    m_pModule->SetL1Size(l1sizes[i]);
    EQ(l1sizes[i], m_pModule->GetL1Size());
  }
}

// Test individual channel enables.

void
ReadoutModes::IndividualEnableTest()
{
  unsigned int nChannels = m_pModule->getChannelCount();

  // This runs O(n^2) in the channel count... hopefully not too bad for
  // the 128 channel models.


  m_pModule->DisableAllChannels(); // Nothing on now.

  // Enable one at a time:

  for(unsigned short i =0; i < nChannels; i++) {
    m_pModule->EnableChannel(i);
    for(unsigned short j = 0; j < nChannels; j++) {
      if (j == i) {
	ASSERT(isEnabled(j));
      } 
      else {
	ASSERT(!isEnabled(j));
      }
    }
    m_pModule->DisableChannel(i);
    for(unsigned short j = 0; j < nChannels; j++) {
      ASSERT(!isEnabled(i));
    }
  }

  // Now enable all one at a time and disable all one at a time.

  for(unsigned short i =0; i < nChannels; i++) {
    m_pModule->EnableChannel(i);
    for(unsigned short j = 0; j < nChannels; j++) {
      if(j <= i) {
	ASSERT(isEnabled(j));
      } 
      else {
	ASSERT(!isEnabled(j));
      }
    }
  }
  // Bad channels should throw.

  EXCEPTION(m_pModule->EnableChannel(nChannels),
	    Require);
  
}

// Test global enable.

void
ReadoutModes::EnableAllTest()
{
  m_pModule->DisableAllChannels();
  for(int i = 0; i < m_pModule->getChannelCount(); i++) {
    ASSERT(!isEnabled(i));
  }
  m_pModule->EnableAllChannels();
  for(int i = 0; i < m_pModule->getChannelCount(); i++) {
    ASSERT(isEnabled(i));
  }
}
// Test enable/disable via masks:

void
ReadoutModes::MaskEnableTest()
{
  unsigned int nChannels = m_pModule->getChannelCount(); 
  vector<unsigned short> mask;

  // Create the channel mask vector... initially all zero.

  for(int i = 0; i < nChannels/16; i++) {
    mask.push_back(0);
  }
  // Now roll a bit through the mask:
  
  for(int i =0; i < nChannels; i++) {
    Set(mask, i);		// Enable the channel.
    m_pModule->SetChannelEnables(mask);
    for(int j = 0; j < nChannels; j++) {
      if(i == j) {
	ASSERT(isEnabled(j));
      }
      else {
	ASSERT(!isEnabled(j));
      }
    }
    Clear(mask,i);
  }
  // Now shift bits into the mask... the prior loops should
  // have left all bits in mask clear.

  for(int i=0; i < nChannels; i++) {
    Set(mask, i);
    m_pModule->SetChannelEnables(mask);
    for(int j=0; j < nChannels; j++) {
      if(j <= i) {
	ASSERT(isEnabled(j));
      }
      else {
	ASSERT(!isEnabled(j));
      }
    }
  }
  // All on:

  for(int i=0; i < mask.size(); i++) {
    mask[i] = 0xffff;
  }
  m_pModule->SetChannelEnables(mask);
  for(int i =0; i < nChannels; i++) {
    ASSERT(isEnabled(i));
  }

  // All off:
 
  for(int i = 0; i < mask.size(); i++) {
    mask[i] = 0;
  }
  m_pModule->SetChannelEnables(mask);
  for(int i = 0; i < nChannels; i++) {
    ASSERT(!isEnabled(i));
  }

  // Even bits on:

  for(int i = 0; i < nChannels; i++) {
    if (!(i%2)) {
      Set(mask, i);
    }
    else {
      Clear(mask, i);
    }
  }
  m_pModule->SetChannelEnables(mask);

  for(int i =0; i < nChannels; i++) {
    if (!(i%2)) {
      ASSERT(isEnabled(i));
    }
    else {
      ASSERT(!isEnabled(i));
    }
  }

  // Odd bits on:

  for(int i =0; i < nChannels; i++) {
    if(i%2) {
      Set(mask, i);
    }
    else {
      Clear(mask, i);
    }
  }
  m_pModule->SetChannelEnables(mask);

  for(int i = 0; i < nChannels; i++) {
    if(i%2) {
      ASSERT(isEnabled(i));
    }
    else {
      ASSERT(!isEnabled(i));
    }
  }
  
}
// Utility to check the enables for a single chip:

void ReadoutModes::TestChip(unsigned int nChip)
{

  // 1 bit at a time.

  for (int i =0; i < 32; i++) {
    unsigned int mask = 1 << i;		// Single channel enabled on the chip.
    m_pModule->SetChipEnables(nChip, mask);
    unsigned int outmask = m_pModule->GetChipEnables(nChip);
    EQ(mask, outmask);
  }
  // Shifting bits in:

  unsigned int mask = 0;
  for(int i =0; i < 32; i++) {
    mask |= 1 << i;		// Add a channel...
    m_pModule->SetChipEnables(nChip, mask);
    unsigned int outmask = m_pModule->GetChipEnables(nChip);
    EQ(mask, outmask);
  }

  // Even channels:

  m_pModule->SetChipEnables(nChip, 0x5a5a5a5a);
  EQ(0x5a5a5a5aU, m_pModule->GetChipEnables(nChip));
  
  // Odd channels.

  m_pModule->SetChipEnables(nChip, 0xa5a5a5a5);
  EQ(0xa5a5a5a5U, m_pModule->GetChipEnables(nChip));

  // All on:

  m_pModule->SetChipEnables(nChip, 0xffffffff);
  EQ(0xffffffffU, m_pModule->GetChipEnables(nChip));

  // All off:

  m_pModule->SetChipEnables(nChip, 0);
  EQ(0U, m_pModule->GetChipEnables(nChip));
     
}
//  Test ability to control enables a chip at a time.
//  Note that regardless of the number of channels on the board,
//  Each chip has 32 enables... in the 1290, some are ganged
//  together for 25ps mode, but in 100ps mode, they are not
//  only some are used.  We'll put the  module into 100ps mode
//  and then treat it as if it's an 1190 for the purposes of this test.
//
void
ReadoutModes::ChipEnableTest()
{
  unsigned int nChips  = m_pModule->getChipCount();
  m_pModule->SetIndividualLSB(CCAENV1x90::Res_100ps);

  for(int i = 0; i < nChips; i++) {
    TestChip(i);
  }
}
