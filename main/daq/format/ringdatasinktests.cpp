// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"

#include "CRingDataSink.h"
#include <CRingBuffer.h>
#include <string>
#include <string.h>

static std::string ringName("myring");

class RingDataSinkTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(RingDataSinkTests);
  CPPUNIT_TEST(puttest);
  CPPUNIT_TEST_SUITE_END();


private:
CRingBuffer* m_pConsumer;

public:
  void setUp() {
    // Get rid of any prior instance:
    try {
        CRingBuffer::remove(ringName);
    } catch(...) {}
    
    CRingBuffer::create(ringName);
    m_pConsumer = new CRingBuffer(ringName);
  }
  void tearDown() {

    delete m_pConsumer;
    CRingBuffer::remove(ringName);
  }
protected:
  void puttest();
};

CPPUNIT_TEST_SUITE_REGISTRATION(RingDataSinkTests);

void RingDataSinkTests::puttest() {
 
    CRingDataSink sink(ringName);
    const char* pData="This is a test";
    sink.put(pData, strlen(pData) +1);
    
    char buffer[1000];
    size_t nBytes = m_pConsumer->get(buffer, strlen(pData) +1, 10);  // Small timeout
    EQ(strlen(pData) + 1, nBytes);
    EQ(0, strcmp(buffer, pData));
}
