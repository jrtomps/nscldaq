// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include <Asserts.h>
#include "CPublishRingStatistics.h"
#include "CStatusMessage.h"
#include <CRingBuffer.h>
#include <CRingMaster.h>
#include "TCLInterpreter.h"
#include "TCLObject.h"

#include <zmq.hpp>

#include <iostream>
#include <vector>
#include <string>
#include <cstdint>
#include <errno.h>
#include <os.h>
#include <sys/types.h>
#include <unistd.h>
#include <testutils.h>

static const char* uri="inproc://test";
static std::vector<std::string> command(Os::getProcessCommand(getpid()));



// @note - the ring master must be running for these tests to work.

class RingPubTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(RingPubTests);
  CPPUNIT_TEST(emptyring);
  CPPUNIT_TEST(ringWithProducer);
  CPPUNIT_TEST(ringWithConsumer);
  CPPUNIT_TEST(ringWithProducerAndConsumer);
  CPPUNIT_TEST(ringWithProducerSeveralConsumers);
  CPPUNIT_TEST(multipleRings);
  CPPUNIT_TEST_SUITE_END();


private:
  zmq::context_t*         m_pZmqContext;
  zmq::socket_t*          m_pSender;
  zmq::socket_t*          m_pReceiver;
  CPublishRingStatistics* m_pPublisher;
  
public:
  void setUp() {
    killRings();                       // In setup in case we start with rings.
    
    // Setup the zmq connections sender is a PUSH and receiver a PULL, and we'll
    // directly receive/analyze raw messages.
    
    m_pZmqContext = &CStatusDefinitions::ZmqContext::getInstance();
    m_pSender     = new zmq::socket_t(*m_pZmqContext, ZMQ_PUSH);
    m_pReceiver   = new zmq::socket_t(*m_pZmqContext, ZMQ_PULL);
    
    m_pSender->bind(uri);
    m_pReceiver->connect(uri);
    
    // Now we can set up the publisher
    
    m_pPublisher = new CPublishRingStatistics(*m_pSender, "Test Application");
  
  }
  void tearDown() {
    delete m_pPublisher;
    delete m_pSender;
    delete m_pReceiver;
    CStatusDefinitions::ZmqContext::reset();
    killRings();                        // no rings on exit too.
  }
protected:
  void emptyring();
  void ringWithProducer();
  void ringWithConsumer();
  void ringWithProducerAndConsumer();
  void ringWithProducerSeveralConsumers();
  void multipleRings();
private:
    std::vector<zmq::message_t*> receiveMessage();
};

CPPUNIT_TEST_SUITE_REGISTRATION(RingPubTests);

/*--------------------------- Utility methods -----------------------------*/

// Recieve a multipart message and return it.

std::vector<zmq::message_t*>
RingPubTests::receiveMessage()
{
  return ::receiveMessage(m_pReceiver);
}

/*-------------------------------------- Tests ----------------------------*/

// Make a ring buffer and publish. We should get a single message stream with
// no producers or consumers, but a ring id segment.

void RingPubTests::emptyring() {
  CRingBuffer::create("test_ring");             // Make the test ring.
  (*m_pPublisher)();                            // Publish ring info.
  
  std::vector<zmq::message_t*> message = receiveMessage();
  
  EQ(size_t(2), message.size());           // Header and ring id only.
  
  // Ensure header correctness.
  
  CStatusDefinitions::Header* pHeader =
    reinterpret_cast<CStatusDefinitions::Header*>(message[0]->data());
  EQ(CStatusDefinitions::MessageTypes::RING_STATISTICS, pHeader->s_type);
  EQ(CStatusDefinitions::SeverityLevels::INFO, pHeader->s_severity);
  EQ(std::string("Test Application"), std::string(pHeader->s_application));
  
       /* Source gets filled in autonomously and is tested elsewhere */
  
  // Ensure ring id correctness.
  
  CStatusDefinitions::RingStatIdentification* pId =
    reinterpret_cast<CStatusDefinitions::RingStatIdentification*>(message[1]->data());
  EQ(std::string("test_ring"), std::string(pId->s_ringName));
  
  
  // Done with the messages so:
  
  freeMessage(message);
  
  // Should be no more messages in the queue:
  
  zmq::message_t dummy;
  ASSERT(!m_pReceiver->recv(&dummy, ZMQ_NOBLOCK));
  EQ(EAGAIN, errno);
  

}

// Make a ring and add ourselves as a producer.  Put some data in and
// now publication should give a RingStatClient for us.  We'll have to take
// the s_command at its word as we don't know the actual command the Makefile
// uses to run us.

void RingPubTests::ringWithProducer()
{
  CRingBuffer::create("test_ring");
  CRingBuffer  producer("test_ring", CRingBuffer::producer);
  std::uint8_t buffer[100];   // Just some junk to write to the ringbuffer
  
  // 100 puts into the ring:
  
  for (int i = 0; i < 100; i++) {
    producer.put(buffer, sizeof(buffer));
  }
  
  // Publish and read the publication:
  
  (*m_pPublisher)();
  std::vector<zmq::message_t*> message = receiveMessage();
  
  // Should be three message segments and the last one should be a producer segment:
  
  EQ(size_t(3), message.size());
  
  CStatusDefinitions::RingStatClient* pClient =
    reinterpret_cast<CStatusDefinitions::RingStatClient*>(message[2]->data());
  
  EQ(uint64_t(100), pClient->s_operations);
  EQ(uint64_t(100*sizeof(buffer)), pClient->s_bytes);
  ASSERT(pClient->s_isProducer);
  EQ(command, marshallVector(pClient->s_command));
  EQ(uint64_t(0), pClient->s_backlog);
  EQ(uint64_t(getpid()), pClient->s_pid); 
  
  freeMessage(message);
}

// Make a ring with a consumer (ourself) with only a  consumer no data
// transfer can occur.

void RingPubTests::ringWithConsumer()
{
  CRingBuffer::create("test_ring");
  CRingBuffer consumer("test_ring");               // Consumer by default.
  
  (*m_pPublisher)();                               // publish.
  std::vector<zmq::message_t*> message = receiveMessage();
  
  EQ(size_t(3), message.size());
  
  CStatusDefinitions::RingStatClient* pClient =
    reinterpret_cast<CStatusDefinitions::RingStatClient*>(message[2]->data());
  
  EQ(uint64_t(0), pClient->s_operations);
  EQ(uint64_t(0), pClient->s_bytes);
  ASSERT(!pClient->s_isProducer);
  EQ(uint64_t(0), pClient->s_backlog);
  EQ(uint64_t(getpid()), pClient->s_pid);
  EQ(command, marshallVector(pClient->s_command));
  
  freeMessage(message);
}
// Make a ring with both a producer and a consumer.  Send data from one to the
// other.  Publish statistics and ensure that we have the proper info for both
// producer and consumer processes.

void RingPubTests::ringWithProducerAndConsumer()
{
  CRingBuffer::create("test_ring");
  CRingBuffer producer("test_ring", CRingBuffer::producer);
  CRingBuffer consumer("test_ring");
  
  // Exchange random bytes of data:
  
  std::uint8_t buffer[100];      // Contents don't matter we trust put/get.
  for (int i =0; i < 100; i++) {
    producer.put(buffer, sizeof(buffer));
    consumer.get(buffer, sizeof(buffer), sizeof(buffer)); // data's already there.
  }
  
  // Publish and read:
  
  (*m_pPublisher)();
  std::vector<zmq::message_t*> message = receiveMessage();
  
  
  EQ(size_t(4), message.size());
  
  // analzye the producer message (that's always first):
  
  CStatusDefinitions::RingStatClient* pProducer =
    reinterpret_cast<CStatusDefinitions::RingStatClient*>(message[2]->data());
  
  ASSERT(pProducer->s_isProducer);  
  EQ(uint64_t(100), pProducer->s_operations);
  EQ(uint64_t(100*sizeof(buffer)), pProducer->s_bytes);
  EQ(uint64_t(0), pProducer->s_backlog);
  EQ(uint64_t(getpid()), pProducer->s_pid);
  EQ(command, marshallVector(pProducer->s_command));
  
  
  CStatusDefinitions::RingStatClient* pConsumer =
    reinterpret_cast<CStatusDefinitions::RingStatClient*>(message[3]->data());
  ASSERT(!pConsumer->s_isProducer);
  EQ(uint64_t(100), pConsumer->s_operations);
  EQ(uint64_t(100*sizeof(buffer)), pConsumer->s_bytes);
  EQ(uint64_t(getpid()), pProducer->s_pid);
  EQ(uint64_t(0), pProducer->s_backlog);
  EQ(command, marshallVector(pConsumer->s_command));
  
  freeMessage(message);
}
// Several consumers to one ring.  In this case, we have a bit ring and
// not all the consumers will look at all messages.
// Note - ring buffer usage information for consumers is in order of attachement
//        for the case shown.
//
void RingPubTests::ringWithProducerSeveralConsumers()
{
  CRingBuffer::create("test_ring");
  CRingBuffer producer("test_ring", CRingBuffer::producer);
  CRingBuffer cons1("test_ring");
  CRingBuffer cons2("test_ring");
  CRingBuffer cons3("test_ring");
  
  std::uint8_t buffer[100];
  
  // 102 is exactly divisible by three and event so there's no uncertainty
  // for the number of reads done by consumers;  Note that the way this is done,
  // the consumers that don't get all items will have backlogs:
  
  for(int i = 0; i < 102; i++) {
    producer.put(buffer, sizeof(buffer));
    cons1.get(buffer, sizeof(buffer), sizeof(buffer));
    
    // Cons 2 reads for every other put (leaves data in the buffer).
    
    if ((i%2) == 0) {
      cons2.get(buffer, sizeof(buffer), sizeof(buffer));
    }
    // Cons3 every 3'd put:
    
    if ((i%3) == 0) {
      cons3.get(buffer, sizeof(buffer), sizeof(buffer));
    }
  }
  // publish and read:
  
  (*m_pPublisher)();
  std::vector<zmq::message_t*> message = receiveMessage();
  
  // Number of segments is header + id + producer + 3* consumers = 6:
  
  EQ(size_t(6), message.size());
  
  // Already trust producer... let's look at the three consumers:
  
  CStatusDefinitions::RingStatClient* pCons1 =
    reinterpret_cast<CStatusDefinitions::RingStatClient*>(message[3]->data());
  CStatusDefinitions::RingStatClient* pCons2 =
    reinterpret_cast<CStatusDefinitions::RingStatClient*>(message[4]->data());
  CStatusDefinitions::RingStatClient* pCons3 =
    reinterpret_cast<CStatusDefinitions::RingStatClient*>(message[5]->data());
  
  // Cons 1 sees all messages:
  
  EQ(std::uint64_t(102), pCons1->s_operations);
  EQ(std::uint64_t(102*sizeof(buffer)), pCons1->s_bytes);
  EQ(std::uint64_t(0), pCons1->s_backlog);             // so no backlog.
  
  // Cons 2 sees every other message:
  
  EQ(std::uint64_t(102/2), pCons2->s_operations);
  EQ(std::uint64_t(102*sizeof(buffer)/2), pCons2->s_bytes);
  EQ(std::uint64_t(102*sizeof(buffer)/2), pCons2->s_backlog);   // 1/2 the data backlogged.
  
  // Cons 3 sees every  third message:
  
  EQ(std::uint64_t(102/3), pCons3->s_operations);
  EQ(std::uint64_t(102*sizeof(buffer)/3), pCons3->s_bytes);
  EQ(std::uint64_t(102*2*sizeof(buffer)/3), pCons3->s_backlog);  // 2/3 the data backlogged.
  
  freeMessage(message);
}
// Multiple rings.. ringmaster sends data out in alpha order so we can
// predict the order for each message.

void RingPubTests::multipleRings()
{
  const char* ringNames[] = {"a", "b", "c", "d", 0};
  const char** p(ringNames);
  while (*p) {
    CRingBuffer::create(*p);
    p++;
  }
  
  (*m_pPublisher)();
  
  // Should be a message for each ring in the order of ringNames:
  
  p = ringNames;
  while(*p) {
    std::vector<zmq::message_t*> message  = receiveMessage();
    CStatusDefinitions::RingStatIdentification* pRing =
      reinterpret_cast<CStatusDefinitions::RingStatIdentification*>(message[1]->data());
    EQ(std::string(*p), std::string(pRing->s_ringName));
    
    freeMessage(message);
    p++;
  }
  
}