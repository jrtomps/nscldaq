#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include <CNSCLPhysicsBuffer.h>
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

class PhysicsBufferTests : public CppUnit::TestFixture {
private:

  // Test Suite and test registration.

  CPPUNIT_TEST_SUITE(PhysicsBufferTests);
  TEST(testConstruction);
  TEST(testPutEvent);
  TEST(testRetractEvent);
  CPPUNIT_TEST_SUITE_END();

  CNSCLPhysicsBuffer* m_pBuffer;


public:
  void setUp()
  {
    m_pBuffer = new CNSCLPhysicsBuffer; // Default size is good enough
  }
  void tearDown()
  {
    delete m_pBuffer;
  }
protected:
  void testConstruction();
  void testPutEvent();
  void testRetractEvent();
};
CPPUNIT_TEST_SUITE_REGISTRATION(PhysicsBufferTests);

static void 
CopyOut(void* dest, DAQWordBuffer& src, int offset, int count)
{
  short* d = (short*)dest;
  for(int i =0; i < count/sizeof(short) ; i++) {
    *d++ = src[i+offset];
  }
}
// Test construction:
//
void
PhysicsBufferTests::testConstruction()
{
  bheader hdr;
  CopyOut(&hdr, m_pBuffer->getBuffer(), 0, sizeof(hdr));
  AssertEQMsg("Buffer Type: ", DATABF, hdr.type);

}
// Test put event:  The event will be our usual counting pattern:
void
PhysicsBufferTests::testPutEvent()
{
  DAQWordBufferPtr p = m_pBuffer->StartEvent();
  for(int i =0; i < 10; i++) {
    *p = i;
    ++p;
  }
  m_pBuffer->EndEvent(p);

  DAQWordBuffer& buf(m_pBuffer->getBuffer());
  AssertEQMsg("Event size: ", 11, buf[16]);
  for(int i =0; i < 10; i++) {
    char msg[100];
    sprintf(msg, "Event contents [%d]", i);
    AssertEQMsg(msg, i, buf[17+i]);
  }
}
// Test ability to retract an event.
void
PhysicsBufferTests::testRetractEvent()
{
  DAQWordBufferPtr p;
  p   = m_pBuffer->StartEvent();
  for(int i =0; i < 10; i++) {
    *p = i;
    ++p;
  }
  
  m_pBuffer->RetractEvent(p);
  
  //  The offset should be 16.
  
  p = m_pBuffer->getBufferPtr();
  Assert(p.GetIndex() == 16);
}





