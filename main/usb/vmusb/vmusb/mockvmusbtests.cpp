// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>

#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <algorithm>
#include <iterator>
#include <iostream>

#include "Asserts.h"

#define private public
#define protected public
#include <CMockVMUSB.h>
#undef protected
#undef private

using namespace std;

// If your memory is in a different place.. change the three lines
// below:

class CMockVMUSBTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(CMockVMUSBTests);
  CPPUNIT_TEST (executeList_0); 
  CPPUNIT_TEST (executeList_1); 
  CPPUNIT_TEST (executeList_2); 
  CPPUNIT_TEST (executeList_3); 
  CPPUNIT_TEST (executeList_4); 
  CPPUNIT_TEST (globalMode_0); 
  CPPUNIT_TEST (eventsPerBuffer_0); 
  CPPUNIT_TEST (addReturnDatum_0);
  CPPUNIT_TEST (addReturnData_0);
  CPPUNIT_TEST_SUITE_END();

  private:
  CMockVMUSB* m_pCtlr;

  public:
  void setUp() {
    m_pCtlr = new CMockVMUSB;
  }
  void tearDown() {
    delete m_pCtlr;
  }
  private:
  void executeList_0(); 
  void executeList_1(); 
  void executeList_2(); 
  void executeList_3(); 
  void executeList_4(); 
  void globalMode_0(); 
  void eventsPerBuffer_0();

  void addReturnDatum_0();
  void addReturnData_0();

};

CPPUNIT_TEST_SUITE_REGISTRATION(CMockVMUSBTests);

template<class T>
void print_vectors(const vector<T>& expected, const vector<T>& actual) {

  cout.flags(ios::hex);

  copy(expected.begin(), expected.end(), ostream_iterator<T>(cout,"\n"));
  cout << "---" << endl;
  copy(actual.begin(), actual.end(), ostream_iterator<T>(cout,"\n"));

  cout.flags(ios::dec);
}


/** Passing a reference of type CVMUSBReadoutList that actually references
 *  a CLoggingReadoutList object copies human readable data.
 */
void CMockVMUSBTests::executeList_0() {
  CVMUSBReadoutList* list = m_pCtlr->createReadoutList();
  list->addMarker(0xaaaa);

  size_t nBytesRead=0;
  uint32_t data[100]; 
  m_pCtlr->executeList(*list, data, sizeof(data), &nBytesRead);

  vector<string> expected(3);
  expected[0] = "executeList::begin";
  expected[1] = "addMarker aaaa";
  expected[2] = "executeList::end";

  CPPUNIT_ASSERT( expected == m_pCtlr->getOperationRecord());

   // cleanup
  delete list;
}

/** Passing a CVMUSBReadoutList object to executeList copies raw
 *  stack data.
 */
void CMockVMUSBTests::executeList_1() {
  CVMUSBReadoutList list;
  list.addMarker(0xaaaa);

  size_t nBytesRead=0;
  uint32_t data[100]; 
  m_pCtlr->executeList(list, data, sizeof(data), &nBytesRead);

  vector<string> expected(4);
  expected[0] = "executeList::begin";
  expected[1] = "0:00002000";
  expected[2] = "1:0000aaaa";
  expected[3] = "executeList::end";

//  print_vectors(expected, m_pCtlr->getOperationRecord());

  CPPUNIT_ASSERT( expected == m_pCtlr->getOperationRecord());
}

/** Ensure that we can set what gets read back
 *
 */
void CMockVMUSBTests::executeList_2() 
{
  vector<uint16_t> data = {0, 1, 2, 3};
  m_pCtlr->addReturnData(data,0);

  CVMUSBReadoutList list; // a dummy list
  uint16_t buffer[32];
  size_t nBytesRead;

  m_pCtlr->executeList(list, buffer, sizeof(buffer), &nBytesRead);
  CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(8), nBytesRead);
  for (size_t element=0; element<4; ++element) {
    CPPUNIT_ASSERT_EQUAL(data[element],buffer[element]);
  }
}

/**  Set up for multiple reads and provide the data to be read back on each.
 *  These are independent sets of data to return for subsequent executeList.
 *
 */
void CMockVMUSBTests::executeList_3() 
{
  vector<uint16_t> data0 = {0, 1, 2, 3};
  m_pCtlr->addReturnData(data0, 0);

  // create a new buffer to store data in
  std::vector<uint16_t> data1 = {4, 5};
  m_pCtlr->addReturnData(data1, 0);

  CVMUSBReadoutList list; // a dummy list
  uint16_t buffer[32];
  size_t nBytesRead;

  // Read the first bit of data.
  m_pCtlr->executeList(list, buffer, sizeof(buffer), &nBytesRead);
  CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(8), nBytesRead);
  for (size_t element=0; element<4; ++element) {
    CPPUNIT_ASSERT_EQUAL(data0[element],buffer[element]);
  }

  // Read the second bit of data.
  m_pCtlr->executeList(list, buffer, sizeof(buffer), &nBytesRead);
  CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(4), nBytesRead);
  for (size_t element=0; element<2; ++element) {
    CPPUNIT_ASSERT_EQUAL(data1[element],buffer[element]);
  }

}

// Force VMUSB to return specific status value
void CMockVMUSBTests::executeList_4() 
{
  m_pCtlr->addReturnDatum(3,-1);

  size_t buf;
  CLoggingReadoutList list;
  CPPUNIT_ASSERT_EQUAL(-1, 
                       m_pCtlr->executeList(list,&buf,sizeof(buf),&buf));
}

/** Test that writing to global mode can be read back appropriately
 * and that the operations get recorded properly.
 */
void CMockVMUSBTests::globalMode_0() {
  int writeValue = 0x0123;

  m_pCtlr->writeGlobalMode(writeValue);
  int readValue = m_pCtlr->readGlobalMode();

  vector<string> expected(2);
  expected[0] = "writeGlobalMode(0x00000123)";
  expected[1] = "readGlobalMode(0x00000123)";

  CPPUNIT_ASSERT( expected == m_pCtlr->getOperationRecord());
  CPPUNIT_ASSERT_EQUAL(writeValue, readValue);
}

/** Test that writing to global mode can be read back appropriately
 * and that the operations get recorded properly.
 */
void CMockVMUSBTests::eventsPerBuffer_0() {
  uint32_t writeValue = 0x0123;

  CVMUSB* pCtlr = m_pCtlr;
  pCtlr->writeEventsPerBuffer(writeValue);
  uint32_t readValue = pCtlr->readEventsPerBuffer();

  vector<string> expected(2);
  expected[0] = "writeEventsPerBuffer(0x00000123)";
  expected[1] = "readEventsPerBuffer(0x00000123)";

  CPPUNIT_ASSERT( expected == m_pCtlr->getOperationRecord());
  CPPUNIT_ASSERT_EQUAL(writeValue, readValue);
}


void CMockVMUSBTests::addReturnDatum_0() {
  auto& retData = m_pCtlr->getReturnData();

  CPPUNIT_ASSERT(0 == retData.size());

  m_pCtlr->addReturnDatum(25,0);

  CPPUNIT_ASSERT(1 == retData.size());
  CPPUNIT_ASSERT(1 == retData.front().second.size());
  CPPUNIT_ASSERT(25 == retData.front().second.at(0));
  CPPUNIT_ASSERT(0 == retData.front().first); // return status
}

void CMockVMUSBTests::addReturnData_0() {
  // gain a reference to the return data
  auto& returnData = m_pCtlr->getReturnData();

  // at this point there should be nothing in it
  CPPUNIT_ASSERT(0 == returnData.size());

  // add some data to it
  std::vector<uint16_t> data = {0,1,2};
  m_pCtlr->addReturnData(data,0); // by default the status is set to 0

  // grab the first element's data buffer
  auto& retData = returnData.front().second;

  // it should now have 1 element
  CPPUNIT_ASSERT(1 == returnData.size());
  CPPUNIT_ASSERT(3 == retData.size()); // 
  CPPUNIT_ASSERT(0 == retData.at(0));
  CPPUNIT_ASSERT(1 == retData.at(1));
  CPPUNIT_ASSERT(2 == retData.at(2));
}

