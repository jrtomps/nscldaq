
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"

#include <Globals.cpp>
#include <CReadoutModule.h>
#include <CMockVMUSB.h>
#include <string>

#define private public
#define protected public
#include <CVMUSBControl.h>
#undef protected
#undef private


// Fool the linker
//namespace Globals {
//  ::CConfiguration* pConfig;
//}


class cvmusbtest : public CppUnit::TestFixture {
  public:
  CPPUNIT_TEST_SUITE(cvmusbtest);
  CPPUNIT_TEST(testConfigureGlobalMode0);
  CPPUNIT_TEST(testConfigureGlobalMode1);
  CPPUNIT_TEST(testConfigureGlobalMode2);
  CPPUNIT_TEST(testConfigureGlobalMode3);
  CPPUNIT_TEST(testConfigureGlobalMode4);
  CPPUNIT_TEST(testConfigureGlobalMode5);
//  CPPUNIT_TEST(testConfigureGlobalMode6);
  CPPUNIT_TEST(testConfigureGlobalMode7);
  CPPUNIT_TEST(testConfigureEventsPerBuffer);
  CPPUNIT_TEST_SUITE_END();


private:
  CVMUSBControl*  m_pModule;
  CReadoutModule* m_pConfig;

public:
  void setUp() {
    m_pConfig = new CReadoutModule("testvmusb", CVMUSBControl());
    m_pModule = static_cast<CVMUSBControl*>(m_pConfig->getHardwarePointer());
  }
  void tearDown() {
    delete m_pConfig;
  }
protected:
  void testConfigureGlobalMode0();
  void testConfigureGlobalMode1();
  void testConfigureGlobalMode2();
  void testConfigureGlobalMode3();
  void testConfigureGlobalMode4();
  void testConfigureGlobalMode5();
  void testConfigureGlobalMode6();
  void testConfigureGlobalMode7();
  void testConfigureEventsPerBuffer();
};

CPPUNIT_TEST_SUITE_REGISTRATION(cvmusbtest);

 
// On creating a module, attaching it to a configuration should
// cause the right configuration entries and defaults to be made.
//
// Readout list executed immediately should return no data since there are
// no triggers.
//
void cvmusbtest::testConfigureEventsPerBuffer()
{
  std::string baseString = "20";
  m_pConfig->configure("-eventsperbuffer", baseString);

  CMockVMUSB ctlr;
  m_pModule->configureEventsPerBuffer(ctlr);

  std::vector<std::string> ops = ctlr.getOperationRecord();
  CPPUNIT_ASSERT_EQUAL(std::string("writeEventsPerBuffer(0x00000014)"), ops.at(0));

}

void cvmusbtest::testConfigureGlobalMode0()
{
  m_pConfig->configure("-bufferlength", "13k");
  m_pConfig->configure("-forcescalerdump", "yes");
  m_pConfig->configure("-mixedbuffers", "no");
  m_pConfig->configure("-busreqlevel", "3");

  CMockVMUSB ctlr;
  m_pModule->Initialize(ctlr);

  std::vector<std::string> ops = ctlr.getOperationRecord();


  // this should produce
  // 0x3020
  CPPUNIT_ASSERT_EQUAL(std::string("writeGlobalMode(0x00003040)"), ops.at(7));
}

// Test that the default value 
void cvmusbtest::testConfigureGlobalMode1()
{
  CMockVMUSB ctlr;
  m_pModule->configureGlobalMode(ctlr);

  // this should produce
  // 0x4000
  // -busreqlevel 4
  // -flushscaler off
  // -mixedbuffers off
  // -spanbuffer off
  // -optional header off
  // -align32 off (not supported)

  std::vector<std::string> ops = ctlr.getOperationRecord();
  CPPUNIT_ASSERT_EQUAL(std::string("writeGlobalMode(0x00004000)"), ops.at(1));
}

// Test the various buffer sizes
void cvmusbtest::testConfigureGlobalMode2()
{
  CMockVMUSB ctlr;
  const std::vector<std::string>& ops = ctlr.getOperationRecord();

  m_pConfig->configure("-busreqlevel", "0");
  m_pConfig->configure("-bufferlength", "13k");
  m_pModule->configureGlobalMode(ctlr);

  CPPUNIT_ASSERT_EQUAL(std::string("writeGlobalMode(0x00000000)"), ops.at(1));

  m_pConfig->configure("-bufferlength", "8k");
  m_pModule->configureGlobalMode(ctlr);
  CPPUNIT_ASSERT_EQUAL(std::string("writeGlobalMode(0x00000001)"), ops.at(4));

  m_pConfig->configure("-bufferlength", "4k");
  m_pModule->configureGlobalMode(ctlr);
  CPPUNIT_ASSERT_EQUAL(std::string("writeGlobalMode(0x00000002)"), ops.at(7));

  m_pConfig->configure("-bufferlength", "2k");
  m_pModule->configureGlobalMode(ctlr);
  CPPUNIT_ASSERT_EQUAL(std::string("writeGlobalMode(0x00000003)"), ops.at(10));

  m_pConfig->configure("-bufferlength", "1k");
  m_pModule->configureGlobalMode(ctlr);
  CPPUNIT_ASSERT_EQUAL(std::string("writeGlobalMode(0x00000004)"), ops.at(13));

  m_pConfig->configure("-bufferlength", "512");
  m_pModule->configureGlobalMode(ctlr);
  CPPUNIT_ASSERT_EQUAL(std::string("writeGlobalMode(0x00000005)"), ops.at(16));

  m_pConfig->configure("-bufferlength", "256");
  m_pModule->configureGlobalMode(ctlr);
  CPPUNIT_ASSERT_EQUAL(std::string("writeGlobalMode(0x00000006)"), ops.at(19));

  m_pConfig->configure("-bufferlength", "128");
  m_pModule->configureGlobalMode(ctlr);
  CPPUNIT_ASSERT_EQUAL(std::string("writeGlobalMode(0x00000007)"), ops.at(22));

  m_pConfig->configure("-bufferlength", "64");
  m_pModule->configureGlobalMode(ctlr);
  CPPUNIT_ASSERT_EQUAL(std::string("writeGlobalMode(0x00000008)"), ops.at(25));

  m_pConfig->configure("-bufferlength", "evtcount");
  m_pModule->configureGlobalMode(ctlr);
  CPPUNIT_ASSERT_EQUAL(std::string("writeGlobalMode(0x00000009)"), ops.at(28));
}

// Test span buffer 
void cvmusbtest::testConfigureGlobalMode3()
{
  CMockVMUSB ctlr;
  const std::vector<std::string>& ops = ctlr.getOperationRecord();

  m_pConfig->configure("-busreqlevel", "0");
  m_pConfig->configure("-spanbuffers", "on");
  m_pModule->configureGlobalMode(ctlr);

  CPPUNIT_ASSERT_EQUAL(std::string("writeGlobalMode(0x00000010)"), ops.at(1));

}

// Test span buffer 
void cvmusbtest::testConfigureGlobalMode4()
{
  CMockVMUSB ctlr;
  const std::vector<std::string>& ops = ctlr.getOperationRecord();

  m_pConfig->configure("-mixedbuffers", "on");
  m_pModule->configureGlobalMode(ctlr);

  CPPUNIT_ASSERT_EQUAL(std::string("writeGlobalMode(0x00004020)"), ops.at(1));

}

// Test flush scalers
void cvmusbtest::testConfigureGlobalMode5()
{
  CMockVMUSB ctlr;
  const std::vector<std::string>& ops = ctlr.getOperationRecord();

  m_pConfig->configure("-forcescalerdump", "on");
  m_pModule->configureGlobalMode(ctlr);

  CPPUNIT_ASSERT_EQUAL(std::string("writeGlobalMode(0x00004040)"), ops.at(1));

}

// Test optional header 
void cvmusbtest::testConfigureGlobalMode6()
{
  CMockVMUSB ctlr;
  const std::vector<std::string>& ops = ctlr.getOperationRecord();

  m_pConfig->configure("-optionalheader", "on");
  m_pModule->configureGlobalMode(ctlr);

  CPPUNIT_ASSERT_EQUAL(std::string("writeGlobalMode(0x00004100)"), ops.at(1));
}

// Test optional header 
void cvmusbtest::testConfigureGlobalMode7()
{
  CMockVMUSB ctlr;
  const std::vector<std::string>& ops = ctlr.getOperationRecord();

  m_pConfig->configure("-busreqlevel", "0");
  m_pModule->configureGlobalMode(ctlr);

  CPPUNIT_ASSERT_EQUAL(std::string("writeGlobalMode(0x00000000)"), ops.at(1));

  m_pConfig->configure("-busreqlevel", "1");
  m_pModule->configureGlobalMode(ctlr);
  CPPUNIT_ASSERT_EQUAL(std::string("writeGlobalMode(0x00001000)"), ops.at(4));

  m_pConfig->configure("-busreqlevel", "2");
  m_pModule->configureGlobalMode(ctlr);
  CPPUNIT_ASSERT_EQUAL(std::string("writeGlobalMode(0x00002000)"), ops.at(7));

  m_pConfig->configure("-busreqlevel", "3");
  m_pModule->configureGlobalMode(ctlr);
  CPPUNIT_ASSERT_EQUAL(std::string("writeGlobalMode(0x00003000)"), ops.at(10));

  // .... they 4 - 6 are the same

  m_pConfig->configure("-busreqlevel", "7");
  m_pModule->configureGlobalMode(ctlr);
  CPPUNIT_ASSERT_EQUAL(std::string("writeGlobalMode(0x00007000)"), ops.at(13));

  CPPUNIT_ASSERT_THROW( m_pConfig->configure("-busreqlevel", "9"), std::string);
}
