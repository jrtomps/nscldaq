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

  CPPUNIT_TEST_SUITE_END();

private:
  CRingBuffer* m_pProducer;
  CRingBuffer* m_pConsumer;
public:
  void setUp();
  void tearDown();

protected:
};

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
