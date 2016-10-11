// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include <Asserts.h>
#include <iostream>
#include <zmq.hpp>

#define private public
#include "CStatusMessage.h"
#undef private

class RingStatTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(RingStatTests);
  CPPUNIT_TEST(construction);
  
  CPPUNIT_TEST(startMessage);
  CPPUNIT_TEST(startWhenOpen);
  CPPUNIT_TEST(startClearsProducer);
  
  CPPUNIT_TEST(addProducer);
  CPPUNIT_TEST(addSecondProducer);
  
  //CPPUNIT_TEST(addConsumer);
  CPPUNIT_TEST_SUITE_END();


private:
  zmq::context_t* m_pContext;
public:
  void setUp() {
    m_pContext = new zmq::context_t(1);
  }
  void tearDown() {
    delete m_pContext;
  }
protected:
  void construction();
  
  void startMessage();
  void startWhenOpen();
  void startClearsProducer();
  
  void addProducer();
  void addSecondProducer();
  
  void addConsumer();
};

CPPUNIT_TEST_SUITE_REGISTRATION(RingStatTests);


static std::vector<std::string>
marshallVector(const char* s)
{
  std::vector<std::string> result;
  while(*s) {
    result.push_back(std::string(s));
    s += strlen(s) + 1;
  }
  return result;
}
// So we can EQ on vectors e.g.
std::ostream& operator<<(std::ostream& s, const std::vector<std::string>& v)
{
  s <<  "[";
  for (int i = 0; i < v.size(); i++) {
    s << v[i];
    if (i < (v.size() -1)) s << ", ";
  }
  s << "]";
  
  return s;
}

// Construction should set the socket, the application, and clear the
// msgOpen flag.

void RingStatTests::construction() {
  zmq::socket_t sock(*m_pContext, ZMQ_PUB);
  CStatusDefinitions::RingStatistics msg(sock, "TestApp");
  
  EQ(&sock, &(msg.m_socket));
  EQ(std::string("TestApp"), msg.m_applicationName);
  EQ(false, msg.m_msgOpen);
}


// Starting a message should store the ring name.

void RingStatTests::startMessage()
{
  zmq::socket_t sock(*m_pContext, ZMQ_PUB);
  CStatusDefinitions::RingStatistics msg(sock, "TestApp");
  msg.startMessage("aring");
  
  EQ(std::string("aring"), msg.m_ringName);
  EQ(true, msg.m_msgOpen);
  
  // Need to 'adjust' the open flag to allow destruction:
  
  msg.m_msgOpen = false;
  
}
// Starting a message when the message is open throws an exception:

void RingStatTests::startWhenOpen()
{
  zmq::socket_t sock(*m_pContext, ZMQ_PUB);
  CStatusDefinitions::RingStatistics msg(sock, "TestApp");
  msg.startMessage("aring");
  
  CPPUNIT_ASSERT_THROW(
    msg.startMessage("atest"),
    std::logic_error
  );
  
  msg.m_msgOpen = false;
}
// Starting a message clears the producer flag:

void RingStatTests::startClearsProducer()
{
  zmq::socket_t sock(*m_pContext, ZMQ_PUB);
  CStatusDefinitions::RingStatistics msg(sock, "TestApp");

  msg.startMessage("aring");
  
  ASSERT(!msg.m_producer);
  
}


// Adding a  producer sets the fields appropriately.

void RingStatTests::addProducer()
{
  zmq::socket_t sock(*m_pContext, ZMQ_PUB);
  CStatusDefinitions::RingStatistics msg(sock, "TestApp");
  msg.startMessage("aring");
  
  std::vector<std::string> command = {"this", "is", "the", "command"};
  msg.addProducer(command, 1234, 5678);
  
  ASSERT(msg.m_producer);
  std::vector<std::string> msgCmd = marshallVector(msg.m_producer->s_command);
  EQ(command, msgCmd);
  EQ(uint64_t(1234), msg.m_producer->s_operations);
  EQ(uint64_t(5678), msg.m_producer->s_bytes);
  EQ(uint32_t(1),    msg.m_producer->s_isProducer);
}
// Adding a second producer makes a logic_error:

void RingStatTests::addSecondProducer()
{
  zmq::socket_t sock(*m_pContext, ZMQ_PUB);
  CStatusDefinitions::RingStatistics msg(sock, "TestApp");
  msg.startMessage("aring");
  
  std::vector<std::string> command = {"this", "is", "the", "command"};
  msg.addProducer(command, 1234, 5678);

  CPPUNIT_ASSERT_THROW(
    msg.addProducer(command, 1234, 5678),
    std::logic_error
  );
}


void RingStatTests::addConsumer()
{
  zmq::socket_t sock(*m_pContext, ZMQ_PUB);
  CStatusDefinitions::RingStatistics msg(sock, "TestApp");
  msg.startMessage("aring");
  
  std::vector<std::string> command = {"this", "is", "the", "command"};
  msg.addConsumer(command, 1234, 5678);
  
  EQ(size_t(1), msg.m_consumers.size());
  CStatusDefinitions::RingStatClient& c = *(msg.m_consumers.front());
  std::vector<std::string> cCommand = marshallVector(c.s_command);
  EQ(command, cCommand);
  EQ(uint64_t(1234), c.s_operations);
  EQ(uint64_t(5678), c.s_bytes);
  EQ(uint32_t(0), c.s_isProducer);

}