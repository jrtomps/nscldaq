#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include <CNSCLEndBuffer.h>
#include <buffer.h>
#include <buftypes.h>
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

class EndBufferTests : public CppUnit::TestFixture {
private:

  // Test Suite and test registration.

  CPPUNIT_TEST_SUITE(EndBufferTests);
  TEST(testConstruction);
  CPPUNIT_TEST_SUITE_END();

  CNSCLEndBuffer* m_pBuffer;


public:
  void setUp()
  {
    m_pBuffer = new CNSCLEndBuffer; // Default size is good enough
  }
  void tearDown()
  {
    delete m_pBuffer;
  }
protected:
  void testConstruction();
};
CPPUNIT_TEST_SUITE_REGISTRATION(EndBufferTests);

static void 
CopyOut(void* dest, DAQWordBuffer& src, int offset, int count)
{
  short* d = (short*)dest;
  for(int i =0; i < count/sizeof(short) ; i++) {
    *d++ = src[i+offset];
  }
}
// Test construction:
//    Type is begin run 
//    Sor offset is 0.
//
void
EndBufferTests::testConstruction()
{
  m_pBuffer->PutTimeOffset(1234);
  struct buffer {
    bheader header;
    ctlbody body;
  } bf;
  CopyOut(&bf, m_pBuffer->getBuffer(), 0, sizeof(bf));
  AssertEQMsg("Buffer type field", ENDRUNBF, bf.header.type);
  AssertEQMsg("Buffer time offset", 1234, bf.body.sortim);
}
