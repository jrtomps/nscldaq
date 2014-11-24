

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include <vector>
#include <string>
#include <memory>
#include <iterator>
#include <algorithm>
#include <CMockVMUSB.h>
#include <CControlModule.h>

#define private public
#define protected public
#include <CMxDCRCBus.h>
#undef protected
#undef private

using namespace std;

class CMxDCRCBusTests : public CppUnit::TestFixture {
  private:
    CMxDCRCBus* m_pHdwr;
    unique_ptr<CControlModule> m_pModule;
    unique_ptr<CMockVMUSB> m_pCtlr;

  public:
    CPPUNIT_TEST_SUITE(CMxDCRCBusTests);
    CPPUNIT_TEST(activate_0);
//    CPPUNIT_TEST(set_0);
//   CPPUNIT_TEST(get_0);
    CPPUNIT_TEST_SUITE_END();


public:
  void setUp() {
    m_pModule = unique_ptr<CControlModule>(
        new CControlModule("test", new CMxDCRCBus));

    m_pModule->configure("-base","0xff000000");

    m_pCtlr = unique_ptr<CMockVMUSB>(new CMockVMUSB);
  }
  void tearDown() {
  }
protected:
  void activate_0();
  void set_0();
  void get_0();

};

CPPUNIT_TEST_SUITE_REGISTRATION(CMxDCRCBusTests);

template<class T>
void print_vectors(const vector<T>& expected, const vector<T>& actual) {
  cout.flags(ios::hex);

  copy(expected.begin(), expected.end(), ostream_iterator<T>(cout,"\n"));
  cout << "---" << endl;
  copy(actual.begin(), actual.end(), ostream_iterator<T>(cout,"\n"));

  cout.flags(ios::dec);
}

void CMxDCRCBusTests::activate_0() {
  CMxDCRCBus* hdwr = static_cast<CMxDCRCBus*>(m_pModule->getHardware());
  hdwr->activate(*m_pCtlr);

  vector<string> record = m_pCtlr->getOperationRecord();
  vector<string> expected(3);
  expected[0] = "executeList::begin";
  expected[1] = "addWrite16 ff00606e 09 3";
  expected[2] = "executeList::end";

  print_vectors(expected, record);

  CPPUNIT_ASSERT(expected == record);
}


void CMxDCRCBusTests::set_0() 
{
  // provide dev number, address
  m_pHdwr->Set(*m_pCtlr, "d7a24", "48");

  auto record = m_pCtlr->getOperationRecord();

  vector<string> expected(11);
  expected[0] = "executeList::begin";
  expected[1] = "addWrite16 ff006082 7";
  expected[2] = "addWrite16 ff006084 16";
  expected[3] = "addWrite16 ff006086 24";
  expected[4] = "executeList::end";
  // then read for data as single shot commands..
  expected[5] = "executeList::begin";
  expected[6] = "addRead16 ff00608a 09";
  expected[7] = "executeList::end";

  expected[8] = "executeList::begin";
  expected[9] = "addRead16 ff006088 09";
  expected[10] = "executeList::end";

  CPPUNIT_ASSERT( expected == record );
}


void CMxDCRCBusTests::get_0() 
{
  // provide dev number, address
  //m_pHdwr->Get(*m_pCtlr, "d7a16");

  auto record = m_pCtlr->getOperationRecord();

  vector<string> expected(4);
}

