#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include <CStateVariableBuffer.h>
#include <buftypes.h>
#include <buffer.h>
#include <string.h>
#include <time.h>

extern "C" {
extern char* strtrim(const char* str);
};

#define Assert(expr)  CPPUNIT_ASSERT(expr)
#define AssertEQMsg(msg, d1, d2) \
                    CPPUNIT_ASSERT_EQUAL_MESSAGE(msg, (int)(d1),(int)(d2))
#define TEST(fcn) CPPUNIT_TEST(fcn)

// Test NSCL control buffer structure.

class StateVariableBufferTests : public CppUnit::TestFixture {
private:

  // Test Suite and test registration.

  CPPUNIT_TEST_SUITE(StateVariableBufferTests);
  TEST(testConstruction);
  CPPUNIT_TEST_SUITE_END();

  // data:

  CStateVariableBuffer* m_pBuffer;


public:
  void setUp()
  {
    m_pBuffer = new CStateVariableBuffer; // Default size is good enough
  }
  void tearDown()
  {
    delete m_pBuffer;
  }
protected:
  void testConstruction();

};

CPPUNIT_TEST_SUITE_REGISTRATION(StateVariableBufferTests);

#define wordSizeof(entity) (sizeof(entity)/sizeof(short))
static void 
CopyOut(void* dest, DAQWordBuffer& src, int offset, int count)
{
  short* d = (short*)dest;
  for(int i =0; i < count/sizeof(short) ; i++) {
    *d++ = src[i+offset];
  }
}
  // test implementations.

// Test construction to correct buffersize.
void
StateVariableBufferTests::testConstruction()
{
  m_pBuffer->ComputeSize();
  bheader hdr;
  CopyOut(&hdr, m_pBuffer->getBuffer(), 0, sizeof(hdr));

  char msg[100];
  AssertEQMsg("Buffer size mismatch: ",  wordSizeof(hdr) ,    hdr.nwds);

  // Check buffer type:

  AssertEQMsg("Buffer type mismatch: ", STATEVARBF, hdr.type);

}

