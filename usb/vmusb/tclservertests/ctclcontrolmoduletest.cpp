

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include <vector>
#include <string>
#include <memory>
#include <iostream>
#include <iterator>
#include <CControlModule.h>
#include <CMockVMUSB.h>

#define private public
#define protected public
#include <CTclControlModule.h>
#undef protected
#undef private

using namespace std;

class CTclControlModuleTests : public CppUnit::TestFixture {
  public:
    CPPUNIT_TEST_SUITE(CTclControlModuleTests);
    CPPUNIT_TEST(construct_0);
    CPPUNIT_TEST(cget_0);
    CPPUNIT_TEST_SUITE_END();


public:
  void setUp() {
  }
  void tearDown() {
  }
protected:
  void construct_0();
  void cget_0();

};

CPPUNIT_TEST_SUITE_REGISTRATION(CTclControlModuleTests);


void CTclControlModuleTests::construct_0() {
  CTclControlModule bus;
}

void CTclControlModuleTests::cget_0() {
  unique_ptr<CTclControlModule> bus(new CTclControlModule);
  CControlModule module("test",move(bus));

  CPPUNIT_ASSERT_NO_THROW( module.cget("-ensemble"));
}
