// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include <CRingBuffer.h>
#include <string.h>

using namespace std;

#ifndef SHM_TESTFILE
#define SHM_TESTFILE "whilepred"
#endif


class WhilePredTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(WhilePredTest);
  CPPUNIT_TEST(skipTest);
  CPPUNIT_TEST_SUITE_END();


private:

public:
  void setUp() {
    CRingBuffer::create(string(SHM_TESTFILE));
  }
  void tearDown() {
    CRingBuffer::remove(string(SHM_TESTFILE));
  }
protected:
  void skipTest();
};

// messages look like this

struct message {
    int size;
    int id;
    char payload[100];
};  

// Helper class:


class SkipTo : public CRingBuffer::CRingBufferPredicate
{
private:
  int m_id;
public:
  SkipTo(int id) : 
    m_id(id) {}
  virtual bool operator()(CRingBuffer& ring);
};

bool
SkipTo::operator()(CRingBuffer& ring)
{
  // peek for the message:
  message msg;
  ring.peek(&msg, sizeof(int)*2);

  // True if we have the right id and skip the message
  // and false if not.

  if (msg.id == m_id) {
    return true;
  }
  else {
    ring.skip(msg.size);
    return false;
  }
}

CPPUNIT_TEST_SUITE_REGISTRATION(WhilePredTest);

//
// 1. Attach to the ring buffer as a producer and a consumer.
//    Produce two messages of the form
//      size/id/counting pattern 
//    where size/id are longwords and counting pattern is a set of
//    bytes.
//    The first message has id = 1 the second 2.
// 2. Construct a skip predicate that skips until the message with id 2 is found.
// 3. Use While to do the skip.
// 4. Get the message and ensure it's right.
//

void WhilePredTest::skipTest() {
  CRingBuffer producer(string(SHM_TESTFILE), CRingBuffer::producer);
  CRingBuffer consumer(string(SHM_TESTFILE), CRingBuffer::consumer);

  message sendmsg, rcvmsg;

  memset(&rcvmsg, 0, sizeof(rcvmsg));

  for(int i = 0; i < sizeof(sendmsg.payload); i++) {
    sendmsg.payload[i] = i;
  }
  sendmsg.size = sizeof(message);
  sendmsg.id   = 1;

  producer.put(&sendmsg, sizeof(sendmsg)); // first message.

  sendmsg.id   = 2;
  producer.put(&sendmsg, sizeof(sendmsg)); // second one


  // Skip to message with id 2:

  SkipTo predicate(2);
  consumer.While(predicate);

  // The next message should be the second one:

  consumer.get(&rcvmsg, sizeof(message), sizeof(message));

  EQ(sizeof(message), (size_t)(rcvmsg.size));
  EQ(2, rcvmsg.id);


}
