// Template for a test suite.

#include <config.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include <CVMEAddressRange.h>
#include <RangeError.h>

#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif



// Since CVMEAddressRange is an abstract class I need something
// concrete to test:

class CtestAddressRange : public CVMEAddressRange
{
public:
  CtestAddressRange(unsigned short am,
		    unsigned long base,
		    size_t        bytes) :
    CVMEAddressRange(am, base, bytes) {}

  // Dummies for the pure virtual at heart.

  virtual void* mappingPointer() {return NULL;}

  virtual void pokel(size_t offset, long data) {}
  virtual void pokew(size_t offset, short data) {}
  virtual void pokeb(size_t offset, char data) {}

  virtual unsigned long  peekl(size_t offset) {return 0UL;}
  virtual unsigned short peekw(size_t offset) {return 0;}
  virtual unsigned char  peekb(size_t offset) {return 0;}
  
};

class addressRangeTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(addressRangeTest);
  CPPUNIT_TEST(construct);
  CPPUNIT_TEST(compare);
  CPPUNIT_TEST(range);
  CPPUNIT_TEST_SUITE_END();


private:

public:
  void setUp() {
  }
  void tearDown() {
  }
protected:
  void construct();
  void compare();
  void range();
};

CPPUNIT_TEST_SUITE_REGISTRATION(addressRangeTest);

void addressRangeTest::construct() {
  CtestAddressRange range(0x12, 0x500000, 0x100000);

  EQMSG("base", 0x500000UL, range.base());
  EQMSG("size", static_cast<size_t>(0x100000), range.size());
  EQMSG("am  ", static_cast<unsigned short>(0x12), range.addressModifier());
}

void addressRangeTest::compare()
{
  CtestAddressRange range1(0x12, 0x500000, 0x100000);
  CtestAddressRange range2(0x13, 0x500000, 0x100000);
  CtestAddressRange range3(0x12, 0x500001, 0x100000);
  CtestAddressRange range4(0x12, 0x500000, 0x100001);

  ASSERT(range1 == range1);
  ASSERT(range1 != range2);
  ASSERT(range1 != range3);
  ASSERT(range1 != range4);
}
void addressRangeTest::range()
{
  CtestAddressRange range(0x12, 0x500000, 0x100000);
  bool thrown = false;
  try {
    range.rangeCheck(0x0fffff);
  }
  catch (CRangeError& re) {
    thrown = true;
  }
  ASSERT(!thrown);

  try {
    range.rangeCheck(0x100000);
  }
  catch (CRangeError& e) {
    thrown = true;
  }
  ASSERT(thrown);
}
