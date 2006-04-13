// Template for a test suite.

#include <config.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include <stdio.h>
#include <iostream>
#include <CSBSVmeAddressRange.h>


#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif


static const long   base(0x500000);
static const size_t size(0x100000);
static const unsigned short am(0x39); // Standard user data.


class SBSrangetest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(SBSrangetest);
  CPPUNIT_TEST(longs);
  CPPUNIT_TEST(shorts);
  CPPUNIT_TEST(bytes);
  CPPUNIT_TEST(pointer);
  CPPUNIT_TEST_SUITE_END();
protected:
  void longs();
  void shorts();
  void bytes();
  void pointer();

private:
  static bool          warned;
  bt_desc_t            m_handle;
  CSBSVmeAddressRange* m_Range;
  
public:
  void setUp() {
    if(!warned) {
      warned = true;
      cerr << "\nWarning: the tests in testSBSRange.cpp require:\n";
      cerr << "   1. An SBS PCI/VME interface connected to an online VME crate\n";
      cerr << "   2.A24  Memory in that crate in the range 0x500000-0x5fffff\n";
      cerr << "If these conditions are not satisfied, tests will fail\n";
    }
    // Open the SBS device on crate 0 for A24:

    char name[100];
    if (bt_gen_name(0, BT_DEV_A24, name, sizeof(name)) != name) {
      cerr << "bt_gen_name failed for a24, crate 0\n";
      return;
    }
    bt_error_t stat = bt_open(&m_handle,
			      name, BT_RDWR);
    if (stat != BT_SUCCESS) {
      char errorstr[100];
      cerr << bt_strerror(m_handle, stat, "bt_open failed: ", errorstr, 
						sizeof(errorstr)) << endl;
      return;
    }
    bt_reset(m_handle);
    bt_clrerr(m_handle);

    m_Range = new CSBSVmeAddressRange(m_handle,
				      am, base, size);

  }
  void tearDown() {
    delete m_Range;
    bt_close(m_handle);
  }
};

bool SBSrangetest::warned(false);

CPPUNIT_TEST_SUITE_REGISTRATION(SBSrangetest);

// Set/get longs at offset 123
void SBSrangetest::longs() {
  long patterns[] = {
    0x0, 0xffffffff, 0xaaaaaaaa, 0x55555555, 0xf0f0f0f0, 0x0f0f0f0f};
  int npatterns = sizeof(patterns)/sizeof(long);

  for (int i = 0; i < npatterns; i++) {
    m_Range->pokel(123, patterns[i]);
    unsigned long r = m_Range->peekl(123);

    EQ(patterns[i], (long)r);
  }
  
}

void SBSrangetest::shorts()
{
  short patterns[] = {
    0x0, 0xffff, 0xaaaa, 0x5555, 0xf0f0, 0x0f0f};
  int npatterns = sizeof(patterns)/sizeof(short);

  for (int i =0; i < npatterns; i++) {
    m_Range->pokew(123, patterns[i]);
    unsigned short r = m_Range->peekw(123);
    EQ(patterns[i], (short)r);
  }
}

void SBSrangetest::bytes()
{
  // 256 is few enough to exhaustively test all bit patterns>

  for(int i = 0; i < 256; i++) {
    m_Range->pokeb(6, (char)i);
    unsigned char r = m_Range->peekb(6);
    EQ((unsigned char)i, r);
  }
}
// Pointer pokes should match peeks.
void SBSrangetest::pointer()
{
  unsigned char* ptr = static_cast<unsigned char*>(m_Range->mappingPointer());
  
  // romp over a range of 16 bytes with counting patterns.

  for(int off = 0; off < 16; off++) {
    for(int i =0; i < 255; i++) {
      ptr[off] = (unsigned char)i;
      EQ((unsigned char)i, m_Range->peekb(off));
    }
  }
}
