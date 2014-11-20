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
  CPPUNIT_TEST (globalMode_0); 
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
  void globalMode_0(); 

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

//  print_vectors(expected, m_pCtlr->getOperationRecord());

  CPPUNIT_ASSERT( expected == m_pCtlr->getOperationRecord());
  CPPUNIT_ASSERT_EQUAL(writeValue, readValue);
}
