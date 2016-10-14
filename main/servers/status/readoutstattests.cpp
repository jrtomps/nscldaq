// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include <zmq.hpp>
#include "Asserts.h"

#include <os.h>

#define private public
#include "CStatusMessage.h"
#undef private

static std::string uri = "inproc://test";
static std::string app = "TestApp";

class ReadoutStatTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(ReadoutStatTests);
  CPPUNIT_TEST(construct);
  CPPUNIT_TEST(begin);
  CPPUNIT_TEST(stats);
  CPPUNIT_TEST(statsNeverBegun);
  CPPUNIT_TEST_SUITE_END();


private:
  zmq::context_t*  m_pContext;
  zmq::socket_t*   m_pSender;
  zmq::socket_t*   m_pReceiver;
  CStatusDefinitions::ReadoutStatistics* m_pTestObject;
public:
  void setUp() {
    m_pContext = new zmq::context_t(1);
    
    // Create an internal push/pull pair between sender/receiver:
    
    m_pSender   = new zmq::socket_t(*m_pContext, ZMQ_PUSH);
    m_pReceiver = new zmq::socket_t(*m_pContext, ZMQ_PULL);
    
    m_pReceiver->bind(uri.c_str());
    m_pSender->connect(uri.c_str());
    
    // Create an object using the sender socket:
    
    m_pTestObject = new CStatusDefinitions::ReadoutStatistics(*m_pSender, app);
  }
  void tearDown() {
    delete m_pTestObject;
    delete m_pSender;
    delete m_pReceiver;
    delete m_pContext;
    m_pContext = 0;
  }
protected:
  void construct();
  void begin();
  void stats();
  void statsNeverBegun();
};

CPPUNIT_TEST_SUITE_REGISTRATION(ReadoutStatTests);

// Constructing an object sets the fields appropriately.

void ReadoutStatTests::construct() {
  EQ(m_pSender, &(m_pTestObject->m_socket));
  EQ(app, m_pTestObject->m_appName);
  EQ(std::time_t(0), m_pTestObject->m_runStartTime);
  EQ(false, m_pTestObject->m_haveOpenRun);
}

// begin sends the appropriate messages and modifies the internal
// state properly.


void ReadoutStatTests::begin()
{
  m_pTestObject->beginRun(1234, "This is a title");
  
  zmq::message_t header;
  zmq::message_t beg;
  
  m_pReceiver->recv(&header);
  CStatusDefinitions::Header* pHeader =
    reinterpret_cast<CStatusDefinitions::Header*>(header.data());
  
  // Check header contents, and that there should be more stuff:
  
  EQ(CStatusDefinitions::MessageTypes::READOUT_STATISTICS, pHeader->s_type);
  EQ(CStatusDefinitions::SeverityLevels::INFO, pHeader->s_severity);
  EQ(app, std::string(pHeader->s_application));
  EQ(Os::hostname(), std::string(pHeader->s_source));
 
  int haveMore;
  size_t size;
  m_pReceiver->getsockopt(ZMQ_RCVMORE, &haveMore, &size);
  ASSERT(haveMore);
  
  // Receive the message body part:
  
  m_pReceiver->recv(&beg);
  CStatusDefinitions::ReadoutStatRunInfo* pInfo =
    reinterpret_cast<CStatusDefinitions::ReadoutStatRunInfo*>(beg.data());
    
  // Check contents and that there should be no more message parts:
  
  EQ(int64_t(m_pTestObject->m_runStartTime), pInfo->s_startTime);
  EQ(uint32_t(1234), pInfo->s_runNumber);
  EQ(std::string("This is a title"), std::string(pInfo->s_title));
  m_pReceiver->getsockopt(ZMQ_RCVMORE, &haveMore, &size);
  ASSERT(!haveMore);
  
  // Check that the haveOpenRunFlag is now true:
  
  ASSERT(m_pTestObject->m_runStartTime != 0);
  ASSERT(m_pTestObject->m_haveOpenRun);
}
// Check that statistics are properly emitted.

void ReadoutStatTests::stats()
{
  begin();                        // Starts the run and checks stuff out.
  
  // Send some statistics:
  
  m_pTestObject->emitStatistics(100, 150, 2560);
  
  // We're supposed to get a header, an ident and a stats message:
  
  zmq::message_t header;
  zmq::message_t ident;
  zmq::message_t statistics;
  
  int haveMore;
  size_t s;
  
  m_pReceiver->recv(&header);
  m_pReceiver->getsockopt(ZMQ_RCVMORE, &haveMore, &s);
  ASSERT(haveMore);
  
  m_pReceiver->recv(&ident);
  m_pReceiver->getsockopt(ZMQ_RCVMORE, &haveMore, &s);
  ASSERT(haveMore);
  
  m_pReceiver->recv(&statistics);
  m_pReceiver->getsockopt(ZMQ_RCVMORE, &haveMore, &s);
  ASSERT(!haveMore);

  // We're only going to bother to check the ident and statistics messages
  // as the header's pretty well established by now.
  
  CStatusDefinitions::ReadoutStatRunInfo* pInfo =
    reinterpret_cast<CStatusDefinitions::ReadoutStatRunInfo*>(ident.data());
  
  EQ(int64_t(m_pTestObject->m_runStartTime), pInfo->s_startTime);
  EQ(uint32_t(1234), pInfo->s_runNumber);
  EQ(std::string("This is a title"), std::string(pInfo->s_title));
  ASSERT(!haveMore);
  
  // Check the statistics:
  
  CStatusDefinitions::ReadoutStatCounters* pCounters =
    reinterpret_cast<CStatusDefinitions::ReadoutStatCounters*>(statistics.data());
  //Not going to check the elapsed time
  
  EQ(uint64_t(100), pCounters->s_triggers);
  EQ(uint64_t(150), pCounters->s_events);
  EQ(uint64_t(2560), pCounters->s_bytes);
}
// Attempting to emit a stats record without ever starting a run throws logic_error:

void ReadoutStatTests::statsNeverBegun()
{
  CPPUNIT_ASSERT_THROW(
    m_pTestObject->emitStatistics(1,2,3),
    std::logic_error
  );
}