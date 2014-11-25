

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include <vector>
#include <string>
#include <memory>
#include <iterator>
#include <algorithm>
#include <CMockVMUSB.h>
#include <CLoggingReadoutList.h>
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
    CPPUNIT_TEST(set_0);
    CPPUNIT_TEST(readResponse_0);
    CPPUNIT_TEST(parseAddress_0);
    CPPUNIT_TEST(addParameterWrite_0);
    CPPUNIT_TEST(addParameterRead_0);
    CPPUNIT_TEST(responseIndicatesError_0);
    CPPUNIT_TEST(responseIndicatesError_1);
    CPPUNIT_TEST(convertResponseToErrorString_0);
    CPPUNIT_TEST(convertResponseToErrorString_1);
    CPPUNIT_TEST(convertResponseToErrorString_2);
    CPPUNIT_TEST(get_0);
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

  void readResponse_0();
  void parseAddress_0();
  void addParameterWrite_0();
  void addParameterRead_0();
  void responseIndicatesError_0();
  void responseIndicatesError_1();
  void convertResponseToErrorString_0();
  void convertResponseToErrorString_1();
  void convertResponseToErrorString_2();

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

  CPPUNIT_ASSERT(expected == record);
}


void CMxDCRCBusTests::set_0() 
{
  // fill the return data
  vector<uint16_t> returnData0 = {1};
  vector<uint16_t>& buffer0 = m_pCtlr->createReturnDataStructure();
  buffer0.insert(buffer0.end(), returnData0.begin(), returnData0.end());

  vector<uint16_t> returnData1= {48};
  vector<uint16_t>& buffer1 = m_pCtlr->createReturnDataStructure();
  buffer1.insert(buffer1.end(), returnData1.begin(), returnData1.end());

  vector<uint16_t> returnData2= {48};
  vector<uint16_t>& buffer2 = m_pCtlr->createReturnDataStructure();
  buffer2.insert(buffer2.end(), returnData2.begin(), returnData2.end());

  // provide dev number, address
  CMxDCRCBus* hdwr = static_cast<CMxDCRCBus*>(m_pModule->getHardware());
  string result = hdwr->Set(*m_pCtlr, "d7a24", "48");
  cout << "set_0 result=" << result << endl;

  auto record = m_pCtlr->getOperationRecord();

  vector<string> expected(12);
  expected[0] = "executeList::begin";
  expected[1] = "addWrite16 ff006082 09 7";
  expected[2] = "addWrite16 ff006084 09 16";
  expected[3] = "addWrite16 ff006086 09 24";
  expected[4] = "addWrite16 ff006088 09 48";
  expected[5] = "executeList::end";
  // then read for data as single shot commands..
  expected[6] = "executeList::begin";
  expected[7] = "addRead16 ff00608a 09";
  expected[8] = "executeList::end";

  expected[9] = "executeList::begin";
  expected[10] = "addRead16 ff006088 09";
  expected[11] = "executeList::end";

  CPPUNIT_ASSERT( expected == record );
}


void CMxDCRCBusTests::get_0() 
{
  // setup what we expect the VM-USB to return to us
  vector<uint16_t> returnData = {0,23};
  vector<uint16_t>& buffer0 = m_pCtlr->createReturnDataStructure();
  buffer0.insert(buffer0.end(), returnData.begin(), returnData.end());

  // execute the Get command
  CMxDCRCBus* hdwr = static_cast<CMxDCRCBus*>(m_pModule->getHardware());
  std::string retStr = hdwr->Get(*m_pCtlr, "d7a24");
  cout << "retStr = " << retStr << endl;

  auto record = m_pCtlr->getOperationRecord();

  vector<string> expected(12);
  // setup and start transaction
  expected[0] = "executeList::begin";
  expected[1] = "addWrite16 ff006082 09 7";  // dev number
  expected[2] = "addWrite16 ff006084 09 18"; // read operation
  expected[3] = "addWrite16 ff006086 09 24"; // param ddress
  expected[4] = "addWrite16 ff006088 09 0"; // initiate transaction
  expected[5] = "executeList::end";

  // read response
  expected[6] = "executeList::begin";
  expected[7] = "addRead16 ff00608a 09";
  expected[8] = "executeList::end";

  expected[9] = "executeList::begin";
  expected[10] = "addRead16 ff006088 09";
  expected[11] = "executeList::end";

  print_vectors(expected,record);
  
  CPPUNIT_ASSERT(expected == record);
}


void CMxDCRCBusTests::readResponse_0() 
{
  // provide dev number, address
  CMxDCRCBus* hdwr = static_cast<CMxDCRCBus*>(m_pModule->getHardware());
  hdwr->readResponse(*m_pCtlr);

  auto record = m_pCtlr->getOperationRecord();

  vector<string> expected(3);
  expected[0] = "executeList::begin";
  expected[1] = "addRead16 ff00608a 09";
  expected[2] = "executeList::end";

//  print_vectors(expected,record);

  CPPUNIT_ASSERT(expected==record);
}


void CMxDCRCBusTests::parseAddress_0()
{
  // provide dev number, address
  CMxDCRCBus* hdwr = static_cast<CMxDCRCBus*>(m_pModule->getHardware());
  auto address = hdwr->parseAddress("d123a4567");

  CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(123), address.first);
  CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(4567), address.second);
}


void CMxDCRCBusTests::addParameterWrite_0() 
{
  CMxDCRCBus* hdwr = static_cast<CMxDCRCBus*>(m_pModule->getHardware());

  // create a logging list
  CLoggingReadoutList list;

  // define the dev number (24), and address (56), some data to write
  std::pair<uint16_t,uint16_t> addresses(24,56);
  uint16_t dataToWrite = 124;

  // perform the operation
  hdwr->addParameterWrite(list, addresses, dataToWrite);

  vector<string> expected(4);
  expected[0] = "addWrite16 ff006082 09 24"; // device num
  expected[1] = "addWrite16 ff006084 09 16"; // opcode=write
  expected[2] = "addWrite16 ff006086 09 56"; // param address
  expected[3] = "addWrite16 ff006088 09 124"; // value & initiate transaction

  CPPUNIT_ASSERT(expected == list.getLog());
  
}

void CMxDCRCBusTests::addParameterRead_0() 
{
  CMxDCRCBus* hdwr = static_cast<CMxDCRCBus*>(m_pModule->getHardware());

  // create a logging list
  CLoggingReadoutList list;

  // define the dev number (24), and address (56), some data to write
  std::pair<uint16_t,uint16_t> addresses(24,56);
  uint16_t dataToWrite = 124;

  // perform the operation
  hdwr->addParameterRead(list, addresses);

  vector<string> expected(4);
  expected[0] = "addWrite16 ff006082 09 24"; // device number
  expected[1] = "addWrite16 ff006084 09 18"; // opcode=read
  expected[2] = "addWrite16 ff006086 09 56"; // param address
  expected[3] = "addWrite16 ff006088 09 0";  // initiate transaction

  CPPUNIT_ASSERT(expected == list.getLog());
  
}

void CMxDCRCBusTests::responseIndicatesError_0()
{
  CMxDCRCBus* hdwr = static_cast<CMxDCRCBus*>(m_pModule->getHardware());
  CPPUNIT_ASSERT_EQUAL(false, hdwr->responseIndicatesError(0));
}

void CMxDCRCBusTests::responseIndicatesError_1()
{
  CMxDCRCBus* hdwr = static_cast<CMxDCRCBus*>(m_pModule->getHardware());
  CPPUNIT_ASSERT_EQUAL(true, hdwr->responseIndicatesError(1));
}

void CMxDCRCBusTests::convertResponseToErrorString_0 () 
{
  CMxDCRCBus* hdwr = static_cast<CMxDCRCBus*>(m_pModule->getHardware());
  std::string expected = "ERROR - Address collision during last RC-bus operation";
  expected += " : code=1";
  CPPUNIT_ASSERT_EQUAL(expected, hdwr->convertResponseToErrorString(1));
}

void CMxDCRCBusTests::convertResponseToErrorString_1 () 
{
  CMxDCRCBus* hdwr = static_cast<CMxDCRCBus*>(m_pModule->getHardware());
  std::string expected = "ERROR - No response during last RC-bus operation";
  expected += " : code=2";
  CPPUNIT_ASSERT_EQUAL(expected, hdwr->convertResponseToErrorString(2));
}

void CMxDCRCBusTests::convertResponseToErrorString_2 () 
{
  CMxDCRCBus* hdwr = static_cast<CMxDCRCBus*>(m_pModule->getHardware());
  std::string expected = "ERROR - Unknown error code returned from last ";
  expected += "RC-bus operation : code=4";
  // i don't really know if multiple error bits can be specified, but 
  // I will test a condition that should be independent. For safety, I
  // will test what happens when an unknown bit is set.
  CPPUNIT_ASSERT_EQUAL(expected, hdwr->convertResponseToErrorString(4));
}
