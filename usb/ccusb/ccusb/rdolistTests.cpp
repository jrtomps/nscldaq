// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include <vector>
#include <stdint.h>
#include <iostream>
#include <iomanip>
#include <iterator>

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
  CPPUNIT_TEST(append_0);
  CPPUNIT_TEST(addWrite16_0);
  CPPUNIT_TEST(addWrite24_0);
  CPPUNIT_TEST(addRead16_0);
  CPPUNIT_TEST(addRead16_1);
  CPPUNIT_TEST(addRead24_0);
  CPPUNIT_TEST(addRead24_1);
  CPPUNIT_TEST(addControl_0);
  CPPUNIT_TEST(addQStop_0);
  CPPUNIT_TEST(addQStop_1);
  CPPUNIT_TEST(addQStop24_0);
  CPPUNIT_TEST(addQStop24_1);
  CPPUNIT_TEST(addQScan_0);
  CPPUNIT_TEST(addQScan_1);
  CPPUNIT_TEST(addRepeat_0);
  CPPUNIT_TEST(addRepeat_1);
  CPPUNIT_TEST(addMarker_0);
  CPPUNIT_TEST_SUITE_END();


public:
  void setUp() {
  }
  void tearDown() {
  }

  void listConstruct0();
  void addAddressPatternRead16_0(); 
  void addAddressPatternRead16_1(); 
  void append_0();
  void addWrite16_0();
  void addWrite24_0();
  void addRead16_0();
  void addRead16_1();
  void addRead24_0();
  void addRead24_1();
  void addControl_0();
  void addQStop_0();
  void addQStop_1();
  void addQStop24_0();
  void addQStop24_1();
  void addQScan_0();
  void addQScan_1();
  void addRepeat_0();
  void addRepeat_1();
  void addMarker_0();
};

CPPUNIT_TEST_SUITE_REGISTRATION(CCCUSBReadoutListTests);

// Utility function to print two vectors 
template<class T>
void print_vectors(const vector<T>& expected, const vector<T>& actual) {
  cout.flags(ios::hex);

  copy(expected.begin(), expected.end(), ostream_iterator<T>(cout,"\n"));
  cout << "---" << endl;
  copy(actual.begin(), actual.end(), ostream_iterator<T>(cout,"\n"));

  cout.flags(ios::dec);
}

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



void CCCUSBReadoutListTests::append_0 () 
{

  CCCUSBReadoutList rdoList0;
  rdoList0.addWrite16(10, 0, 16, static_cast<uint16_t>(100));

  CCCUSBReadoutList rdoList1;
  rdoList1.addWrite16(11, 1, 17, static_cast<uint16_t>(101));

  rdoList0.append(rdoList1);

  std::vector<uint16_t> expected(4);
  expected[0] = ((10<<9) | (0<<5) | 16);
  expected[1] = 100;
  expected[2] = ((11<<9) | (1<<5) | 17);
  expected[3] = 101;

  CPPUNIT_ASSERT( expected == rdoList0.get() );
}

void CCCUSBReadoutListTests::addWrite16_0 ()
{
  CCCUSBReadoutList rdolist;
  uint16_t data = 234;
  rdolist.addWrite16(8, 0 , 16, data);

  std::vector<uint16_t> expected(2);
  expected[0] = (8<<9)|(0<<5)|16; 
  expected[1] = 234;

  //print_vectors(expected, rdolist.get());
  CPPUNIT_ASSERT( expected == rdolist.get() );
}


void CCCUSBReadoutListTests::addWrite24_0 ()
{
  CCCUSBReadoutList rdolist;
  uint16_t data = 234;
  rdolist.addWrite24(8, 0 , 16, data);

  std::vector<uint16_t> expected(3);
  expected[0] = (1<<14)|(8<<9)|(0<<5)|16; 
  expected[1] = 234;
  expected[2] = 0;

  //print_vectors(expected, rdolist.get());
  CPPUNIT_ASSERT( expected == rdolist.get() );
}

void CCCUSBReadoutListTests::addRead16_0 ()
{
  CCCUSBReadoutList rdolist;
  bool lamwait = true;
  rdolist.addRead16(8, 0 , 0, lamwait);

  std::vector<uint16_t> expected(2);
  expected[0] = (1<<15)|(8<<9)|(0<<5); 
  expected[1] = (1<<7);

  CPPUNIT_ASSERT( expected == rdolist.get() );
}


void CCCUSBReadoutListTests::addRead16_1 ()
{
  CCCUSBReadoutList rdolist;
  bool lamwait = false;
  rdolist.addRead16(8, 0 , 0, lamwait);

  std::vector<uint16_t> expected(1);
  expected[0] = (8<<9)|(0<<5); 

  CPPUNIT_ASSERT( expected == rdolist.get() );
}


void CCCUSBReadoutListTests::addRead24_0 ()
{
  CCCUSBReadoutList rdolist;
  bool lamwait = true;
  rdolist.addRead24(8, 0 , 0, lamwait);

  std::vector<uint16_t> expected(2);
  expected[0] = (1<<15)|(1<<14)|(8<<9)|(0<<5); 
  expected[1] = (1<<7);

  CPPUNIT_ASSERT( expected == rdolist.get() );
}


void CCCUSBReadoutListTests::addRead24_1 ()
{
  CCCUSBReadoutList rdolist;
  bool lamwait = false;
  rdolist.addRead24(8, 0 , 0, lamwait);

  std::vector<uint16_t> expected(1);
  expected[0] = (1<<14)|(8<<9)|(0<<5); 

  CPPUNIT_ASSERT( expected == rdolist.get() );
}

void CCCUSBReadoutListTests::addControl_0 ()
{
  CCCUSBReadoutList rdolist;
  rdolist.addControl(8, 0 , 8);

  std::vector<uint16_t> expected(1);
  expected[0] = (8<<9)|(0<<5)|8; 

  CPPUNIT_ASSERT( expected == rdolist.get() );
}


void CCCUSBReadoutListTests::addQStop_0 ()
{
  CCCUSBReadoutList rdolist;
  uint16_t nMax = 10;
  bool lamwait = false;
  rdolist.addQStop(8, 0 , 0, nMax, lamwait);

  std::vector<uint16_t> expected(3);
  expected[0] = (1<<15)|(8<<9)|(0<<5)|0;  // contin. bit, NAF=(8,0,0);
  expected[1] = (1<<15)|(1<<4); // contin. bit, Qstop mode
  expected[2] = 10; // max iters

  //print_vectors(expected, rdolist.get());
  CPPUNIT_ASSERT( expected == rdolist.get() );
}


void CCCUSBReadoutListTests::addQStop_1 ()
{
  CCCUSBReadoutList rdolist;
  uint16_t nMax = 10;
  bool lamwait = true;
  rdolist.addQStop(8, 0 , 0, nMax, lamwait);

  std::vector<uint16_t> expected(3);
  expected[0] = (1<<15)|(8<<9)|(0<<5)|0;  // contin. bit, NAF=(8,0,0);
  expected[1] = (1<<15)|(1<<7)|(1<<4); // contin. bit, LAM mode, Qstop mode
  expected[2] = 10; // max iters

  //print_vectors(expected, rdolist.get());
  CPPUNIT_ASSERT( expected == rdolist.get() );
}


void CCCUSBReadoutListTests::addQStop24_0 ()
{
  CCCUSBReadoutList rdolist;
  uint16_t nMax = 10;
  bool lamwait = false;
  rdolist.addQStop24(8, 0 , 0, nMax, lamwait);

  std::vector<uint16_t> expected(3);
  expected[0] = (1<<15)|(1<<14)|(8<<9)|(0<<5)|0;  // contin. bit, long, NAF=(8,0,0);
  expected[1] = (1<<15)|(1<<4); // contin. bit, Qstop mode
  expected[2] = 10; // max iters

  //print_vectors(expected, rdolist.get());
  CPPUNIT_ASSERT( expected == rdolist.get() );
}


void CCCUSBReadoutListTests::addQStop24_1 ()
{
  CCCUSBReadoutList rdolist;
  uint16_t nMax = 10;
  bool lamwait = true;
  rdolist.addQStop24(8, 0 , 0, nMax, lamwait);

  std::vector<uint16_t> expected(3);
  expected[0] = (1<<15)|(1<<14)|(8<<9)|(0<<5)|0;  // contin. bit, long, NAF=(8,0,0);
  expected[1] = (1<<15)|(1<<7)|(1<<4); // contin. bit, LAM mode, Qstop mode
  expected[2] = 10; // max iters

  //print_vectors(expected, rdolist.get());
  CPPUNIT_ASSERT( expected == rdolist.get() );
}


void CCCUSBReadoutListTests::addQScan_0 ()
{
  CCCUSBReadoutList rdolist;
  uint16_t nMax = 10;
  bool lamwait = false;
  rdolist.addQScan(8, 0 , 0, nMax, lamwait);

  std::vector<uint16_t> expected(3);
  expected[0] = (1<<15)|(8<<9)|(0<<5)|0;  // contin. bit, NAF=(8,0,0);
  expected[1] = (1<<15)|(1<<5); // contin. bit, address scan
  expected[2] = 10; // max iters

  //print_vectors(expected, rdolist.get());
  CPPUNIT_ASSERT( expected == rdolist.get() );
}


void CCCUSBReadoutListTests::addQScan_1 ()
{
  CCCUSBReadoutList rdolist;
  uint16_t nMax = 10;
  bool lamwait = true;
  rdolist.addQScan(8, 0 , 0, nMax, lamwait);

  std::vector<uint16_t> expected(3);
  expected[0] = (1<<15)|(8<<9)|(0<<5)|0;  // contin. bit, NAF=(8,0,0);
  expected[1] = (1<<15)|(1<<7)|(1<<5); // contin. bit, LAM mode, address scan
  expected[2] = 10; // max iters

  //print_vectors(expected, rdolist.get());
  CPPUNIT_ASSERT( expected == rdolist.get() );
}


void CCCUSBReadoutListTests::addRepeat_0 ()
{
  CCCUSBReadoutList rdolist;
  uint16_t nMax = 10;
  bool lamwait = false;
  rdolist.addRepeat(8, 0 , 0, nMax, lamwait);

  std::vector<uint16_t> expected(3);
  expected[0] = (1<<15)|(8<<9)|(0<<5)|0;  // contin. bit, NAF=(8,0,0);
  expected[1] = (1<<15)|(1<<6); // contin. bit, repeat mode 
  expected[2] = 10; // max iters

  //print_vectors(expected, rdolist.get());
  CPPUNIT_ASSERT( expected == rdolist.get() );
}


void CCCUSBReadoutListTests::addRepeat_1 ()
{
  CCCUSBReadoutList rdolist;
  uint16_t nMax = 10;
  bool lamwait = true;
  rdolist.addRepeat(8, 0 , 0, nMax, lamwait);

  std::vector<uint16_t> expected(3);
  expected[0] = (1<<15)|(8<<9)|(0<<5)|0;  // contin. bit, NAF=(8,0,0);
  expected[1] = (1<<15)|(1<<7)|(1<<6); // contin. bit, LAM mode, repeat mode
  expected[2] = 10; // max iters

  //print_vectors(expected, rdolist.get());
  CPPUNIT_ASSERT( expected == rdolist.get() );
}


void CCCUSBReadoutListTests::addMarker_0 ()
{
  CCCUSBReadoutList rdolist;
  rdolist.addMarker(0xabcd);

  std::vector<uint16_t> expected(2);
  expected[0] = (0<<9)|(0<<5)|16;  // NAF=(0,0,16);
  expected[1] = 0xabcd;

  //print_vectors(expected, rdolist.get());
  CPPUNIT_ASSERT( expected == rdolist.get() );
}
