// Template for a test suite.
#include <config.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include <CVMESubsystem.h>
#include <CSBSVMEInterface.h>
#include <CCESCBD8210.h>

#include <CBiRA1302CES8210.h>

using namespace std;


static Warning msg("bira1302 requires crate 1 on branch 0 powered up");
static Warning ks("bira1302 requires crate1 slot 16 stuffed with KS3821");

static const int crate= 1;
static const int branch= 0;
static const int vme= 0;
static const int slot=16;

class bira1302Test : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(bira1302Test);
  CPPUNIT_TEST(getBranch);
  CPPUNIT_TEST(inhibit);
  CPPUNIT_TEST(CZ);
  CPPUNIT_TEST(X);
  CPPUNIT_TEST(rdwr24);
  CPPUNIT_TEST(rdwr16);
  CPPUNIT_TEST_SUITE_END();



private:
  CVMESubsystem*    m_pSubsystem;
  CSBSVMEInterface* m_pVmeCrate;
  CCESCBD8210*      m_pBranch;
  CBiRA1302CES8210* m_pCrate;
public:
  void setUp() {
    m_pSubsystem = new CVMESubsystem;
    m_pVmeCrate  = new CSBSVMEInterface(vme);
    m_pSubsystem->installInterface(*m_pVmeCrate, true);
    m_pBranch    = new CCESCBD8210(vme,branch);
    m_pCrate    = new CBiRA1302CES8210(*m_pBranch, vme, crate);

  }
  void tearDown() {
    delete m_pBranch;		// deletes the crates in theory.
    delete m_pSubsystem;
  
  }


protected:
  void getBranch();
  void inhibit();
  void CZ();
  void X();
  void rdwr24();
  void rdwr16();
};

CPPUNIT_TEST_SUITE_REGISTRATION(bira1302Test);

void bira1302Test::getBranch() {
  EQ(m_pBranch, &(m_pCrate->getInterface()));
}

void bira1302Test::inhibit()
{
  m_pCrate->Inhibit();
  ASSERT(m_pCrate->isInhibited());
  m_pCrate->Uninhibit();
  ASSERT(!(m_pCrate->isInhibited()));
}
// After a C/Z, the inhibit should be on.

void
bira1302Test::CZ()
{
  m_pCrate->Uninhibit();
  m_pCrate->C();
  m_pCrate->Z();
  ASSERT(m_pCrate->isInhibited());
}


// Writing to the 3821 should give X.
//
void
bira1302Test::X()
{
  m_pCrate->write(slot, 17, 0, 0); //  Reset address pointer 0.
  ASSERT(m_pCrate->X());
}
// Test 24 bit read/write.
// The KS 3821 is a 24 bit wide memmory module:
//  F17.A0-3  Writes pointer register n
//  F1.A0-3   Reads pointer register n
//  F16.A0-3  Writes data to the word ponted to by pointer register n and increments.
//  F0.A0-3   Reads data from the word pointed to by pointer register n and increments.

void
bira1302Test::rdwr24()
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
bira1302Test::rdwr16()
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
