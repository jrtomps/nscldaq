// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include <Asserts.h>
#include <iostream>
#include <zmq.hpp>
#include <stdexcept>
#include <os.h>
#include <testutils.h>

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
  
  CPPUNIT_TEST(addConsumer);
  
  CPPUNIT_TEST(fixedMessage);
  CPPUNIT_TEST(prodMessage);
  CPPUNIT_TEST(consumersMessage);
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
  
  void fixedMessage();
  void prodMessage();
  void consumersMessage();
};

CPPUNIT_TEST_SUITE_REGISTRATION(RingStatTests);




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
  
  // Would love to use CPPUNIT_ASSERT_THROW but some versions of CPPUNIT
  // make compilation errors for std::exception derived expected exception types.
  // therefore:
  
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
  msg.addProducer(command, 1234, 5678, 15);
  
  ASSERT(msg.m_producer);
  std::vector<std::string> msgCmd = marshallVector(msg.m_producer->s_command);
  EQ(command, msgCmd);
  EQ(uint64_t(1234), msg.m_producer->s_operations);
  EQ(uint64_t(5678), msg.m_producer->s_bytes);
  EQ(uint32_t(1),    msg.m_producer->s_isProducer);
  EQ(uint64_t(0),    msg.m_producer->s_backlog);           // producers never backlog.
  EQ(uint64_t(15),   msg.m_producer->s_pid);
}
// Adding a second producer makes a logic_error:

void RingStatTests::addSecondProducer()
{
  zmq::socket_t sock(*m_pContext, ZMQ_PUB);
  CStatusDefinitions::RingStatistics msg(sock, "TestApp");
  msg.startMessage("aring");
  
  std::vector<std::string> command = {"this", "is", "the", "command"};
  msg.addProducer(command, 1234, 5678,15);

  
  CPPUNIT_ASSERT_THROW(
    msg.addProducer(command, 1234,5678, 16),
    std::logic_error
  );
  
}


void RingStatTests::addConsumer()
{
  zmq::socket_t sock(*m_pContext, ZMQ_PUB);
  CStatusDefinitions::RingStatistics msg(sock, "TestApp");
  msg.startMessage("aring");
  
  std::vector<std::string> command = {"this", "is", "the", "command"};
  msg.addConsumer(command, 1234, 5678, 999, 10);
  
  EQ(size_t(1), msg.m_consumers.size());
  CStatusDefinitions::RingStatClient& c = *(msg.m_consumers.front());
  std::vector<std::string> cCommand = marshallVector(c.s_command);
  EQ(command, cCommand);
  EQ(uint64_t(1234), c.s_operations);
  EQ(uint64_t(5678), c.s_bytes);
  EQ(uint32_t(0), c.s_isProducer);
  EQ(uint64_t(999), c.s_backlog);
  EQ(uint64_t(10),  c.s_pid);

}
// Ensure the fixed part of a message comes through fine.  This consists
// of an appropriately sized header and identification part.

void RingStatTests::fixedMessage()
{
  std::string uri = "inproc://test";
  zmq::socket_t sender(*m_pContext, ZMQ_PUSH);
  zmq::socket_t receiver(*m_pContext, ZMQ_PULL);
  
  receiver.bind(uri.c_str());
  sender.connect(uri.c_str());
  
  // Make a message with just a header and the id part:
  
  CStatusDefinitions::RingStatistics msg(sender, "TestApp");
  msg.startMessage("testring");
  msg.endMessage();                   // Everything should be queued
  
  zmq::message_t header;
  zmq::message_t ringId;
  
  receiver.recv(&header);
  int64_t haveMore(0);     // Wheezy returns a 64 bit entitie!?!
  size_t  size(sizeof(haveMore));
  receiver.getsockopt(ZMQ_RCVMORE, &haveMore, &size);
  ASSERT(haveMore);                // Should have the ring id part.
  
  receiver.recv(&ringId);
  
  receiver.getsockopt(ZMQ_RCVMORE, &haveMore, &size);
  ASSERT(!haveMore);               // Should be all done.
  
  // Analyze the header we got:
  
  CStatusDefinitions::Header* h =
    reinterpret_cast<CStatusDefinitions::Header*>(header.data());
    
  EQ(CStatusDefinitions::MessageTypes::RING_STATISTICS, h->s_type);
  EQ(CStatusDefinitions::SeverityLevels::INFO, h->s_severity);
  std::string app = h->s_application;
  std::string src = h->s_source;
  EQ(std::string("TestApp"), app);
  EQ(Os::hostname(), src);
  
  // Analyze the ring identification - for now don't worry about the tod field.
  
  CStatusDefinitions::RingStatIdentification* pIdent  =
    reinterpret_cast<CStatusDefinitions::RingStatIdentification*>(ringId.data());
  std::string ring = pIdent->s_ringName;
  EQ(std::string("testring"), ring);
}

// Ensure that a message with a single producer gives me the right stuff:

void
RingStatTests::prodMessage()
{
  std::string uri = "inproc://test";
  zmq::socket_t sender(*m_pContext, ZMQ_PUSH);
  zmq::socket_t receiver(*m_pContext, ZMQ_PULL);
  
  receiver.bind(uri.c_str());
  sender.connect(uri.c_str());
  
  // Make a message with just a header and the id part:
  
  CStatusDefinitions::RingStatistics msg(sender, "TestApp");
  msg.startMessage("testring");
  
  // Add a producer:
  
  std::vector<std::string> command = {"dumper", "--source=tcp://localhost/fox", "this", "that"};
  msg.addProducer(command, 1234, 5678, 15);

  msg.endMessage();                   // Everything should be queued
  
  // Read the message segments from the receiver socket:

  zmq::message_t header;
  zmq::message_t ringId;
  
  
  receiver.recv(&header);
  int64_t     haveMore(0);
  size_t  size(sizeof(haveMore));
  receiver.getsockopt(ZMQ_RCVMORE, &haveMore, &size);
  ASSERT(haveMore);                // Should have the ring id part.
  
  receiver.recv(&ringId);
  
  receiver.getsockopt(ZMQ_RCVMORE, &haveMore, &size);
  ASSERT(haveMore);               // Should have a producer description.
  
  zmq::message_t prodMsg;
  receiver.recv(&prodMsg);
  receiver.getsockopt(ZMQ_RCVMORE, &haveMore, &size);
  ASSERT(!haveMore);
  
  // Analyze the result:
  
  CStatusDefinitions::RingStatClient* prod =
    reinterpret_cast<CStatusDefinitions::RingStatClient*>(prodMsg.data());
  EQ(uint64_t(1234), prod->s_operations);
  EQ(uint64_t(5678), prod->s_bytes);
  ASSERT(prod->s_isProducer);                // This is a producer.
  EQ(command, marshallVector(prod->s_command));
  EQ(uint64_t(15), prod->s_pid);
  EQ(uint64_t(0),  prod->s_backlog);
}

//  Case where there's a few consumers floating around in the message:

void
RingStatTests::consumersMessage()
{
  std::string uri = "inproc://test";
  zmq::socket_t sender(*m_pContext, ZMQ_PUSH);
  zmq::socket_t receiver(*m_pContext, ZMQ_PULL);
  
  receiver.bind(uri.c_str());
  sender.connect(uri.c_str());
  
  // Make a message with just a header and the id part:
  
  CStatusDefinitions::RingStatistics msg(sender, "TestApp");
  msg.startMessage("testring");

  std::vector<std::string> cmd1 = {"ringselector", "testring", "--sample=PHYSICS_EVENT"};
  std::vector<std::string> cmd2 = {"dumper", "--source=tcp://localhost/testring"};
  
  msg.addConsumer(cmd1, 100, 1000, 300, 10);
  msg.addConsumer(cmd2, 200, 2500, 400, 20);
  
  msg.endMessage();                      // Send the message:
  
  // Receive the message header and ring id -- this is presumed to work:
  
  zmq::message_t junk;
  receiver.recv(&junk);
  receiver.recv(&junk);
  
  // There should be more data:
  
  int64_t haveMore(0);
  size_t size(sizeof(haveMore));
  receiver.getsockopt(ZMQ_RCVMORE, &haveMore, &size);
  ASSERT(haveMore);
  
  zmq::message_t c1;                // Consumer 1:
  receiver.recv(&c1);
  
  // should be more data:
  
  receiver.getsockopt(ZMQ_RCVMORE, &haveMore, &size);
  ASSERT(haveMore);
  
  zmq::message_t c2;             // Second consumer.
  receiver.recv(&c2);
  
  // That should be the last segment.
  
  receiver.getsockopt(ZMQ_RCVMORE, &haveMore, &size);
  ASSERT(!haveMore);
  
  // Analyze consumer1:
  
  CStatusDefinitions::RingStatClient* pClient =
    reinterpret_cast<CStatusDefinitions::RingStatClient*>(c1.data());
  ASSERT(!pClient->s_isProducer);
  EQ(uint64_t(100), pClient->s_operations);
  EQ(uint64_t(1000), pClient->s_bytes);
  EQ(cmd1, marshallVector(pClient->s_command));
  EQ(uint64_t(300), pClient->s_backlog);
  EQ(uint64_t(10), pClient->s_pid);
  
  // Analyze consumer 2:
  
  pClient = reinterpret_cast<CStatusDefinitions::RingStatClient*>(c2.data());
  ASSERT(!pClient->s_isProducer);
  EQ(uint64_t(200), pClient->s_operations);
  EQ(uint64_t(2500), pClient->s_bytes);
  EQ(cmd2, marshallVector(pClient->s_command));
  EQ(uint64_t(400), pClient->s_backlog);
  EQ(uint64_t(20), pClient->s_pid);

}