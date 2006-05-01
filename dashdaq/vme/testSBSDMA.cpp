// Template for a test suite.

#include <config.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include "CSBSVMEInterface.h"
#include "CSBSVmeDMATransfer.h"
#include <iostream>

#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif


class testSBSDMA : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(testSBSDMA);
  CPPUNIT_TEST(dmaIO);
  CPPUNIT_TEST_SUITE_END();


private:

public:
  void setUp() {
    // Note if tests are added, we need to include 
    // a first time only thingy as with testSBSInterface.cpp e.g.

    cerr << "\nWARNING: The tests in testSBSDMA.cpp require:\n";
    cerr << "   1. An SBS PCI/VME interface connected to an online crate\n";
    cerr << "   2. A24 Memory in that crate in the range 0x500000-0x5fffff\n";
    cerr << "If these conditions are not met, tests will fail\n";
  }
  void tearDown() {
  }
protected:
  void dmaIO();
};

CPPUNIT_TEST_SUITE_REGISTRATION(testSBSDMA);

static  long patterns[0x100000/sizeof(long)];
static   long readvalues[0x100000/sizeof(long)];


void testSBSDMA::dmaIO() 
{
  CSBSVMEInterface interface(0);

  bt_desc_t handle = static_cast<bt_desc_t>(interface.getDeviceHandle());

  CSBSVmeDMATransfer xfer(handle, 
			  0x39, CVMEInterface::TW_32,
			  0x500000, 0x100000);


  for(int i =0; i < sizeof(patterns)/sizeof(long); i++) {
    patterns[i] = i;
  }
  size_t xferred = xfer.Write(patterns);
  EQMSG("writecount", static_cast<size_t>(0x100000), xferred);

  xferred = xfer.Read(readvalues);
  EQMSG("readcount", static_cast<size_t>(0x100000), xferred);

  for (int i=0; i < 0x100000/sizeof(long); i++) {
    EQ(patterns[i], readvalues[i]);
  }
}
