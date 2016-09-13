/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2014.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Authors:
             Ron Fox
             Jeromy Tompkins 
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/


/**
 * @file StatisticsTests.cpp
 * @brief Tests for the statistics maintenance code.
 */
#include <stdlib.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"

// Some if not all tests are whitebox tests.

#define private public
#include <CRingBuffer.h>
#undef private


#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#include <ErrnoException.h>

#include "ringbufint.h"
#include "testcommon.h"

using namespace std;

static std::string ringName("statistics");

class StatisticsTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(StatisticsTests);
  CPPUNIT_TEST(initiallyClear);
  CPPUNIT_TEST(putCounts);
  CPPUNIT_TEST(multiItemPutCounts);

  CPPUNIT_TEST(getCounts);

  CPPUNIT_TEST(incrTest);
  CPPUNIT_TEST_SUITE_END();

private:
  CRingBuffer* m_pProducer;
  CRingBuffer* m_pConsumer;
public:
  void setUp();
  void tearDown();

protected:
  void initiallyClear();
  void putCounts();
  void multiItemPutCounts();

  void getCounts();

  void incrTest();
};

CPPUNIT_TEST_SUITE_REGISTRATION(StatisticsTests);

/**
 *  Set up the tests:
 *  - Create the ring buffer.
 *  - Attach a consumer.
 *  - Attach a producer.
 *
 */
void StatisticsTests::setUp()
{
  CRingBuffer::create(ringName);
  m_pProducer = new CRingBuffer(ringName, CRingBuffer::producer);
  m_pConsumer = new CRingBuffer(ringName, CRingBuffer::consumer);
}
/**
 * Test teardown
 *   Destroy producer, consumer and ring:
 */
void StatisticsTests::tearDown()
{
  delete m_pProducer;
  delete m_pConsumer;
  CRingBuffer::remove(ringName);
}

// Test that the counters are initially zeroed

void
StatisticsTests::initiallyClear()
{
  EQ(uint64_t(0),  uint64_t(m_pProducer->m_pClientInfo->s_transfers));
  EQ(uint64_t(0),  uint64_t(m_pProducer->m_pClientInfo->s_bytes));

  EQ(uint64_t(0), uint64_t(m_pConsumer->m_pClientInfo->s_transfers));
  EQ(uint64_t(0), uint64_t(m_pConsumer->m_pClientInfo->s_bytes));
}


// Put increments the counters.

void
StatisticsTests::putCounts()
{
  char buffer[1000];		// Just some randomized bytes.
  m_pProducer->put(buffer, 1000);

  EQ(uint64_t(1), uint64_t(m_pProducer->m_pClientInfo->s_transfers));
  EQ(uint64_t(1000), uint64_t(m_pProducer->m_pClientInfo->s_bytes));
}

// Put with explicit item count is correct too:

void
StatisticsTests::multiItemPutCounts()
{
  char buffer[1000];;
  m_pProducer->put(buffer, 1000, 100, 256);
  EQ(uint64_t(256),  uint64_t(m_pProducer->m_pClientInfo->s_transfers));
}

// Get counts statistics

void
StatisticsTests::getCounts()
{
  char buffer[1000];

  m_pProducer->put(buffer, 1000);
  m_pConsumer->get(buffer, 100);

  EQ(uint64_t(1), uint64_t(m_pConsumer->m_pClientInfo->s_transfers));
  EQ(uint64_t(100), uint64_t(m_pConsumer->m_pClientInfo->s_bytes));

  m_pConsumer->get(buffer, 200);
  EQ(uint64_t(2), uint64_t(m_pConsumer->m_pClientInfo->s_transfers));
  EQ(uint64_t(300), uint64_t(m_pConsumer->m_pClientInfo->s_bytes));

}

// Incr works.

void
StatisticsTests::incrTest()
{
  m_pConsumer->incrTransferCount(100);

  EQ(uint64_t(100), uint64_t(m_pConsumer->m_pClientInfo->s_transfers));
}
