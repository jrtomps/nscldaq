// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include <usb.h>
#include <CVMUSB.h>
#include <vector>

using namespace std;

static Warning msg(string("regTests requires at least one VM-USB interface"));

class registerTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(registerTests);
  //  CPPUNIT_TEST(action);
  CPPUNIT_TEST(fwid);
  CPPUNIT_TEST_SUITE_END();


private:
  struct usb_device*   m_dev;
  CVMUSB*  m_pInterface;
public:
  void setUp() {
    vector<struct usb_device*> devices = CVMUSB::enumerate();
    m_pInterface = new CVMUSB(devices[0]);
  }
  void tearDown() {
    delete m_pInterface;
  }
protected:
  void action();
  void fwid();
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
