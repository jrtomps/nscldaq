// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include <usb.h>
#include <CVMUSB.h>
#include <vector>
#include <stdio.h>

using namespace std;

static Warning msg(string("regTests requires at least one VM-USB interface"));

class registerTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(registerTests);
  CPPUNIT_TEST(action);
  CPPUNIT_TEST(fwid);
  CPPUNIT_TEST(globmode);
  CPPUNIT_TEST(daqsettings);
  CPPUNIT_TEST(ledsrc);
  CPPUNIT_TEST(devsrc);
  CPPUNIT_TEST(dgg);
  CPPUNIT_TEST(countmask);
  CPPUNIT_TEST(vectors);
  CPPUNIT_TEST(bulksetup);
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
  void action();
  void fwid();
  void globmode();
  void daqsettings();
  void ledsrc();
  void devsrc();
  void dgg();
  void countmask();
  void vectors();
  void bulksetup();
};

CPPUNIT_TEST_SUITE_REGISTRATION(registerTests);

//write action register... can only determine that no throws happen.

void registerTests::action() {
  m_pInterface->writeActionRegister(0);	// this is really the only safe thing to write here.
				    
}

// Read firmware: can only output and hope no exceptions.

void registerTests::fwid()
{
  uint32_t fw = m_pInterface->readFirmwareID();
  cerr << endl << hex << "Firmware id: " << fw << dec << endl;

}
// Write 0's to the gobal mode register... all the 
// bits that count should match.. then turn them all  on too.
//
void registerTests::globmode()
{
  uint32_t validmask = 0x71ff;	// Bits that matter in the register.
  m_pInterface->writeGlobalMode(0);
  uint32_t mode = m_pInterface->readGlobalMode();
  EQMSG("wrote 0", 0U, mode & validmask);

  m_pInterface->writeGlobalMode(0xffffffff);
  mode   = m_pInterface->readGlobalMode();
  EQMSG("wrote 1s", validmask, (uint32_t)(validmask & mode));

}
// Daq settings can accept any bit patterns.

void registerTests::daqsettings()
{
  m_pInterface->writeDAQSettings(0xffffffff);
  uint32_t settings = m_pInterface->readDAQSettings();
  EQMSG("ones", (uint32_t)0xffffffff, settings);
 
  m_pInterface->writeDAQSettings(0);
  settings = m_pInterface->readDAQSettings();
  EQMSG("0's", (uint32_t)0, settings);
}
// LEDSrc has somed dead bits.

void registerTests::ledsrc()
{
  uint32_t usedBits = 0x1f1f1f1f;

  m_pInterface->writeLEDSource(0xffffffff);
  uint32_t value = m_pInterface->readLEDSource();
  EQMSG("ones", usedBits, (value & usedBits));

  m_pInterface->writeLEDSource(0);
  value = m_pInterface->readLEDSource();

  EQMSG("0's", (uint32_t)0, (value & usedBits));

}
// Dev src register also has some dead bits.

void registerTests::devsrc()
{
  uint32_t usedBits = 0x77771f1f;    // 0x77ff1f1f; // Hope I got that right.

  m_pInterface->writeDeviceSource(0xffffffff);
  uint32_t value = m_pInterface->readDeviceSource();
  EQMSG("ones", usedBits, (value & usedBits));

  m_pInterface->writeDeviceSource(0);
  value = m_pInterface->readDeviceSource();
  EQMSG("0's", (uint32_t)0, (value & usedBits));
}
// Three registers for the gate and delay register use all 32 bits.
// dgga, dggb, dggextended.

void registerTests::dgg()
{

  // DGG A control register.

  m_pInterface->writeDGG_A(0xffffffff);
  uint32_t value = m_pInterface->readDGG_A();
  EQMSG("DGGA 1's", (uint32_t)0xffffffff, value);

  m_pInterface->writeDGG_A(0);
  value = m_pInterface->readDGG_A();
  EQMSG("DGGA 0's", (uint32_t)0, value);

  // DGG B control register.

  m_pInterface->writeDGG_B(0xffffffff);
  value = m_pInterface->readDGG_B();
  EQMSG("DGGB 1's", (uint32_t)0xffffffff, value);

  m_pInterface->writeDGG_B(0);
  value = m_pInterface->readDGG_B();
  EQMSG("DGGB 0's", (uint32_t)0, value);

  // DGG Extended control register.

  m_pInterface->writeDGG_Extended(0xffffffff);
  value = m_pInterface->readDGG_Extended();
  EQMSG("Extended 1's", (uint32_t)0xffffffff, value);

  m_pInterface->writeDGG_Extended(0);
  value = m_pInterface->readDGG_Extended();
  EQMSG("Extended 0's", (uint32_t)0, value);
  
}
// Count mask : all bits are legitimate.

void registerTests::countmask()
{
  m_pInterface->writeCountExtractMask(0xffffffff);
  uint32_t value = m_pInterface->readCountExtractMask();
  EQMSG("ones",(uint32_t)0xffffffff, value);

  m_pInterface->writeCountExtractMask(0);
  value = m_pInterface->readCountExtractMask();
  EQMSG("zeroes", (uint32_t)0, value);
}
// Vectors: only some bits are used.
void registerTests::vectors()
{
  uint32_t usedBits = 0x77ff;

  for (int vec =1; vec<=4; vec++) {
    char msg[100];
    
    sprintf(msg, "vector %d, 1's", vec);
    m_pInterface->writeVector(vec, 0xffff);
    uint32_t value = m_pInterface->readVector(vec);
    EQMSG(msg, (uint32_t)usedBits, (value & usedBits));

    sprintf(msg, "vector %d 0's", vec);
    m_pInterface->writeVector(vec, 0);
    value = m_pInterface->readVector(vec);
    EQMSG(msg, (uint32_t)0, (value & usedBits));

  }
}

// only some bits of the bulk transfer setup register are used.

void registerTests::bulksetup()
{
  uint32_t usedBits = 0xfff;
  m_pInterface->writeBulkXferSetup(0xffffffff);
  uint32_t value = m_pInterface->readBulkXferSetup();
  EQMSG("1's", usedBits, (value & usedBits));

  m_pInterface->writeBulkXferSetup(0);
  value = m_pInterface->readBulkXferSetup();
  EQMSG("0's", (uint32_t)0, (value & usedBits));
}
