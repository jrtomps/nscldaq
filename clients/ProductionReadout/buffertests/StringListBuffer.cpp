#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include <CNSCLStringListBuffer.h>
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

class StringListBufferTests : public CppUnit::TestFixture {
private:

  // Test Suite and test registration.

  CPPUNIT_TEST_SUITE(StringListBufferTests);
  TEST(testInsert);
  CPPUNIT_TEST_SUITE_END();

  CNSCLStringListBuffer* m_pBuffer;


public:
  void setUp()
  {
      m_pBuffer = new CNSCLStringListBuffer; // Default size is good enough
  }
  void tearDown()
  {
      delete m_pBuffer;
  }
protected:
  void testInsert();
};
CPPUNIT_TEST_SUITE_REGISTRATION(StringListBufferTests);

static void 
CopyOut(void* dest, DAQWordBuffer& src, int offset, int count)
{
  short* d = (short*)dest;
  for(int i =0; i < count/sizeof(short) ; i++) {
    *d++ = src[i+offset];
  }
}
// Pull a null terminated string from the buffer. p is modified.
//
void GetString(char* pdest, DAQWordBufferPtr& p)
{
  union {
    short word;
    char  bytes[2];
  } word;
  do {
    
    word.word = *p;
    ++p;
    *pdest++ = word.bytes[0];
    *pdest++ = word.bytes[1];
  } while(word.bytes[1] != '\0');
 
}
// Test insertion of string.
//
void
StringListBufferTests::testInsert()
{
  for(int i =0; i < 10; i++) {	// I like 10 I guess.
    char msg[100];
    sprintf(msg, "String %d", i);
    m_pBuffer->PutEntityString(string(msg));
  }

  // Test that the entity count is 10:

  bheader hdr;
  CopyOut(&hdr, m_pBuffer->getBuffer(), 0, sizeof(hdr));
  AssertEQMsg("Entity count", 10, hdr.nevt);

  // Test that the entities have the right contents.

  m_pBuffer->Seek(16, SEEK_SET); // Rewind the pointer.
  DAQWordBufferPtr p = m_pBuffer->getBufferPtr();

  for(int i =0; i < 10; i++) {
    char entity[100];
    char correct[100];
    sprintf(correct, "String %d", i);
    GetString(entity, p);
    strtrim(entity);
    AssertEQMsg(correct, 0, (strcmp(entity, correct)));
  }
  
}
