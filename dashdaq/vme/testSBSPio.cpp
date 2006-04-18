// Template for a test suite.

#include <config.h>
#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include <CSBSVMEInterface.h>
#include <CSBSPio.h>
#include <CSBSVmeAddressRange.h>
#include <iostream>

class testSBSPio : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(testSBSPio);
  CPPUNIT_TEST(longwrite);
  CPPUNIT_TEST(shortwrite);
  CPPUNIT_TEST(bytewrite);
  CPPUNIT_TEST(longread);
  CPPUNIT_TEST(shortread);
  CPPUNIT_TEST(byteread);
  CPPUNIT_TEST_SUITE_END();


private:
  static bool          m_Warned;
  CSBSVMEInterface*    m_pInterface;
  CSBSVmeAddressRange* m_pMap;
  CSBSPio*             m_pPio;

public:
  void setUp() {
    if(!m_Warned) {
      cerr << "testSBSPio requires an SBS VME interface connected to a crate that\n";
      cerr << "           is powered on and contains VME A24 memory at 0x500000\n";
      cerr << "           through 0x600000\n";
      cerr << "If these conditions are not met, the tests will fail\n";
      m_Warned = true;
    }
    m_pInterface = new CSBSVMEInterface(0);
    m_pMap       = new CSBSVmeAddressRange((bt_desc_t)m_pInterface->getDeviceHandle(),
					   0x39, 
					   0x500000, 0x100000);
    m_pPio       = new CSBSPio((bt_desc_t)m_pInterface->getDeviceHandle());
  }
  void tearDown() {
    delete m_pMap;
    delete m_pPio;
    delete m_pInterface;
  }
protected:
  void longwrite();
  void shortwrite();
  void bytewrite();
  void longread();
  void shortread();
  void byteread();
};

bool testSBSPio::m_Warned(false);

CPPUNIT_TEST_SUITE_REGISTRATION(testSBSPio);

void testSBSPio::longwrite() {
  m_pPio->write32(0x39, 0x500000, 0x12345678l);
  EQ(0x12345678ul, m_pMap->peekl(0));
}

void testSBSPio::shortwrite() {
  m_pPio->write16(0x39, 0x500100, 0x5555);
  EQ((unsigned short)0x5555, m_pMap->peekw(0x100/sizeof(short)));

}
void testSBSPio::bytewrite()
{
  m_pPio->write8(0x39, 0x500080, 0xaa);
  EQ((unsigned char)0xaa, m_pMap->peekb(0x80));
}

void testSBSPio::longread()
{
  m_pPio->write32(0x39, 0x500000, 0x87654321l);
  EQ(0x87654321UL, m_pPio->read32(0x39, 0x500000));
}
void testSBSPio::shortread()
{
  m_pPio->write16(0x39, 0x500100, 0xaaaa);
  EQ((unsigned short)0xaaaa, m_pPio->read16(0x39, 0x500100));
}
void testSBSPio::byteread()
{
  m_pPio->write16(0x39, 0x500080, 0x55);
  EQ((unsigned char)0x55, m_pPio->read8(0x39, 0x500080));
}
