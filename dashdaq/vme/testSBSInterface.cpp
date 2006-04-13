// Template for a test suite.
#include <config.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include <iostream>
#include "CSBSVMEInterface.h"
#include "CSBSVmeException.h"
#include "CVMEAddressRange.h"

// Just in case the previous stuff does not get me the
// SBS includes.

#ifndef BT1003
#define BT1003
extern "C" {
#include <btapi.h>
}
#endif

#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif

class testSBSInterface : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(testSBSInterface);
  CPPUNIT_TEST(construct);
  CPPUNIT_TEST(capabilities);
  CPPUNIT_TEST(type);
  CPPUNIT_TEST(lock);
  CPPUNIT_TEST(range);
  CPPUNIT_TEST(parts);
  CPPUNIT_TEST(memsize);
  CPPUNIT_TEST(traces);
  CPPUNIT_TEST_SUITE_END();

protected:
  void construct();
  void capabilities();
  void type();
  void lock();
  void range();
  void parts();
  void memsize();
  void traces();

private:
  static bool warned;
  CSBSVMEInterface* m_pInterface;
public:
  void setUp() {
    if (!warned) {
      cerr << "\nWarning: the tests in testSBSInterface.cpp require:\n";
      cerr << "   1. An SBS PCI/VME interface connected to an online VME crate\n";
      cerr << "   2.A24  Memory in that crate in the range 0x500000-0x5fffff\n";
      cerr << "If these conditions are not satisfied, tests will fail\n";      
      warned = true;
    }
    m_pInterface = new CSBSVMEInterface(0);
  }
  void tearDown() {
    delete m_pInterface;
  }

};

bool testSBSInterface::warned(false);

CPPUNIT_TEST_SUITE_REGISTRATION(testSBSInterface);

void testSBSInterface::construct() {
  bt_desc_t handle = static_cast<bt_desc_t>(m_pInterface->getDeviceHandle());
  
  // We defined ourselves to create an A32 device handle.
  // see if this is correct:

  bt_devdata_t amod;
  bt_error_t status = bt_get_info(handle, BT_INFO_PIO_AMOD,
				  &amod);
  EQMSG("status", BT_SUCCESS, status);
  EQMSG("amod",  (bt_devdata_t)0x0d, amod);

}

void testSBSInterface::capabilities()
{
  EQMSG("mapping", true, m_pInterface->canMap());
  EQMSG("list processor", false, m_pInterface->hasListProcessor());
  EQMSG("DMA", true, m_pInterface->hasDMABlockTransfer());
}

void testSBSInterface::type()
{
  EQ(string("SBSBit3PCIVME"), m_pInterface->deviceType());

}
void testSBSInterface::lock()
{
  // Evidently the lock is a counting lock so there's no
  // double test.

  bool thrown(false);
  try {
    m_pInterface->onLock();
  }
  catch(CSBSVmeException& e) {
    thrown = true;
  }
  ASSERT(!thrown);



  try {
    m_pInterface->onUnlock();
  } 
  catch (CSBSVmeException& e) {
    thrown = true;
  }
  EQMSG("unlock", false, thrown);

  
}
/*!
   Create an address range object and do some basic stuff.
*/
void testSBSInterface::range()
{
  CVMEAddressRange* pRange = m_pInterface->createAddressRange(0x39,
							      0x500000, 0x100);
  ASSERT(pRange);		// Got non zero pointer.

  // zero the range.

  for(int i =0; i < 0x100/sizeof(long); i++) {
    pRange->pokel(i, 0);
  }

  pRange->pokel(0x80/sizeof(long), 0xffffffff);	// Except for one long.

  for(int i=0; i < 0x100/sizeof(long); i++) {
    unsigned long r = pRange->peekl(i);
    if(i != 0x80/sizeof(long)) {
      EQ(0UL, r);
    } else {
      EQ(0xffffffffUL, r);
    }
  }

  delete pRange;
}

void testSBSInterface::parts()
{
  bt_devdata_t ifpn = m_pInterface->getRemotePartNumber();
  bt_desc_t handle  = static_cast<bt_desc_t>(m_pInterface->getDeviceHandle());
  bt_devdata_t dirpn;
  bt_error_t status = bt_get_info(handle, BT_INFO_REM_PN, &dirpn);
  EQMSG("status", BT_SUCCESS, status);
  EQMSG("rempno", dirpn, ifpn);

  ifpn   = m_pInterface->getLocalPartNumber();
  status = bt_get_info(handle, BT_INFO_LOC_PN, &dirpn);
  EQMSG("local status", BT_SUCCESS, status);
  EQMSG("localpno",     dirpn, ifpn);
}

void testSBSInterface::memsize()
{
  bt_devdata_t size = m_pInterface->getMemorySize();

  bt_desc_t handle  = static_cast<bt_desc_t>(m_pInterface->getDeviceHandle());
  bt_devdata_t dsize;
  bt_error_t status = bt_get_info(handle, BT_INFO_LM_SIZE, &dsize);

  EQMSG("status", BT_SUCCESS, status);
  EQMSG("size", dsize, size);

}

void testSBSInterface::traces()
{
  bt_devdata_t traceflags   = BT_TRC_DEFAULT;

  m_pInterface->setDriverTraces(traceflags);

  EQ(traceflags, m_pInterface->getDriverTraces());
}
