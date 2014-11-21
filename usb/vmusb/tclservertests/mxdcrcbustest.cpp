

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include <vector>
#include <string>
#include <memory>
#include <CMockVMUSB.h>

#define private public
#define protected public
#include <CMxDCRCBus.h>
#undef protected
#undef private

using namespace std;

class CMxDCRCBusTests : public CppUnit::TestFixture {
  private:
    std::unique_ptr<CMxDCRCBus> m_pProxy;
    std::unique_ptr<CMockVMUSB> m_pCtlr;

  public:
    CPPUNIT_TEST_SUITE(CMxDCRCBusTests);
    CPPUNIT_TEST(activate_0);
    CPPUNIT_TEST(set_0);
    CPPUNIT_TEST(get_0);
    CPPUNIT_TEST_SUITE_END();


public:
  void setUp() {
    m_pProxy = std::unique_ptr<CMxDCRCBus>(new CMxDCRCBus("test"));
    m_pCtlr = std::unique_ptr<CMockVMUSB>(new CMockVMUSB);
  }
  void tearDown() {
  }
protected:
  void activate_0();
  void set_0();
  void get_0();

};

CPPUNIT_TEST_SUITE_REGISTRATION(CMxDCRCBusTests);

void CMxDCRCBusTests::activate_0() {
  m_pProxy->activate(*m_pCtlr);

  std::vector<std::string> record = m_pCtlr->getOperationRecord();
  std::vector<std::string> expected(1);
  expected[0] = 
}


void CMxDCRCBusTests::set_0() 
{
  // provide dev number, address
  m_pProxy->Set(*m_pCtlr, "d7a16", "48");

  auto record = m_pCtlr->getOperationRecord();

  vector<string> expected(4);
}


void CMxDCRCBusTests::get_0() 
{
  // provide dev number, address
  m_pProxy->Get(*m_pCtlr, "d7a16");

  auto record = m_pCtlr->getOperationRecord();

  vector<string> expected(4);
}

