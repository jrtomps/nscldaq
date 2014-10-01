// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include <vector>
#include <stdint.h>
#include <iostream>
#include <iomanip>

#define protected public
#define private   public
#include <CCCUSBReadoutList.h>
#undef private
#undef protected

using namespace std;

class CCCUSBReadoutListTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(CCCUSBReadoutListTests);
  CPPUNIT_TEST(listConstruct0);
  CPPUNIT_TEST(addAddressPatternRead16_0);
  CPPUNIT_TEST(addAddressPatternRead16_1);
  CPPUNIT_TEST(addAddressPatternRead24_0);
  CPPUNIT_TEST_SUITE_END();


public:
  void setUp() {
  }
  void tearDown() {
  }

  void listConstruct0();
  void addAddressPatternRead16_0(); 
  void addAddressPatternRead16_1(); 
  void addAddressPatternRead24_0(); 
};

CPPUNIT_TEST_SUITE_REGISTRATION(CCCUSBReadoutListTests);


void CCCUSBReadoutListTests::listConstruct0()
{
  std::vector<uint16_t> list;
  list.push_back(0);
  list.push_back(1);
  list.push_back(2);
  list.push_back(3);
  list.push_back(4);

  CCCUSBReadoutList rdolist(list);
  
  CPPUNIT_ASSERT(list == rdolist.m_list);
}




void CCCUSBReadoutListTests::addAddressPatternRead16_0 ()
{

  CCCUSBReadoutList rdolist;
  rdolist.addAddressPatternRead16( 4, 1, 6, 1);
  
  std::vector<uint16_t> expected(2);
  // Should specify the complex command and NAF
  expected[0] = (0x8000 | (4<<9) | (1<<5) | 6);
  // specifies AP data (bit9) and lamwait (bit7)
  expected[1] = ((1<<9) | (1<<7));

  CPPUNIT_ASSERT( expected == rdolist.get());
}


void CCCUSBReadoutListTests::addAddressPatternRead16_1 ()
{

  CCCUSBReadoutList rdolist;
  rdolist.addAddressPatternRead16( 4, 1, 6, 0);
  
  std::vector<uint16_t> expected(2);
  // Should specify the complex command and NAF
  expected[0] = (0x8000 | (4<<9) | (1<<5) | 6);
  // sets AP data (bit9)
  expected[1] = (1<<9);

  CPPUNIT_ASSERT( expected == rdolist.get());
}

void CCCUSBReadoutListTests::addAddressPatternRead24_0 ()
{
  CCCUSBReadoutList rdolist;
  rdolist.addAddressPatternRead24( 4, 1, 6, 1);
  
  std::vector<uint16_t> expected(2);
  // Should specify the complex command, 24-bit transfer, and NAF
  expected[0] = ((1<<15) | (1<<14) | (4<<9) | (1<<5) | 6);
  // specifies AP data (bit9) and lamwait (bit7)
  expected[1] = ((1<<9) | (1<<7));

  CPPUNIT_ASSERT( expected == rdolist.get());
}
