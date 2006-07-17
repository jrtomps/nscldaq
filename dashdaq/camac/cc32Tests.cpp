// Template for a test suite.
#include <config.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include <CVMESubsystem.h>
#include <CSBSVMEInterface.h>
#include <CWienerVC32.h>

#include <CVC32CC32.h>

using namespace std;


static Warning msg("cc32Tests requires crate0 in vme 0 slot 16 with KS3821");

static const int crate= 0;
static const int branch= 0;
static const int vme= 0;
static const int slot=16;

class cc32Test : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(cc32Test);
  CPPUNIT_TEST(rdwr24);
  CPPUNIT_TEST(rdwr16);
  CPPUNIT_TEST_SUITE_END();



private:
  CVMESubsystem*    m_pSubsystem;
  CSBSVMEInterface* m_pVmeCrate;
  CWienerVC32*      m_pBranch;
  CVC32CC32*        m_pCrate;
public:
  void setUp() {
    m_pSubsystem = new CVMESubsystem;
    m_pVmeCrate  = new CSBSVMEInterface(vme);
    m_pSubsystem->installInterface(*m_pVmeCrate, true);
    m_pBranch    = new CWienerVC32(vme, 0xc00000);
    m_pCrate    = new  CVC32CC32(*m_pBranch);

  }
  void tearDown() {
    delete m_pBranch;		// deletes the crates in theory.
    delete m_pSubsystem;
  
  }


protected:
  void rdwr24();
  void rdwr16();
};

CPPUNIT_TEST_SUITE_REGISTRATION(cc32Test);


// Test 24 bit read/write.
// The KS 3821 is a 24 bit wide memmory module:
//  F17.A0-3  Writes pointer register n
//  F1.A0-3   Reads pointer register n
//  F16.A0-3  Writes data to the word ponted to by pointer register n and increments.
//  F0.A0-3   Reads data from the word pointed to by pointer register n and increments.

void
cc32Test::rdwr24()
{
  m_pCrate->write(slot, 17, 0, 0); // Reset pointer register 0.
  m_pCrate->write(slot, 17, 1, 0); //  and 1.

  // now write a rotating bit pattern of 1K elements 

  uint32_t bit = 1;
  for (int i =0; i < 1024; i++) {
    m_pCrate->write(slot, 16, 0, bit);
    bit = (bit << 1) & 0xffffff;
    if (bit == 0) bit =1;	// rotate.
  }

  // Read:

  bit = 1;
  for (int i=0; i < 1024; i++) {
    uint32_t datum = m_pCrate->read(slot, 0, 1); // Read through pointer 1.
    EQMSG("comparing", bit, datum);
    bit = (bit << 1) & 0xffffff;

    if (bit == 0) bit = 1;
  }
}
// 16 bit operations.
// Same as above but with 16 bit operations.. the bit rolls the opposite way.
//

void
cc32Test::rdwr16()
{
  m_pCrate->write(slot, 17, 0, 0); // Reset pointer register 0.
  m_pCrate->write(slot, 17, 1, 0); //  and 1.

  // now write a rotating bit pattern of 1K elements 

  uint32_t bit = 0x8000;
  for (int i =0; i < 1024; i++) {
    m_pCrate->write16(slot, 16, 0, bit);
    bit = (bit >> 1) & 0xffff;
    if (bit == 0) bit = 0x8000;	// rotate.
  }

  // Read:

  bit = 0x8000;
  for (int i=0; i < 1024; i++) {
    uint32_t datum = m_pCrate->read16(slot, 0, 1); // Read through pointer 1.
    EQMSG("comparing", bit, datum);
    bit = (bit >> 1) & 0xffff;

    if (bit == 0) bit = 0x8000;
  }
}
