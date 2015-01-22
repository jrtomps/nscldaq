// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include <vector>
#include <stdint.h>
#include <iostream>
#include <iomanip>
#include <iterator>
#include <string>

#define protected public
#define private   public
#include <CLoggingReadoutList.h>
#undef private
#undef protected

using namespace std;

class CLoggingReadoutListTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(CLoggingReadoutListTests);
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

CPPUNIT_TEST_SUITE_REGISTRATION(CLoggingReadoutListTests);

// Utility function to print two vectors 
template<class T>
void print_vectors(const vector<T>& expected, const vector<T>& actual) {
  cout.flags(ios::hex);

  copy(expected.begin(), expected.end(), ostream_iterator<T>(cout,"\n"));
  cout << "---" << endl;
  copy(actual.begin(), actual.end(), ostream_iterator<T>(cout,"\n"));

  cout.flags(ios::dec);
}



void CLoggingReadoutListTests::addAddressPatternRead16_0 ()
{

  CLoggingReadoutList rdolist;
  rdolist.addAddressPatternRead16( 4, 1, 6, 1);
  
  std::vector<std::string> expected(1);
  expected[0] = "addressPatternRead16 4 1 6 true";

  CPPUNIT_ASSERT( expected == rdolist.getLog());
}


void CLoggingReadoutListTests::addAddressPatternRead16_1 ()
{

  CLoggingReadoutList rdolist;
  rdolist.addAddressPatternRead16( 4, 1, 6, 0);
  
  std::vector<std::string> expected(1);
  expected[0] = "addressPatternRead16 4 1 6 false";

  CPPUNIT_ASSERT( expected == rdolist.getLog());
}



void CLoggingReadoutListTests::append_0 () 
{

  CLoggingReadoutList rdoList0;
  rdoList0.addWrite16(10, 0, 16, static_cast<uint16_t>(100));

  CLoggingReadoutList rdoList1;
  rdoList1.addWrite16(11, 1, 17, static_cast<uint16_t>(101));

  rdoList0.append(rdoList1);

  std::vector<std::string> expected(2);
  expected[0] = "write16 10 0 16 100";
  expected[1] = "write16 11 1 17 101";

  CPPUNIT_ASSERT( expected == rdoList0.getLog() );
}

void CLoggingReadoutListTests::addWrite16_0 ()
{
  CLoggingReadoutList rdolist;
  uint16_t data = 234;
  rdolist.addWrite16(8, 0 , 16, data);

  std::vector<std::string> expected(1);
  expected[0] = "write16 8 0 16 234";

  //print_vectors(expected, rdolist.getLog());
  CPPUNIT_ASSERT( expected == rdolist.getLog() );
}


void CLoggingReadoutListTests::addWrite24_0 ()
{
  CLoggingReadoutList rdolist;
  uint16_t data = 234;
  rdolist.addWrite24(8, 0 , 16, data);

  std::vector<std::string> expected(1);
  expected[0] = "write24 8 0 16 234";

  //print_vectors(expected, rdolist.getLog());
  CPPUNIT_ASSERT( expected == rdolist.getLog() );
}

void CLoggingReadoutListTests::addRead16_0 ()
{
  CLoggingReadoutList rdolist;
  bool lamwait = true;
  rdolist.addRead16(8, 0 , 0, lamwait);

  std::vector<std::string> expected(1);
  expected[0] = "read16 8 0 0 true";

  CPPUNIT_ASSERT( expected == rdolist.getLog() );
}


void CLoggingReadoutListTests::addRead16_1 ()
{
  CLoggingReadoutList rdolist;
  bool lamwait = false;
  rdolist.addRead16(8, 0 , 0, lamwait);

  std::vector<std::string> expected(1);
  expected[0] = "read16 8 0 0 false";

  CPPUNIT_ASSERT( expected == rdolist.getLog() );
}


void CLoggingReadoutListTests::addRead24_0 ()
{
  CLoggingReadoutList rdolist;
  bool lamwait = true;
  rdolist.addRead24(8, 0 , 0, lamwait);

  std::vector<std::string> expected(1);
  expected[0] = "read24 8 0 0 true";

  CPPUNIT_ASSERT( expected == rdolist.getLog() );
}


void CLoggingReadoutListTests::addRead24_1 ()
{
  CLoggingReadoutList rdolist;
  bool lamwait = false;
  rdolist.addRead24(8, 0 , 0, lamwait);

  std::vector<std::string> expected(1);
  expected[0] = "read24 8 0 0 false";

  CPPUNIT_ASSERT( expected == rdolist.getLog() );
}

void CLoggingReadoutListTests::addControl_0 ()
{
  CLoggingReadoutList rdolist;
  rdolist.addControl(8, 0 , 8);

  std::vector<std::string> expected(1);
  expected[0] = "control 8 0 8";

  CPPUNIT_ASSERT( expected == rdolist.getLog() );
}


void CLoggingReadoutListTests::addQStop_0 ()
{
  CLoggingReadoutList rdolist;
  uint16_t nMax = 10;
  bool lamwait = false;
  rdolist.addQStop(8, 0 , 0, nMax, lamwait);

  std::vector<std::string> expected(1);
  expected[0] = "qStop 8 0 0 10 false";

  //print_vectors(expected, rdolist.getLog());
  CPPUNIT_ASSERT( expected == rdolist.getLog() );
}


void CLoggingReadoutListTests::addQStop_1 ()
{
  CLoggingReadoutList rdolist;
  uint16_t nMax = 10;
  bool lamwait = true;
  rdolist.addQStop(8, 0 , 0, nMax, lamwait);

  std::vector<std::string> expected(1);
  expected[0] = "qStop 8 0 0 10 true";

  //print_vectors(expected, rdolist.getLog());
  CPPUNIT_ASSERT( expected == rdolist.getLog() );
}


void CLoggingReadoutListTests::addQStop24_0 ()
{
  CLoggingReadoutList rdolist;
  uint16_t nMax = 10;
  bool lamwait = false;
  rdolist.addQStop24(8, 0 , 0, nMax, lamwait);

  std::vector<std::string> expected(1);
  expected[0] = "qStop24 8 0 0 10 false";

  //print_vectors(expected, rdolist.getLog());
  CPPUNIT_ASSERT( expected == rdolist.getLog() );
}


void CLoggingReadoutListTests::addQStop24_1 ()
{
  CLoggingReadoutList rdolist;
  uint16_t nMax = 10;
  bool lamwait = true;
  rdolist.addQStop24(8, 0 , 0, nMax, lamwait);

  std::vector<std::string> expected(1);
  expected[0] = "qStop24 8 0 0 10 true";

  //print_vectors(expected, rdolist.getLog());
  CPPUNIT_ASSERT( expected == rdolist.getLog() );
}


void CLoggingReadoutListTests::addQScan_0 ()
{
  CLoggingReadoutList rdolist;
  uint16_t nMax = 10;
  bool lamwait = false;
  rdolist.addQScan(8, 0 , 0, nMax, lamwait);

  std::vector<std::string> expected(1);
  expected[0] = "qScan 8 0 0 10 false";

  //print_vectors(expected, rdolist.getLog());
  CPPUNIT_ASSERT( expected == rdolist.getLog() );
}


void CLoggingReadoutListTests::addQScan_1 ()
{
  CLoggingReadoutList rdolist;
  uint16_t nMax = 10;
  bool lamwait = true;
  rdolist.addQScan(8, 0 , 0, nMax, lamwait);

  std::vector<std::string> expected(1);
  expected[0] = "qScan 8 0 0 10 true";

  //print_vectors(expected, rdolist.getLog());
  CPPUNIT_ASSERT( expected == rdolist.getLog() );
}


void CLoggingReadoutListTests::addRepeat_0 ()
{
  CLoggingReadoutList rdolist;
  uint16_t nMax = 10;
  bool lamwait = false;
  rdolist.addRepeat(8, 0 , 0, nMax, lamwait);

  std::vector<std::string> expected(1);
  expected[0] = "repeat 8 0 0 10 false";

  //print_vectors(expected, rdolist.getLog());
  CPPUNIT_ASSERT( expected == rdolist.getLog() );
}


void CLoggingReadoutListTests::addRepeat_1 ()
{
  CLoggingReadoutList rdolist;
  uint16_t nMax = 10;
  bool lamwait = true;
  rdolist.addRepeat(8, 0 , 0, nMax, lamwait);

  std::vector<std::string> expected(1);
  expected[0] = "repeat 8 0 0 10 true";

  //print_vectors(expected, rdolist.getLog());
  CPPUNIT_ASSERT( expected == rdolist.getLog() );
}


void CLoggingReadoutListTests::addMarker_0 ()
{
  CLoggingReadoutList rdolist;
  rdolist.addMarker(0xabcd);

  std::vector<std::string> expected(1);
  expected[0] = "marker abcd"; // markers are special and are recorded in hex

  //print_vectors(expected, rdolist.getLog());
  CPPUNIT_ASSERT( expected == rdolist.getLog() );
}
