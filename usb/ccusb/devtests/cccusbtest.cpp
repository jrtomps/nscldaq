
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"

#include <Globals.cpp>
#include <CReadoutModule.h>
#include <string>
#include <iostream>

#define private public
#define protected public
#include <CCCUSBModule.h>
#undef protected
#undef private


// Fool the linker
//namespace Globals {
//  ::CConfiguration* pConfig;
//}


class cccusbtest : public CppUnit::TestFixture {
  public:
  CPPUNIT_TEST_SUITE(cccusbtest);
  CPPUNIT_TEST(setMixedBufferBits_0);
  CPPUNIT_TEST(setMixedBufferBits_1);
  CPPUNIT_TEST(setForceScalerDumpBits_0);
  CPPUNIT_TEST(setForceScalerDumpBits_1);
  CPPUNIT_TEST(setArbitrateBusBits_0);
  CPPUNIT_TEST(setArbitrateBusBits_1);
  CPPUNIT_TEST(setOptionalHeaderBits_0);
  CPPUNIT_TEST(setOptionalHeaderBits_1);
  CPPUNIT_TEST(setTriggerLatchBits_0);
  CPPUNIT_TEST(setTriggerLatchBits_1);
  CPPUNIT_TEST_SUITE_END();


private:
  CCCUSBModule*  m_pModule;
  CReadoutModule* m_pConfig;

public:
  void setUp() {
    m_pConfig = new CReadoutModule("testccusb", CCCUSBModule());
    m_pModule = static_cast<CCCUSBModule*>(m_pConfig->getHardwarePointer());
  }
  void tearDown() {
    delete m_pConfig;
  }
protected:
  void setMixedBufferBits_0();
  void setMixedBufferBits_1();

  void setForceScalerDumpBits_0();
  void setForceScalerDumpBits_1();

  void setArbitrateBusBits_0();
  void setArbitrateBusBits_1();

  void setOptionalHeaderBits_0();
  void setOptionalHeaderBits_1();

  void setTriggerLatchBits_0();
  void setTriggerLatchBits_1();
};

CPPUNIT_TEST_SUITE_REGISTRATION(cccusbtest);

void cccusbtest::setMixedBufferBits_0()
{
  m_pConfig->configure("-mixedbuffer","on");

  uint16_t newreg = m_pModule->setMixedBufferBits(0);

  uint16_t expected = (1<<5);
  CPPUNIT_ASSERT_EQUAL(expected, newreg);
}

void cccusbtest::setMixedBufferBits_1()
{
  uint16_t newreg = m_pModule->setMixedBufferBits(0);

  uint16_t expected = 0;
  CPPUNIT_ASSERT_EQUAL(expected, newreg);
}

void cccusbtest::setForceScalerDumpBits_0()
{
  m_pConfig->configure("-forcescalerdump","on");

  uint16_t newreg = m_pModule->setForceScalerDumpBits(0);

  uint16_t expected = (1<<6);
  CPPUNIT_ASSERT_EQUAL(expected, newreg);
}

void cccusbtest::setForceScalerDumpBits_1()
{
  uint16_t newreg = m_pModule->setForceScalerDumpBits(0);

  uint16_t expected = 0;
  CPPUNIT_ASSERT_EQUAL(expected, newreg);
}


void cccusbtest::setArbitrateBusBits_0()
{
  m_pConfig->configure("-arbitratebus","on");

  uint16_t newreg = m_pModule->setArbitrateBusBits(0);

  uint16_t expected = (1<<12);
  CPPUNIT_ASSERT_EQUAL(expected, newreg);
}

void cccusbtest::setArbitrateBusBits_1()
{
  uint16_t newreg = m_pModule->setArbitrateBusBits(0);

  uint16_t expected = 0;
  CPPUNIT_ASSERT_EQUAL(expected, newreg);
}


void cccusbtest::setOptionalHeaderBits_0()
{
  m_pConfig->configure("-optionalheader","on");

  uint16_t newreg = m_pModule->setOptionalHeaderBits(0);

//  uint16_t expected = (1<<8);
  uint16_t expected = 0;  // we don't support the option so just this is 
                          // a no-op
  CPPUNIT_ASSERT_EQUAL(expected, newreg);
}

void cccusbtest::setOptionalHeaderBits_1()
{
  uint16_t newreg = m_pModule->setOptionalHeaderBits(0);

  uint16_t expected = 0;
  CPPUNIT_ASSERT_EQUAL(expected, newreg);
}

void cccusbtest::setTriggerLatchBits_0()
{
  m_pConfig->configure("-triggerlatch","on");

  uint16_t newreg = m_pModule->setTriggerLatchBits(0);

  uint16_t expected = (1<<4);
  CPPUNIT_ASSERT_EQUAL(expected, newreg);
}

void cccusbtest::setTriggerLatchBits_1()
{
  uint16_t newreg = m_pModule->setTriggerLatchBits(0);

  uint16_t expected = 0;
  CPPUNIT_ASSERT_EQUAL(expected, newreg);
}
