// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include <CVMUSB.h>
#include <CVMUSBReadoutList.h>

#include <stdio.h>


#include "Asserts.h"

using namespace std;


// If your memory is in a different place.. change the three lines
// below:

static Warning msg("vmeTests requires 1MB VME A24 memory at 0x500000");
static Warning blkmsg("vmeTests requries a few K A32 block xfer memory at 0x8000000");
static const uint32_t vmebase = 0x500000;
static const uint8_t  amod    = CVMUSBReadoutList::a24UserData;

static const uint32_t blockBase = 0x8000000;
static const uint8_t a32amod  = CVMUSBReadoutList::a32PrivData;
static const uint8_t blkamod  = CVMUSBReadoutList::a32PrivBlock;


class vmeTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(vmeTests);
  CPPUNIT_TEST(rdwr32);
  CPPUNIT_TEST(rdwr16);
  CPPUNIT_TEST(rdwr8);
  CPPUNIT_TEST(rdwr8);
  CPPUNIT_TEST(blockReadSingle);
  CPPUNIT_TEST(blockReadTwo);
  CPPUNIT_TEST(blockReadOdd);
  CPPUNIT_TEST(fifoTest);
  CPPUNIT_TEST_SUITE_END();

private:
  struct usb_device*   m_dev;
  CVMUSB*  m_pInterface;
public:
  void setUp() {
    vector<struct usb_device*> devices = CVMUSB::enumerate();
    if (devices.size() == 0) {
      cerr << " NO USB interfaces\n";
      exit(0);
    }
    m_pInterface = new CVMUSB(devices[0]);
  }
  void tearDown() {
    delete m_pInterface;
  }
protected:
  void rdwr32();
  void rdwr16();
  void rdwr8();
  void blockReadSingle();
  void blockReadTwo();
  void blockReadOdd();
  void fifoTest();
};

CPPUNIT_TEST_SUITE_REGISTRATION(vmeTests);


// 32 bit read/writes (can't test one without the other.
// We're going to roll a bit around the 32 bits.
void vmeTests::rdwr32() {
  uint32_t bit = 1;
  while (bit) {

    m_pInterface->vmeWrite32(vmebase, amod, bit);
    uint32_t value;
    int status = m_pInterface->vmeRead32(vmebase, amod, &value);
    EQMSG("status", 0, status);
    EQMSG("value", bit, value);

    bit = bit << 1;
  }

  
}
// 16 bit reads/writes run the bit in the opposite direction of the
// 32 bit version.
void vmeTests::rdwr16() {
  uint16_t bit = 0x8000;
  while (bit) {
    m_pInterface->vmeWrite16(vmebase, amod, bit);
    uint16_t value;
    int status = m_pInterface->vmeRead16(vmebase, amod, &value);
    EQMSG("status", 0, status);
    EQMSG("value", bit, value);
    bit = bit >> 1;
  }
}
// 8 bit reads/writes.  there are few enough patterns that we can do
// all of them.

void vmeTests::rdwr8() {
  uint8_t pattern;
  for (int p = 0; p <= 255; p++) {
    pattern = p;
    m_pInterface->vmeWrite8(vmebase, amod, pattern);
    uint8_t value;
    int status = m_pInterface->vmeRead8(vmebase, amod, &value);
    EQMSG("status", 0, status);
    EQMSG("value", pattern, value);
  }
}
// For block read we need to test a few cases:
//    < 64 (less than one block) - is non MB
//    > 64 multiple of 64        - is MB
//    > 64 not multiple of 64    - MB + odd one left over.
// These separate into three separate tests with differing patterns.
//

void vmeTests::blockReadSingle()
{
  // Write 60 rotating bit longwords; these must be done
  // one long at a time since I have not implemented block writes.
  // (not even sure they work.

  uint32_t bit = 1;
  uint32_t pattern[60];
  for (int i =0; i < 60; i++) {
    pattern[i] = bit;
    m_pInterface->vmeWrite32(blockBase+i*sizeof(uint32_t), a32amod, pattern[i]);
    bit = bit <<1;
    if (!bit) bit = 1;
  }

  // Read the block and compare

  size_t transferred;
  uint32_t rdblock[60];
  m_pInterface->vmeBlockRead(blockBase, blkamod, rdblock, 60, &transferred);

  EQMSG("transfercount", (size_t)60, transferred);
  for (int i= 0; i < 60; i++) {
    EQMSG("compare", pattern[i], rdblock[i]);
  }

}
// Block read of exactly 128 longs.  This is 2 complete blocks of  data
// and should kick in the multiblock capability without any odd blocks needed.
// - We'll use a counting pattern.
//
void vmeTests::blockReadTwo()
{
  const size_t patternSize(128);
  uint32_t     pattern[patternSize];

  for (int i = 0; i < patternSize; i++) {
    pattern[i] = i;
    m_pInterface->vmeWrite32(blockBase+i*sizeof(uint32_t), a32amod, pattern[i]);

  }
  uint32_t rdblock[patternSize];
  size_t   transferred;
  m_pInterface->vmeBlockRead(blockBase, blkamod, rdblock, patternSize, &transferred);
  EQMSG("transfercount", patternSize, transferred);
  for(int i =0; i < patternSize; i++) {
    EQMSG("compare", pattern[i], rdblock[i]);
  }
}
// Block read 180 longs.  This is 2 complete blocks + a bit.
// Should kick in multiblock transfer with a second transfer that
// deals with the partial block.
// - Count down pattern is used.
//
void vmeTests::blockReadOdd()
{
  const size_t patternSize(180);
  uint32_t     pattern[patternSize];

  for (int i = patternSize; i > 0; i--) {
    pattern[i] = i;
    m_pInterface->vmeWrite32(blockBase+i*sizeof(uint32_t), a32amod, pattern[i]);
  }
  
  uint32_t rdblock[patternSize];
  size_t   transferred;
  m_pInterface->vmeBlockRead(blockBase, blkamod, rdblock, patternSize, &transferred);
  EQMSG("Transfer count", patternSize, transferred);
  for (int i =0; i < patternSize; i++) {
    EQMSG("data compare", pattern[i], rdblock[i]);
  }
}
//  Fifo reads just read the memory without autoinc.  uh... since the
//  Board is responsible for handling addresses within a block what we will
//  do is write 180 longs to memory, we will then read 180 longs with
//  FIFO mode. What I >think< we should see is 0..63,0..63,0..51
//
void vmeTests::fifoTest()
{
  const size_t patternSize(180);
  uint32_t     pattern[patternSize];
  for (int i =0; i < patternSize; i ++) {
    pattern[i] = i;
    m_pInterface->vmeWrite32(blockBase+i*sizeof(uint32_t), a32amod, pattern[i]);
  }
  uint32_t rdblock[patternSize];
  size_t   transferred;
  m_pInterface->vmeFifoRead(blockBase, blkamod, rdblock, patternSize, &transferred);
  EQMSG("count", patternSize, transferred);

  for (int i=0; i < patternSize; i++) {
    char msg[100];
    sprintf(msg, "offset 0x%04x", i);
    EQMSG(msg, pattern[i % 64], rdblock[i]);
  }
}
