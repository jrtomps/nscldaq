// It's not possible to have a direct test for 
// CVMEInterface.h since its abstract. We have a 
// test class named nullVMEInterface that is essentially
// a no-op VME interface that we can use to do our testing.

#include <config.h>
#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif


#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"

#include "nullVMEInterface.h"


class vmeInterface : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(vmeInterface);
  CPPUNIT_TEST(canMap);
  CPPUNIT_TEST(hasListProcessor);
  CPPUNIT_TEST(canDMA);
  CPPUNIT_TEST(getDeviceType);
  CPPUNIT_TEST(getHandle);
  CPPUNIT_TEST(addressRange);
  CPPUNIT_TEST(PIO);
  CPPUNIT_TEST(list);
  CPPUNIT_TEST(DMA);
  CPPUNIT_TEST(lockCallbacks);
  CPPUNIT_TEST_SUITE_END();


private:
  nullVMEInterface* m_pInterface;
public:
  void setUp() {
    m_pInterface = new nullVMEInterface;
  }
  void tearDown() {
    delete m_pInterface;
  }
protected:
  void canMap();
  void hasListProcessor();
  void canDMA();
  void getDeviceType();
  void getHandle();
  void addressRange();
  void PIO();
  void list();
  void DMA();
  void lockCallbacks();
};

CPPUNIT_TEST_SUITE_REGISTRATION(vmeInterface);

// canMap should return false.
void vmeInterface::canMap() {
  EQ(false, m_pInterface->canMap());
}
// hasListProcessor should return false.

void vmeInterface::hasListProcessor()
{
  EQ(false, m_pInterface->hasListProcessor());
}
// hasDMABlockTransfer should return false.

void vmeInterface::canDMA()
{
  EQ(false, m_pInterface->hasDMABlockTransfer());
}
// The device type string should be "NULL testing only"

void vmeInterface::getDeviceType()
{
  string type = m_pInterface->deviceType();
  EQ(string("NULL testing only"), type);
}
// Test handle should be null.

void vmeInterface::getHandle()
{
  EQ((void*)NULL, m_pInterface->getDeviceHandle());
}
// Address range should reutrn null...

void vmeInterface::addressRange()
{
  CVMEAddressRange* result = m_pInterface->createAddressRange(0, 0, 0x100);
  EQ(static_cast<CVMEAddressRange*>(NULL),  result);
}

// PIO device should be null.

void vmeInterface::PIO()
{
  CVMEPio* result = m_pInterface->createPioDevice();
  EQ(static_cast<CVMEPio*>(NULL), result);
}

// List should be null.

void vmeInterface::list()
{
  CVMEList* result = m_pInterface->createList();
  EQ(static_cast<CVMEList*>(NULL), result);
}
// DMA should be null too.

void vmeInterface::DMA()
{
  CVmeDMATransfer* result = m_pInterface->createDMATransfer(0, 
						    CVMEInterface::TW_32,
						    0x400000, 0x1000);
  EQ(static_cast<CVmeDMATransfer*>(NULL), result);
}
// Check that lock callbacks have desired effect.

void vmeInterface::lockCallbacks()
{
  ASSERT(!m_pInterface->locked());
  
  m_pInterface->onLock();

  ASSERT(m_pInterface->locked());

  m_pInterface->onUnlock();
  ASSERT(!m_pInterface->locked());
}
