// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include <Asserts.h>
#include <CRingStatusDaemon.h>   // Unit under test.
#include <CRingMaster.h>
#include <CRingBuffer.h>
#include "CStatusMessage.h"

#include <thread>
#include <zmq.hpp>
#include <os.h>
#include <TCLInterpreter.h>
#include <TCLObject.h>
#include "testutils.h"
#include <cstdint>


static const char* uri="inproc://test";

static std::vector<std::string> command(Os::getProcessCommand(getpid()));


class rsdTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(rsdTest);
  CPPUNIT_TEST(norings);
  CPPUNIT_TEST(aring);
  CPPUNIT_TEST(aringPeriodic);
  CPPUNIT_TEST(aringWithStatistics);
  CPPUNIT_TEST(coupleORings);
  CPPUNIT_TEST_SUITE_END();


private:
  zmq::context_t*   m_zmqContext;
  zmq::socket_t*    m_publisher;
  zmq::socket_t*    m_subscriber;
  
  CRingStatusDaemon* m_pDaemon;
  
  std::thread*      m_thread;                // For tests that need to run daemon.
  
public:
  void setUp() {
    killRings();
    // Setup the push/pull zmq connection between publisher and subscribder:
    
    m_zmqContext = new zmq::context_t(1);
    m_publisher  = new zmq::socket_t(*m_zmqContext, ZMQ_PUSH);
    m_subscriber = new zmq::socket_t(*m_zmqContext, ZMQ_PULL);
    
    m_subscriber->bind(uri);
    m_publisher->connect(uri);
    
    // Make the object under test.
    
    m_pDaemon = new CRingStatusDaemon(*m_publisher, 1);
    
  }
  // Any thread must be done by now;
  
  void tearDown() {
      delete m_pDaemon;
      delete m_subscriber;
      delete m_publisher;
      delete m_zmqContext;
      
      killRings();
  }
protected:
  void norings();
  void aring();
  void aringPeriodic();
  void aringWithStatistics();
  void coupleORings();
private:
  void startDaemon();
  void stopDaemon();
  
};

CPPUNIT_TEST_SUITE_REGISTRATION(rsdTest);

/*--------------------------   Helper funtions --------------------------*/


/**
 * startDaemon
 *    Start a thread running m_pDaemon.
 */
void rsdTest::startDaemon()
{
  m_thread = new std::thread([this]() {
    (*m_pDaemon)();
  });
}
/**
 * stopDaemon
 *    Request the daemon stop and join with/delete the thread.
 */
void rsdTest::stopDaemon()
{
  m_pDaemon->halt();
  m_thread->join();
  delete m_thread;
}

/*--------------------------   TESTS --------------------------------------*/


// If there are no rings, I would not expect to get any messages from the
// daemon in the first second of its life (it runs its little game right away.)
void rsdTest::norings() {
  // setup ensures there are no rings:
  
  startDaemon();
  zmq_pollitem_t item = {(void*)(*m_subscriber), -1, ZMQ_POLLIN, 0};
  int status = zmq_poll(&item, 1, 1000);    // 1/2 second.
  EQ(0, status);
  stopDaemon();
}
// With a ring, we should get a message for the ringbuffer with the ring id.

void rsdTest::aring()
{
  CRingBuffer::create("testring");
  
  startDaemon();
  
  std::vector<zmq::message_t*> message = receiveMessage(m_subscriber);
  EQ(size_t(2), message.size());
  
  // For this case we want to see that the header and body are correct:
  
  CStatusDefinitions::Header* pHeader =
    reinterpret_cast<CStatusDefinitions::Header*>(message[0]->data());
  EQ(CStatusDefinitions::MessageTypes::RING_STATISTICS, pHeader->s_type);
  EQ(CStatusDefinitions::SeverityLevels::INFO, pHeader->s_severity);
  EQ(std::string("RingStatisticsDaemon"), std::string(pHeader->s_application));
  
  CStatusDefinitions::RingStatIdentification* pId =
    reinterpret_cast<CStatusDefinitions::RingStatIdentification*>(message[1]->data());
  EQ(std::string("testring"), std::string(pId->s_ringName));
  
  freeMessage(message);
  stopDaemon();
  
}
// Status messages should come periodically:

void rsdTest::aringPeriodic()
{
  CRingBuffer::create("testring");
  startDaemon();
  
  std::vector<zmq::message_t*> message = receiveMessage(m_subscriber);
  EQ(size_t(2), message.size());
  
  freeMessage(message);
  message.clear();
  
  message = receiveMessage(m_subscriber);    // within a second in any event.
  EQ(size_t(2), message.size());
  
  stopDaemon();
  freeMessage(message);
}

// Create a ring with ourself as a producer and several consumers.
// Ship a few messages through the ring and ensure that the
// resulting statistics make sense.

void rsdTest::aringWithStatistics()
{
  CRingBuffer::create("testring");
  CRingBuffer  producer("testring", CRingBuffer::producer);
  CRingBuffer  cons1("testring");
  CRingBuffer  cons2("testring");
  CRingBuffer  cons3("testring");
  
  // Send data, cons1 gets all items, cons2, ever other, cons3 every third.
  // Note 102 is divisible by 2 and 3.
  
  std::uint8_t buffer[100];
  for (int i = 0; i < 102; i++) {
    producer.put(buffer, sizeof(buffer));
    cons1.get(buffer, sizeof(buffer), sizeof(buffer));
    
    if ((i % 2) == 0) {
      cons2.get(buffer, sizeof(buffer), sizeof(buffer));
    }
    
    if ((i % 3) == 0) {
      cons3.get(buffer, sizeof(buffer), sizeof(buffer));
    }
  }
  // Start the daemon - we should get:
  // header, ring id, producer, cons1, cons2, cons3 message parts in that order.
  
  startDaemon();
  std::vector<zmq::message_t*> message = receiveMessage(m_subscriber);
  stopDaemon();
  
  EQ(size_t(6), message.size());
  
  //Make sure this is a header:
  
  CStatusDefinitions::Header* pHeader =
    reinterpret_cast<CStatusDefinitions::Header*>(message[0]->data());
  EQ(CStatusDefinitions::MessageTypes::RING_STATISTICS, pHeader->s_type);
  
  //Make sure we have a ring id:
  //
  
  CStatusDefinitions::RingStatIdentification* pId =
    reinterpret_cast<CStatusDefinitions::RingStatIdentification*>(message[1]->data());
  EQ(std::string("testring"), std::string(pId->s_ringName));
  
  // Producer statistics:
  
  CStatusDefinitions::RingStatClient* pClient =
    reinterpret_cast<CStatusDefinitions::RingStatClient*>(message[2]->data());
  EQ(std::uint64_t(102), pClient->s_operations);
  EQ(std::uint64_t(102*sizeof(buffer)), pClient->s_bytes);
  EQ(command, marshallVector(pClient->s_command));
  
  // Cons1 - gets all messages:
  
  pClient =
    reinterpret_cast<CStatusDefinitions::RingStatClient*>(message[3]->data());
  EQ(std::uint64_t(102), pClient->s_operations);
  EQ(std::uint64_t(102*sizeof(buffer)), pClient->s_bytes);
  EQ(command, marshallVector(pClient->s_command));
  
  // Cons2 gets half the messages.
  
  pClient =
    reinterpret_cast<CStatusDefinitions::RingStatClient*>(message[4]->data());
  EQ(std::uint64_t(102/2), pClient->s_operations);
  EQ(std::uint64_t(102*sizeof(buffer)/2), pClient->s_bytes);
  EQ(command, marshallVector(pClient->s_command));
  
  // cons3 gets 1/3'd the messages.
  
  pClient =
    reinterpret_cast<CStatusDefinitions::RingStatClient*>(message[5]->data());
  EQ(std::uint64_t(102/3), pClient->s_operations);
  EQ(std::uint64_t(102*sizeof(buffer)/3), pClient->s_bytes);
  EQ(command, marshallVector(pClient->s_command));
  
  freeMessage(message);
}
// A couple of rings with a producer and client and so on.

void rsdTest::coupleORings()
{
  CRingBuffer::create("ring1");
  CRingBuffer::create("ring2");
  
  // Produce for both rings:
  
  CRingBuffer p1("ring1", CRingBuffer::producer);
  CRingBuffer p2("ring2", CRingBuffer::producer);
  
  // A consumer for each ring:
  
  CRingBuffer c1("ring1");
  CRingBuffer c2("ring2");
  
  // Send data through both rings.
  
  std::uint8_t buffer[100];
  
  for (int i =0; i < 100; i++) {
    p1.put(buffer, sizeof(buffer));
    c1.get(buffer, sizeof(buffer), sizeof(buffer));
    
    if ((i % 2) == 0) {
      p2.put(buffer, sizeof(buffer));
      c2.get(buffer, sizeof(buffer), sizeof(buffer));
    }
  }
  
  // Start the daemon get the two message we should now get and stop the daemon:
  
  startDaemon();
  std::vector<zmq::message_t*> m1 = receiveMessage(m_subscriber);
  std::vector<zmq::message_t*> m2 = receiveMessage(m_subscriber);
  stopDaemon();
  
  // M1 m2 sizes are 4.
  
  EQ(size_t(4), m1.size());
  EQ(size_t(4), m2.size());
  
  CStatusDefinitions::Header*                 pHeader;
  CStatusDefinitions::RingStatIdentification* pId;
  CStatusDefinitions::RingStatClient*         producer;
  CStatusDefinitions::RingStatClient*         consumer;
  
  // Analyze m1 (ring1):
  
  pHeader =
    reinterpret_cast<CStatusDefinitions::Header*>(m1[0]->data());
  pId     =
    reinterpret_cast<CStatusDefinitions::RingStatIdentification*>(m1[1]->data());
  producer=
    reinterpret_cast<CStatusDefinitions::RingStatClient*>(m1[2]->data());
  consumer=
    reinterpret_cast<CStatusDefinitions::RingStatClient*>(m1[3]->data());
 
 EQ(CStatusDefinitions::MessageTypes::RING_STATISTICS, pHeader->s_type);
 EQ(CStatusDefinitions::SeverityLevels::INFO, pHeader->s_severity);
 EQ(std::string("RingStatisticsDaemon"), std::string(pHeader->s_application));

 EQ(std::string("ring1"), std::string(pId->s_ringName));
 
 EQ(std::uint64_t(100), producer->s_operations);
 EQ(std::uint64_t(100*sizeof(buffer)), producer->s_bytes);
 EQ(std::uint32_t(1), producer->s_isProducer);
 EQ(command, marshallVector(producer->s_command));
 
 EQ(std::uint64_t(100), consumer->s_operations);
 EQ(std::uint64_t(100*sizeof(buffer)), consumer->s_bytes);
 EQ(std::uint32_t(0), consumer->s_isProducer);
 EQ(command, marshallVector(consumer->s_command));
 
 
  // analyze m2 (ring2):
  
 pHeader =
    reinterpret_cast<CStatusDefinitions::Header*>(m2[0]->data());
  pId     =
    reinterpret_cast<CStatusDefinitions::RingStatIdentification*>(m2[1]->data());
  producer=
    reinterpret_cast<CStatusDefinitions::RingStatClient*>(m2[2]->data());
  consumer=
    reinterpret_cast<CStatusDefinitions::RingStatClient*>(m2[3]->data());  
  
  EQ(CStatusDefinitions::MessageTypes::RING_STATISTICS, pHeader->s_type);
  EQ(CStatusDefinitions::SeverityLevels::INFO, pHeader->s_severity);
  EQ(std::string("RingStatisticsDaemon"), std::string(pHeader->s_application));
 
  EQ(std::string("ring2"), std::string(pId->s_ringName));
  
  EQ(std::uint64_t(50), producer->s_operations);
  EQ(std::uint64_t(50*sizeof(buffer)), producer->s_bytes);
  EQ(std::uint32_t(1), producer->s_isProducer);
  EQ(command, marshallVector(producer->s_command));
  
  EQ(std::uint64_t(50), consumer->s_operations);
  EQ(std::uint64_t(50*sizeof(buffer)), consumer->s_bytes);
  EQ(std::uint32_t(0), consumer->s_isProducer);
  EQ(command, marshallVector(consumer->s_command));
   
  // free the message data
  
  freeMessage(m1);
  freeMessage(m2);
}