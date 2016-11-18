// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include <os.h>

#define private public
#include "CStatusMessage.h"
#undef private

static std::string uri = "inproc://test";
static std::string app = "TestApp";

class LogTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(LogTests);
  CPPUNIT_TEST(construct);
  CPPUNIT_TEST(message);
  CPPUNIT_TEST_SUITE_END();


private:
  zmq::context_t*  m_pContext;
  zmq::socket_t*   m_pSender;
  zmq::socket_t*   m_pReceiver;
  
  CStatusDefinitions::LogMessage* m_pTestObject;
public:
  void setUp() {
    m_pContext = &CStatusDefinitions::ZmqContext::getInstance();
    
    // Create an internal push/pull pair between sender/receiver:
    
    m_pSender   = new zmq::socket_t(*m_pContext, ZMQ_PUSH);
    m_pReceiver = new zmq::socket_t(*m_pContext, ZMQ_PULL);
    
    m_pReceiver->bind(uri.c_str());
    m_pSender->connect(uri.c_str());
    
    // Create an object using the sender socket:
    
    m_pTestObject = new CStatusDefinitions::LogMessage(*m_pSender, app);
  }
  void tearDown() {
    delete m_pTestObject;
    delete m_pSender;
    delete m_pReceiver;
    CStatusDefinitions::ZmqContext::reset();
    m_pContext = 0;    
  }
protected:
  void construct();
  void message();
};

CPPUNIT_TEST_SUITE_REGISTRATION(LogTests);

// Construction should set all the appropriate stuff.

void LogTests::construct()
{
  EQ(m_pSender, &(m_pTestObject->m_socket));
  EQ(app, m_pTestObject->m_application);
}

// send /receive a message:

void LogTests::message()
{
  std::string msg("This is a message");
  m_pTestObject->Log(CStatusDefinitions::SeverityLevels::WARNING, msg);
  
  // Get the message parts:
  
  zmq::message_t header;
  zmq::message_t body;
  
  m_pReceiver->recv(&header);
  
  int64_t haveMore(0);
  size_t s(sizeof(haveMore));
  m_pReceiver->getsockopt(ZMQ_RCVMORE, &haveMore, &s);
  ASSERT(haveMore);                // Must be another part.
  
  m_pReceiver->recv(&body);
  m_pReceiver->getsockopt(ZMQ_RCVMORE, &haveMore, &s);
  ASSERT(!haveMore);               // Last part.
  
  // Analyze the parts:
  
  CStatusDefinitions::Header* pHeader =
    reinterpret_cast<CStatusDefinitions::Header*>(header.data());
    
  EQ(CStatusDefinitions::MessageTypes::LOG_MESSAGE, pHeader->s_type);
  EQ(CStatusDefinitions::SeverityLevels::WARNING, pHeader->s_severity);
  EQ(app, std::string(pHeader->s_application));
  EQ(Os::hostname(), std::string(pHeader->s_source));
  
  // Note in the message body we don't really know how to check the time
  // as there can be legitimate differences between _now_ and when the
  // message was sent.
  
  CStatusDefinitions::LogMessageBody* pBody =
    reinterpret_cast<CStatusDefinitions::LogMessageBody*>(body.data());
  
  EQ(msg, std::string(pBody->s_message));
}