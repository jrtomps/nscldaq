// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

#include <ringbufint.h>

#include <CRingBuffer.h>
#include <testcommon.h>

#include <string>
#include <iostream>

using namespace std;


class InfoTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(InfoTests);
  CPPUNIT_TEST(interval);
  CPPUNIT_TEST(usageempty);
  CPPUNIT_TEST(usage1consumer);
  CPPUNIT_TEST(usageconsumers);
  CPPUNIT_TEST_SUITE_END();


private:
    std::string SHM_TESTFILE;

public:
  void setUp() {
    SHM_TESTFILE = uniqueRing("infotest");
    CRingBuffer::create(string(SHM_TESTFILE));
  }
  void tearDown() {
    try {
      CRingBuffer::remove(SHM_TESTFILE);
    }
    catch (...) {}
  }
protected:
  void interval();
  void usageempty();
  void usage1consumer();
  void usageconsumers();
};

CPPUNIT_TEST_SUITE_REGISTRATION(InfoTests);

// Test the ability to set the polling interval.

void InfoTests::interval() {
  CRingBuffer ring(SHM_TESTFILE);

  unsigned long initial = ring.getPollInterval();
  unsigned long original= ring.setPollInterval(initial+2);
  EQ(initial, original);
  EQ(initial+2, ring.getPollInterval());
}


// Check the Usage for a ring buffer with only a producer.
// - The buffer space shoud match what we get from the rin gheader.
// - The put space should be the bufferspace -1.
// - The producer should be getpid(), 
// - The max/min get space entries should be 0.
// - The consumer list should be empty.
//
void InfoTests::usageempty()
{
  CRingBuffer ring(string(SHM_TESTFILE), CRingBuffer::producer);
  CRingBuffer::Usage use = ring.getUsage();

  pRingBuffer pRing = reinterpret_cast<pRingBuffer>(mapRingBuffer(SHM_TESTFILE.c_str()));
  RingHeader& header(pRing->s_header);

  size_t bufferSpace = header.s_dataBytes;
  size_t maxConsumers= header.s_maxConsumer;
  EQ(bufferSpace,   use.s_bufferSpace);
  EQ(header.s_dataBytes-1, use.s_putSpace);
  EQ(maxConsumers, use.s_maxConsumers);
  EQ(getpid(),             use.s_producer);
  EQ(size_t(0),            use.s_maxGetSpace);
  EQ(size_t(0),            use.s_minGetSpace);
  EQ(size_t(0),            use.s_consumers.size());

}
// Check the Usage for a ring buffer with a producer and a single consumer.
// We'll put a 100byte message in the ring buffer and not get it out
// to force some values other than that for an empty buffer.
// - Buffer space should still be the same as ring header.
// - put space should be bufferspace - 101
// - max/min get space should be 100.
// - Should be one consumer in the list:
//   - pid is us
//   - size available is 100.
//
void InfoTests::usage1consumer()
{
  CRingBuffer cons(SHM_TESTFILE);
  CRingBuffer prod(SHM_TESTFILE, CRingBuffer::producer);
  pRingBuffer pRing = reinterpret_cast<pRingBuffer>(mapRingBuffer(SHM_TESTFILE.c_str()));
  RingHeader& header(pRing->s_header);

  char msg[100];		// contents don' matter for this test...

  CRingBuffer::Usage u = cons.getUsage();
  EQ(header.s_dataBytes -1, u.s_putSpace);
  prod.put(msg, 100);
  
  CRingBuffer::Usage use = prod.getUsage();

  size_t bufferSpace = header.s_dataBytes;
  EQ(bufferSpace,  use.s_bufferSpace);
  EQ(header.s_dataBytes - 100 -1, use.s_putSpace);
  EQ(getpid(),            use.s_producer);
  EQ(sizeof(msg),         use.s_maxGetSpace);
  EQ(sizeof(msg),         use.s_minGetSpace);
  EQ(size_t(1),           use.s_consumers.size());
  EQ(getpid(),            use.s_consumers[0].first);
  EQ(sizeof(msg),         use.s_consumers[0].second);

  
}
void InfoTests::usageconsumers()
{
  CRingBuffer prod(SHM_TESTFILE, CRingBuffer::producer);
  CRingBuffer c1(SHM_TESTFILE);
  CRingBuffer c2(SHM_TESTFILE);
  CRingBuffer c3(SHM_TESTFILE);

  char msg[100];		// contents don' matter for this test...
  prod.put(msg, sizeof(msg));


  pRingBuffer pRing = reinterpret_cast<pRingBuffer>(mapRingBuffer(SHM_TESTFILE.c_str()));
  RingHeader& header(pRing->s_header);

  // Get different amounts from each of the consumers:
  // c1 gets nothing
  // c2 gets sizeof(msg)/4
  // c3 gets sizeof(msg)/2
  // 
  // c1 determines put_space, and maxGetSpace.
  // c3 determines minGetSpace
  // Since the assignment of consumers is linear, 
  // The consumers should be in order in the usage vector.

  c2.get(msg, sizeof(msg)/4);
  c3.get(msg, sizeof(msg)/2);

  CRingBuffer::Usage use = prod.getUsage();
  EQ(header.s_dataBytes  - sizeof(msg) -1, use.s_putSpace);
  EQ(sizeof(msg),         use.s_maxGetSpace);
  EQ(sizeof(msg) - sizeof(msg)/2, use.s_minGetSpace);
  EQ((size_t)3,           use.s_consumers.size());
  EQ(sizeof(msg),         use.s_consumers[0].second);
  EQ(sizeof(msg) - sizeof(msg)/4, use.s_consumers[1].second);
  EQ(sizeof(msg) - sizeof(msg)/2, use.s_consumers[2].second);

  // once c1 catches up, c2 determines maxGetSpace:
  // min is zero.
  c1.get(msg, sizeof(msg));

  use = prod.getUsage();

  EQ(sizeof(msg) - sizeof(msg)/4, use.s_maxGetSpace);
  EQ((size_t)0,                   use.s_minGetSpace);
}
