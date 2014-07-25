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

class CVMUSBReadoutListTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(CVMUSBReadoutListTests);
  CPPUNIT_TEST(listConstruct0);
  CPPUNIT_TEST_SUITE_END();


public:
  void setUp() {
  }
  void tearDown() {
  }

  void listConstruct0();
};

CPPUNIT_TEST_SUITE_REGISTRATION(CVMUSBReadoutListTests);


void CVMUSBReadoutListTests::listConstruct0()
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
