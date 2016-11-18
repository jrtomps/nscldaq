// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include <Asserts.h>
#include <CStatusSubscription.h>
#include <CStatusMessage.h>
#include <zmq.hpp>
#include <unistd.h>

static const char* uri="inproc://testing";

class SubTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(SubTests);
  CPPUNIT_TEST(suball);
  CPPUNIT_TEST(subone);
  CPPUNIT_TEST(oneTypeOneSev);
  CPPUNIT_TEST(allTypesOneSev);
  CPPUNIT_TEST(allFromApp);
  CPPUNIT_TEST_SUITE_END();


private:
  zmq::context_t*   m_pContext;
  zmq::socket_t*    m_pSender;
  zmq::socket_t*    m_pReceiver;
  
  CStatusSubscription* m_pObject;
public:
  void setUp() {
    m_pContext = &CStatusDefinitions::ZmqContext::getInstance();
    m_pSender  = new zmq::socket_t(*m_pContext, ZMQ_PUB);  // Sender publishes.
    m_pReceiver= new zmq::socket_t(*m_pContext, ZMQ_SUB);  // Receiver subscribes.
    
    // Connect the subscriber to the publisher:
    
    m_pSender->bind(uri);
    m_pReceiver->connect(uri);
    
    // Make the test object:
    
    m_pObject = new CStatusSubscription(*m_pReceiver);
    
  }
  void tearDown() {
    
    delete m_pObject;
    
    delete m_pReceiver;
    delete m_pSender;
    CStatusDefinitions::ZmqContext::reset();
  }
protected:
  void suball();
  void subone();
  void oneTypeOneSev();
  
  void allTypesOneSev();
  void allFromApp();
private:
  void checkLogMessage(uint32_t sev, const char* msg);
  void checkStateChange(const char* leaving, const char* entering);
};

CPPUNIT_TEST_SUITE_REGISTRATION(SubTests);


/*--------------------------- Utilities --------------------------------*/

void
SubTests::checkLogMessage(uint32_t sev, const char* msg)
{
  // Should get two Message parts.:
  
  zmq::message_t hdr;
  zmq::message_t body;
  uint64_t       more(0);
  size_t         s(sizeof(more));
  
  m_pReceiver->recv(&hdr);
  m_pReceiver->getsockopt(ZMQ_RCVMORE, &more, &s);
  ASSERT(more);
  
  
  m_pReceiver->recv(&body);
  m_pReceiver->getsockopt(ZMQ_RCVMORE, &more, &s);
  ASSERT(!more);
  
  // Make sure the header is a log message with the stated severity.
  
  CStatusDefinitions::Header* pHeader =
    reinterpret_cast<CStatusDefinitions::Header*>(hdr.data());
    
  EQ(CStatusDefinitions::MessageTypes::LOG_MESSAGE, pHeader->s_type);
  EQ(sev, pHeader->s_severity);
  
  // Make sure the body has the specified log message.
  
  CStatusDefinitions::LogMessageBody* pBody =
    reinterpret_cast<CStatusDefinitions::LogMessageBody*>(body.data());
  EQ(std::string(msg), std::string(pBody->s_message));
}
void
SubTests::checkStateChange(const char* leaving, const char* entering)
{
  // Should get two message parts:
  
  zmq::message_t hdr;
  zmq::message_t body;
  uint64_t       more(0);
  size_t         s(sizeof(more));
  
  m_pReceiver->recv(&hdr);
  m_pReceiver->getsockopt(ZMQ_RCVMORE, &more, &s);
  ASSERT(more);
  
  
  m_pReceiver->recv(&body);
  m_pReceiver->getsockopt(ZMQ_RCVMORE, &more, &s);
  ASSERT(!more);

  // Header should have a type of STATE_CHANGE with severity INFO:
  
  CStatusDefinitions::Header* pHeader =
    reinterpret_cast<CStatusDefinitions::Header*>(hdr.data());
  EQ(CStatusDefinitions::MessageTypes::STATE_CHANGE, pHeader->s_type);
  EQ(CStatusDefinitions::SeverityLevels::INFO, pHeader->s_severity);

  
  // Body should have the appropriate state change
  
  CStatusDefinitions::StateChangeBody* pBody =
    reinterpret_cast<CStatusDefinitions::StateChangeBody*>(body.data());
  EQ(std::string(leaving), std::string(pBody->s_leaving));
  EQ(std::string(entering), std::string(pBody->s_entering));
}

/*------------------------------------------  Tests --------------------------*/
// If I subscribe to all things I should get all things.

void SubTests::suball() {
  CStatusSubscription::RequestedTypes types;
  CStatusSubscription::RequestedSeverities sevs;
  m_pObject->subscribe(types, sevs);
  
  // Send some messages with different types and severities:
  
  CStatusDefinitions::LogMessage log(*m_pSender, "TestApp");
  CStatusDefinitions::StateChange sc(*m_pSender, "TestApp");
  
  // Send/receive a log message (INFO):
  
  log.Log(CStatusDefinitions::SeverityLevels::INFO, "Test1");
  checkLogMessage(CStatusDefinitions::SeverityLevels::INFO, "Test1");
  
  // Send/receive a log message (Warning):
  
  log.Log(CStatusDefinitions::SeverityLevels::WARNING, "Test2");
  checkLogMessage(CStatusDefinitions::SeverityLevels::WARNING, "Test2");
  
  // Send a statechange message:
  
  sc.logChange("NotReady", "Readying");
  checkStateChange("NotReady", "Readying");
  
}

// If I subscribe only to e.g. Log messages, I should miss the other
// message types:

void SubTests::subone()
{
  CStatusSubscription::RequestedTypes types = {CStatusDefinitions::MessageTypes::LOG_MESSAGE};
  CStatusSubscription::RequestedSeverities sevs;
  
  m_pObject->subscribe(types, sevs);
  
  CStatusDefinitions::LogMessage log(*m_pSender, "TestApp");
  CStatusDefinitions::StateChange sc(*m_pSender, "TestApp");
  
  log.Log(CStatusDefinitions::SeverityLevels::INFO, "Test1");
  checkLogMessage(CStatusDefinitions::SeverityLevels::INFO, "Test1");
  
  sc.logChange("NotReady", "Readying");    // Should not receive this so:
  
  log.Log(CStatusDefinitions::SeverityLevels::WARNING, "Test2");
  checkLogMessage(CStatusDefinitions::SeverityLevels::WARNING, "Test2");
}
// Subscribe to exactly one type and one severity:

void SubTests::oneTypeOneSev()
{
  CStatusSubscription::RequestedTypes types = {CStatusDefinitions::MessageTypes::LOG_MESSAGE};
  CStatusSubscription::RequestedSeverities sevs = {CStatusDefinitions::SeverityLevels::INFO};
  
  m_pObject->subscribe(types, sevs);
  
  CStatusDefinitions::LogMessage log(*m_pSender, "TestApp");
  CStatusDefinitions::StateChange sc(*m_pSender, "TestApp");
  
  // Should get this
  
  log.Log(CStatusDefinitions::SeverityLevels::INFO, "Test1");
  checkLogMessage(CStatusDefinitions::SeverityLevels::INFO, "Test1");  
  
  
  // Should not get these
  
  sc.logChange("NotReady", "Readying");    // Should not receive this so:
  log.Log(CStatusDefinitions::SeverityLevels::WARNING, "Test2");
  
  // Should get this:
  
  log.Log(CStatusDefinitions::SeverityLevels::INFO, "Testing");
  checkLogMessage(CStatusDefinitions::SeverityLevels::INFO, "Testing");
}

// No types but a severity level should result in multiple subscriptions that
// will give all types but only one severity level:

void SubTests::allTypesOneSev()
{
  CStatusSubscription::RequestedTypes types;
  CStatusSubscription::RequestedSeverities sevs = {CStatusDefinitions::SeverityLevels::INFO};
  
  m_pObject->subscribe(types, sevs);
  
  CStatusDefinitions::LogMessage log(*m_pSender, "TestApp");
  CStatusDefinitions::StateChange sc(*m_pSender, "TestApp");
  
  // Should get this
  
  log.Log(CStatusDefinitions::SeverityLevels::INFO, "Test1");
  checkLogMessage(CStatusDefinitions::SeverityLevels::INFO, "Test1");
  
  // Should get this too:
  
  sc.logChange("NotReady", "Readying");
  checkStateChange("NotReady", "Readying");
  
  // Should not get this (wrong severity):
  
  log.Log(CStatusDefinitions::SeverityLevels::WARNING, "A warning");
  
  // Should get this again:
  
  sc.logChange("Readying", "Ready");
  checkStateChange("Readying", "Ready");
}
//  Accept all messages from an app:

void SubTests::allFromApp()
{
  CStatusSubscription::RequestedTypes types;
  CStatusSubscription::RequestedSeverities sevs;

  
  
  m_pObject->subscribe(types, sevs, "TestApp");
  
  CStatusDefinitions::LogMessage log2(*m_pSender, "WrongApp"); 
  // Should not get this
  
  log2.Log(CStatusDefinitions::SeverityLevels::INFO, "Test1");
  
  // Should  get these
  
  CStatusDefinitions::LogMessage log(*m_pSender, "TestApp");
  CStatusDefinitions::StateChange sc(*m_pSender, "TestApp");
  
  // Should not get this:
  
  log2.Log(CStatusDefinitions::SeverityLevels::INFO, "should not get this");
  
  // Should get this:
  
  log.Log(CStatusDefinitions::SeverityLevels::DEBUG, "Should Get This");
  checkLogMessage(CStatusDefinitions::SeverityLevels::DEBUG, "Should Get This");
  
  
    
  // Should get this too:
  
  sc.logChange("From", "to");
  checkStateChange("From", "to");
  
 
}