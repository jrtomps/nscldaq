#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include <CNSCLScalerBuffer.h>
#include <buffer.h>
#include <buftypes.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

extern "C" {
extern char* strtrim(const char* str);
};

#define Assert(expr)  CPPUNIT_ASSERT(expr)
#define AssertEQMsg(msg, d1, d2) \
                    CPPUNIT_ASSERT_EQUAL_MESSAGE(msg, (int)(d1),(int)(d2))
#define TEST(fcn) CPPUNIT_TEST(fcn)

// Test NSCL control buffer structure.

class ScalerBufferTests : public CppUnit::TestFixture {
private:

  // Test Suite and test registration.

  CPPUNIT_TEST_SUITE(ScalerBufferTests);
  TEST(testConstruction);
  TEST(testTimes);
  TEST(testScalers);
  TEST(testAll);
  CPPUNIT_TEST_SUITE_END();

  CNSCLScalerBuffer* m_pBuffer;


public:
  void setUp()
  {
    m_pBuffer = new CNSCLScalerBuffer; // Default size is good enough
  }
  void tearDown()
  {
    delete m_pBuffer;
  }
protected:
  void testConstruction();
  void testTimes();
  void testScalers();
  void testAll();
};
CPPUNIT_TEST_SUITE_REGISTRATION(ScalerBufferTests);

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
ScalerBufferTests::testConstruction()
{
  // Require the header is of type SCALERBF

  bheader hdr;
  CopyOut(&hdr, m_pBuffer->getBuffer(), 0, sizeof(hdr));
  AssertEQMsg("Buffer type", SCALERBF, hdr.type);

}
//  Test setting interval times:
//
void
ScalerBufferTests::testTimes()
{
  m_pBuffer->SetEndTime(0x87654321);
  m_pBuffer->SetStartTime(0x12345678);

  sclbody Body;
  CopyOut(&Body, m_pBuffer->getBuffer(), 16, sizeof(Body));
  AssertEQMsg("End time",    0x87654321, Body.etime);
  AssertEQMsg("Start time",  0x12345678, Body.btime);
}
// Put scalers in the buffer
void
ScalerBufferTests::testScalers()
{
  vector<unsigned long> low;
  vector<unsigned long> hi;
  for(int i=0; i < 10; i++) {
    low.push_back(i);
    hi.push_back(i << 16);
  }
  m_pBuffer->PutScalerVector(low);
  m_pBuffer->PutScalerVector(hi);

  sclbody* pBody = (sclbody*)malloc(sizeof(sclbody) + 20*sizeof(long));;
  CopyOut(pBody, m_pBuffer->getBuffer(), 16, sizeof(sclbody) + 20*sizeof(long));

  for(int i=0; i < 10; i++) {
    char msg[100];
    sprintf(msg, "Low [%d]", i);
    AssertEQMsg(msg, i, pBody->scalers[i]);
    sprintf(msg, "Hi [%d]", i << 16);
    AssertEQMsg(msg, i << 16, pBody->scalers[i+10]);
  }
  free(pBody);
			  
}
//  Test formatted buffer as readout will build it:

void
ScalerBufferTests::testAll()

{
  testScalers(); 
  testTimes();
}
