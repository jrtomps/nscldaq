#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include <CNSCLControlBuffer.h>
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

class ControlBufferTests : public CppUnit::TestFixture {
private:

  // Test Suite and test registration.

  CPPUNIT_TEST_SUITE(ControlBufferTests);
  TEST(testConstruction);
  TEST(testTitle);
  TEST(testMyTime);
  TEST(testSysTime);
  CPPUNIT_TEST_SUITE_END();

  // data:

  CNSCLControlBuffer* m_pBuffer;


public:
  void setUp()
  {
    m_pBuffer = new CNSCLControlBuffer; // Default size is good enough
  }
  void tearDown()
  {
    delete m_pBuffer;
  }
protected:
  void testConstruction();
  void testTitle();
  void testMyTime();
  void testSysTime();
};

CPPUNIT_TEST_SUITE_REGISTRATION(ControlBufferTests);

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
ControlBufferTests::testConstruction()
{
  m_pBuffer->ComputeSize();
  bheader hdr;
  CopyOut(&hdr, m_pBuffer->getBuffer(), 0, sizeof(hdr));

  char msg[100];
  AssertEQMsg("Buffer size mismatch: ", hdr.nwds, wordSizeof(hdr) + wordSizeof(ctlbody));


}

/// Test ability to put the title into the buffer:

void
ControlBufferTests::testTitle()
{
  const char* title = "This is a test title!"; // odd length!!
  m_pBuffer->PutTitle(title);

  struct ctlbody body;

  CopyOut(&body,m_pBuffer->getBuffer(), 16, sizeof(body));
  strtrim(body.title);

  Assert(strcmp(title, body.title) == 0);

}
//  Test my runtime.
void
ControlBufferTests::testMyTime()
{
  m_pBuffer->PutTimeOffset(1234);
  ctlbody body;
  CopyOut(&body, m_pBuffer->getBuffer(), 16, sizeof(body));
  AssertEQMsg("My time mismatch", 1234, body.sortim);
}
// Test the automatically generated time.  We'll just test the
// Month/day/year since we really just want to be sure the data gets
// put in the right slots of the buffer.
//
void 
ControlBufferTests::testSysTime()
{
  ctlbody body;			// System time is set in construction.
  CopyOut(&body, m_pBuffer->getBuffer(), 16, sizeof(body));
  
  tm now;
  time_t t = time(NULL);
  memcpy(&now, localtime(&t), sizeof(now));
  
  AssertEQMsg("Month: ", now.tm_mon, body.tod.month);
  AssertEQMsg("Day: ",   now.tm_mday, body.tod.day);
  AssertEQMsg("Year:",   now.tm_year + 1900, body.tod.year);
}
