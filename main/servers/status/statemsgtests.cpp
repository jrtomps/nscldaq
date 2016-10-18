// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include <os.h>
#include <zmq.hpp>


#define private public
#include "CStatusMessage.h"
#undef private


static std::string uri = "inproc://test";
static std::string app = "TestApp";

class StateTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(StateTests);
  CPPUNIT_TEST(construct);
  CPPUNIT_TEST(message);
  // CPPUNIT_TEST(leavingTooLong);
  // CPPUNIT_TEST(enteringTooLong);
  CPPUNIT_TEST_SUITE_END();


private:
  zmq::context_t*  m_pContext;
  zmq::socket_t*   m_pSender;
  zmq::socket_t*   m_pReceiver;
  CStatusDefinitions::StateChange* m_pTestObject;
public:
  void setUp() {
    m_pContext = new zmq::context_t(1);
    
    // Create an internal push/pull pair between sender/receiver:
    
    m_pSender   = new zmq::socket_t(*m_pContext, ZMQ_PUSH);
    m_pReceiver = new zmq::socket_t(*m_pContext, ZMQ_PULL);
    
    m_pReceiver->bind(uri.c_str());
    m_pSender->connect(uri.c_str());
    
    // Create an object using the sender socket:
    
    m_pTestObject = new CStatusDefinitions::StateChange(*m_pSender, app);
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
  void message();
};

CPPUNIT_TEST_SUITE_REGISTRATION(StateTests);

// Construction should fill in past elements.

void StateTests::construct() {
  EQ(m_pSender, &(m_pTestObject->m_socket));
  EQ(app, m_pTestObject->m_application);
}


// Send and receive the message:

void StateTests::message()
{
  m_pTestObject->logChange("NotReady", "Readying");
  
  // Should have two message parts:
  
  zmq::message_t hMsg;
  zmq::message_t bMsg;
  int64_t        haveMore(0);
  size_t         s(sizeof(haveMore));
  
  m_pReceiver->recv(&hMsg);
  m_pReceiver->getsockopt(ZMQ_RCVMORE, &haveMore, &s);
  ASSERT(haveMore);
  
  m_pReceiver->recv(&bMsg);
  m_pReceiver->getsockopt(ZMQ_RCVMORE, &haveMore, &s);
  ASSERT(!haveMore);
  
  // Analyze the header:
  
  CStatusDefinitions::Header* h =
    reinterpret_cast<CStatusDefinitions::Header*>(hMsg.data());
  EQ(CStatusDefinitions::MessageTypes::STATE_CHANGE, h->s_type);
  EQ(CStatusDefinitions::SeverityLevels::INFO, h->s_severity);
  EQ(app, std::string(h->s_application));
  EQ(Os::hostname(), std::string(h->s_source));
  
  // Analyze the body (ignore tod).
  
  CStatusDefinitions::StateChangeBody* pBody =
    reinterpret_cast<CStatusDefinitions::StateChangeBody*>(bMsg.data());
    
  EQ(std::string("NotReady"), std::string(pBody->s_leaving));
  EQ(std::string("Readying"), std::string(pBody->s_entering));
  
}