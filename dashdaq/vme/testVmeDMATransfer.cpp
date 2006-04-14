// Template for a test suite.
#include <config.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"

#include "CVmeDMATransfer.h"

#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif


// need a test dma transfer class since the one we're
// testing is abstract.


class testDMA : public CVmeDMATransfer
{
public:
  testDMA(unsigned short               addressModifier,
	  CVMEInterface::TransferWidth width,
	  unsigned long                base, 
	  size_t                       length) :
    CVmeDMATransfer(addressModifier, width, base, length) {}
  testDMA(const testDMA& rhs) :
    CVmeDMATransfer(rhs) {}

  virtual ~testDMA () { }

  virtual size_t Read(void* buffer) { return 0;}
  virtual size_t Write(void* buffer) {return 0;}
};


class dmaTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(dmaTest);
  CPPUNIT_TEST(construct);
  CPPUNIT_TEST(copyequal);
  CPPUNIT_TEST(assignequal);
  CPPUNIT_TEST(inequal);
  CPPUNIT_TEST_SUITE_END();


private:

public:
  void setUp() {
  }
  void tearDown() {
  }
protected:
  void construct();
  void copyequal();
  void assignequal();
  void inequal();
};

CPPUNIT_TEST_SUITE_REGISTRATION(dmaTest);

void dmaTest::construct() {
  testDMA test(0x12, CVMEInterface::TW_32, 0x500000, 0x1000);
  
  EQMSG("base", (unsigned long)0x500000, test.base());
  EQMSG("length", (size_t)0x1000, test.length());
  EQMSG("amod",   (unsigned short)0x12, test.modifier());
  EQMSG("width", CVMEInterface::TW_32,  test.width());
}

void dmaTest::copyequal()
{
  testDMA test1(0x12, CVMEInterface::TW_32, 0x500000, 0x1000);
  testDMA test2(test1);

  ASSERT(test1 == test2);
}
void dmaTest::assignequal()
{
  testDMA test1(0x12, CVMEInterface::TW_32, 0x500000, 0x1000);
  testDMA test2(0, CVMEInterface::TW_16, 0, 0);
 
  test2 = test1;
  ASSERT(test1 == test2);
}
void dmaTest::inequal()
{
  testDMA test1(0x12, CVMEInterface::TW_32, 0x500000, 0x1000);
  testDMA test2(0, CVMEInterface::TW_16, 0, 0);

  ASSERT(test1 != test2);
}
