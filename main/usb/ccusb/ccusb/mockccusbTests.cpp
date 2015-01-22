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
#include <CMockCCUSB.h>
#undef private
#undef protected

using namespace std;

class CMockCCUSBTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(CMockCCUSBTests);
  CPPUNIT_TEST(executeList_0);
  CPPUNIT_TEST(executeList_1);
  CPPUNIT_TEST(simpleControl_0);
  CPPUNIT_TEST(addReturnDatum_0);
  CPPUNIT_TEST(addReturnData_0);
  CPPUNIT_TEST_SUITE_END();


public:
  void setUp() {
  }
  void tearDown() {
  }

  void executeList_0();
  void executeList_1();
  void simpleControl_0();
  void addReturnDatum_0();
  void addReturnData_0();
};

CPPUNIT_TEST_SUITE_REGISTRATION(CMockCCUSBTests);

// Utility function to print two vectors 
template<class T>
void print_vectors(const vector<T>& expected, const vector<T>& actual) {
  cout.flags(ios::hex);

  copy(expected.begin(), expected.end(), ostream_iterator<T>(cout,"\n"));
  cout << "---" << endl;
  copy(actual.begin(), actual.end(), ostream_iterator<T>(cout,"\n"));

  cout.flags(ios::dec);
}

void CMockCCUSBTests::executeList_0()
{
  // test for the ability to execute a normal CCCUSBReadoutList
  // It should record the raw commands into string format.
  //
  vector<uint16_t> raw = {0, 1 ,2, 3};
  CCCUSBReadoutList list(raw);

  CMockCCUSB ctlr;
  size_t nBytes=0;
  uint8_t buffer[8];
  int status = ctlr.executeList(list, buffer, sizeof(buffer), &nBytes);

  CPPUNIT_ASSERT(0 == status);

  vector<std::string> expected(6);
  expected[0] = "executeList::begin";
  expected[1] = "0000";
  expected[2] = "0001";
  expected[3] = "0002";
  expected[4] = "0003";
  expected[5] = "executeList::end";

  auto record = ctlr.getOperationsRecord();
  //print_vectors(expected, record);

  CPPUNIT_ASSERT( expected == record);
}

void CMockCCUSBTests::executeList_1()
{
  cout << "\nexecuteList_1" << endl;
  // test for the ability to test CLoggingReadoutList
  // It should record the nice human readable record
  //
  CMockCCUSB ctlr;
  CLoggingReadoutList* pList = ctlr.createReadoutList();
  pList->addWrite16(16, 0, 16, static_cast<uint16_t>(213));

  size_t nBytes=0;
  uint8_t buffer[8];
  int status = ctlr.executeList(*pList, buffer, sizeof(buffer), &nBytes);

  CPPUNIT_ASSERT(0 == status);

  vector<std::string> expected(3);
  expected[0] = "executeList::begin";
  expected[1] = "write16 16 0 16 213";
  expected[2] = "executeList::end";

  auto record = ctlr.getOperationsRecord();
  //print_vectors(expected, record);

  CPPUNIT_ASSERT( expected == record);
}


void CMockCCUSBTests::simpleControl_0() 
{
  CMockCCUSB ctlr;
  ctlr.simpleControl(10,0,10);

  vector<string> expected={"executeList::begin",
    "control 10 0 10",
    "executeList::end"};
  auto record = ctlr.getOperationsRecord();

  print_vectors(expected, record);
  CPPUNIT_ASSERT(expected == record);
}

void CMockCCUSBTests::addReturnDatum_0()
{
  CMockCCUSB ctlr;
  ctlr.addReturnDatum(3);

  CCCUSBReadoutList list; 
  uint16_t buffer[2];
  size_t nBytes=0;
  int status = ctlr.executeList(list, buffer, sizeof(buffer), &nBytes);

  CPPUNIT_ASSERT_EQUAL(0, status);
  CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2),nBytes);
  CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(3),buffer[0]);


}
void CMockCCUSBTests::addReturnData_0()
{
  CMockCCUSB ctlr;
  vector<uint16_t> toReturn = {4, 3, 2, 1};
  ctlr.addReturnData(toReturn);

  CCCUSBReadoutList list; 
  uint16_t buffer[2];
  size_t nBytes=0;
  int status = ctlr.executeList(list, buffer, sizeof(buffer), &nBytes);

  CPPUNIT_ASSERT_EQUAL(0, status);
  CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(4),nBytes);
  CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(4),buffer[0]);
  CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(3),buffer[1]);

}
