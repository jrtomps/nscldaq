#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include <CNSCLOutputBuffer.h>
#include <buffer.h>
#include <string.h>

#define Assert(expr)  CPPUNIT_ASSERT(expr)
#define AssertEQMsg(msg, d1, d2) \
                    CPPUNIT_ASSERT_EQUAL_MESSAGE(msg, (int)d1,(int)d2)
#define TEST(fcn) CPPUNIT_TEST(fcn)

// Test generic NSCL buffer generating class.

class OutputBufferTests : public CppUnit::TestFixture {

  // Test suite and test registration.

  CPPUNIT_TEST_SUITE(OutputBufferTests);
  CPPUNIT_TEST(testConstruction);
  CPPUNIT_TEST(testComputeSize);
  CPPUNIT_TEST(testSetType);
  CPPUNIT_TEST(testChecksum);
  CPPUNIT_TEST(testEntityPrimitive);
  CPPUNIT_TEST(testEntityAdvanced);
  CPPUNIT_TEST(testHeaderSet);
  CPPUNIT_TEST(testLongs);
  TEST(testBlockWords);
  TEST(testString);
  TEST(testSeek);
  CPPUNIT_TEST_SUITE_END();

  // Data for the tests:

  CNSCLOutputBuffer* m_pBuffer;	// An output buffer.
  
  // Common setup and teardown:

public:

  void setUp()
  {
    m_pBuffer = new CNSCLOutputBuffer; // Default size is good enough
  }
  void tearDown()
  {
    delete m_pBuffer;
  }

  // The tests:

protected:
  void testConstruction();
  void testComputeSize();
  void testSetType();
  void testChecksum();
  void testEntityPrimitive();
  void testEntityAdvanced();
  void testHeaderSet();
  void testLongs();
  void testBlockWords();
  void testString();
  void testSeek();
private:
  static void CopyOut(void* dest, DAQWordBuffer& src, int offset, int count);
  void CopyHeader(bheader* hdr) {
    CopyOut(hdr, m_pBuffer->getBuffer(), 0, sizeof(bheader));
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(OutputBufferTests);

// utilities

void 
OutputBufferTests::CopyOut(void* dest, DAQWordBuffer& src, int offset, int count)
{
  short* d = (short*)dest;
  for(int i =0; i < count/sizeof(short) ; i++) {
    *d++ = src[i+offset];
  }
}
  // test implementations.

// Does construction yield the correct fields?

void
OutputBufferTests::testConstruction()
{
  // Check simple statistics.

  Assert(m_pBuffer->getWords() == 4096);	// Default buffer size.
  Assert(m_pBuffer->getEntityCount() == 0);    // Nothing in the buffer yet.
  Assert(m_pBuffer->getBufferPtr().GetIndex() == 16); // Cursor after header.

  DAQWordBuffer& buf(m_pBuffer->getBuffer());

  // Pull header out into struct.

  bheader header;
  CopyHeader(&header);

  Assert(header.nwds == 0);
  Assert(header.type == 0);
  Assert(header.cks  == 0);
  Assert(header.seq  == m_pBuffer->getSequence());
  Assert(header.nevt == 0);
  Assert(header.buffmt == 5);	// Format 5 for now.
  Assert(header.ssignature == 0x0102);
  Assert(header.lsignature == 0x01020304);
}


// Does compute size work (tests PutWord along the way):

void
OutputBufferTests::testComputeSize()
{
  m_pBuffer->ComputeSize();	// Should be 16 words now.
  bheader header;
  CopyHeader(&header);
  Assert(header.nwds == 16);

  // Add a few words to the buffer:

  for(int i = 0; i < 100; i++) {
    m_pBuffer->PutWord(i);
  }
  m_pBuffer->ComputeSize();	// Update size.


  // See if the words are there:

  struct {
    bheader header;
    short   data[100];
  } UsedData;
  char message[1000];

  CopyOut(&UsedData, m_pBuffer->getBuffer(), 0, sizeof(UsedData));
  for(int i = 0; i < 100 ; i++) {
    sprintf(message, "Body offset %d sb %d is %d",
	    i,i,UsedData.data[i]);
    AssertEQMsg(message, i, UsedData.data[i]);
  }

  // See if size computes correctly:

  Assert(UsedData.header.nwds == 16 + 100);

}
//  Test set type:
void
OutputBufferTests::testSetType()
{
  m_pBuffer->SetType(1);	// Turn it into a data buffer.
  bheader header;
  CopyHeader(&header);
  Assert(header.type == 1);

  m_pBuffer->SetType(2);	// Turn it into a scaler buffer.
  CopyHeader(&header);
  Assert(header.type == 2);

}

//  Test checksum.
//    The checksum must sum the buffer to zero and the
//    value should be order independent:
//
void
OutputBufferTests::testChecksum()
{
  for(int i =0; i < 100; i ++) {
    m_pBuffer->PutWord(i);
  }
  m_pBuffer->ComputeSize();
  m_pBuffer->ComputeChecksum();

  bheader header;
  CopyHeader(&header);
  short cksum1 = header.cks;

  // Fill in another buffer in reveres order.

  delete m_pBuffer;
  m_pBuffer = new CNSCLOutputBuffer; // teardown will destroy this.

  for(int i = 99; i >= 0; i--) {
    m_pBuffer->PutWord(i);
  }
  m_pBuffer->ComputeSize();
  m_pBuffer->ComputeChecksum();

  CopyHeader(&header);
  short cksum2 = header.cks;


  Assert(cksum1 == cksum2);	// Checksums must agree.

  // And buffer must sum to zero:

  DAQWordBuffer& buf(m_pBuffer->getBuffer());
  short sum(0);
  for(int i =0; i < sizeof(bheader)/sizeof(short) + 100; i++) {
    sum += buf[i];
  }
  Assert(sum == 0);

  
  
}
//  Tests the Start/End Entity functions.
//  These should just increment the entity count.
//
void
OutputBufferTests::testEntityPrimitive()
{
  // Put 10 entities in the buffer:

  for(int entity = 0; entity < 10; entity++) {
    DAQWordBufferPtr p = m_pBuffer->StartEntity();
    for(int i =0; i < 10; i++) { // Each entity is 10 words:
      *p = i;
      ++p;
    }
    m_pBuffer->EndEntity(p);
  }
  bheader header;
  CopyHeader(&header);
  Assert(header.nevt == 10);
}
//  Test the advanced entity functions:
//   PutEntity, EntityFits
//
void
OutputBufferTests::testEntityAdvanced()
{
  int n(0);
  short entity[10];
  for(int i =0; i < 10; i++) entity[i] = i;

  while(m_pBuffer->EntityFits(10)) {
    n++;
    m_pBuffer->PutEntity(entity, 10);
  }
  // Make sure the right number of entities got in the buffer:

  Assert(n == (4096-16)/10);	// body size / entity size truncated.
  bheader header;
  CopyHeader(&header);
  Assert(n == header.nevt);	// Make sure we both counted the same.
}
//  Test ability to set header words.
void
OutputBufferTests::testHeaderSet()
{
  m_pBuffer->SetCpuNum(1);	//They're just numbers after all.
  m_pBuffer->SetNbitRegisters(2);
  m_pBuffer->SetLamRegisters(3);
  m_pBuffer->SetRun(4);

  bheader header;
  CopyHeader(&header);
  Assert(header.cpu == 1);
  Assert(header.nbit == 2);
  Assert(header.nlam == 3);
  Assert(header.run  == 4);

}
// Test long word ordering:

void
OutputBufferTests::testLongs()
{
  for(int i =0; i < 100; i++ ) {
    m_pBuffer->PutLong(i);
  }
  long body[100];
  CopyOut(body, m_pBuffer->getBuffer(), 16, sizeof(body));
  
  for(int i =0; i < 100; i++) {
    char msg[100];
    sprintf(msg, "Long mismatch [%d] sb: %d was: %d",
	    i,i, body[i]);

    // long words, so can't use  our abbrev. 

    CPPUNIT_ASSERT_EQUAL_MESSAGE(msg,(long)i,body[i]);
  }
}
// test block put of words:

void
OutputBufferTests::testBlockWords()
{
  unsigned short words[10];
  for(int i= 0; i < 10; i++) {
    words[i]= i;
  }
  m_pBuffer->PutWords(words, sizeof(words)/sizeof(short));
  
  bheader hdr;

  m_pBuffer->ComputeSize();
  m_pBuffer->ComputeChecksum();
  CopyHeader(&hdr);


  Assert(hdr.nwds == (10 + 16));	// Header + 10 words I put in.
  CopyOut(words, m_pBuffer->getBuffer(), 16, sizeof(words));  
  for(int i =0; i< 10; i++) {
    char msg[100];
    sprintf(msg, "word mismatch [%d] sb %d was %d",
	    i,i,words[i]);
    AssertEQMsg(msg, i, words[i]);
  }
}
//  Test put string.
void
OutputBufferTests::testString()
{
  const char* data = "This is a test string"; // Odd size!!!
  m_pBuffer->PutString(data);
  char buffer[strlen(data) + 2];
  CopyOut(buffer, m_pBuffer->getBuffer(), 16, strlen(data) + 2);
  Assert(strcmp(buffer, data) == 0);
  
}
// Test seeks.
void
OutputBufferTests::testSeek()
{
  // Put out 100 words:

  for(int i = 0; i < 100; i++) {
    m_pBuffer->PutWord(i);
  }
  // Check size:
  m_pBuffer->ComputeSize();
  bheader hdr;
  CopyHeader(&hdr);
  Assert(hdr.nwds == (100 + 16));
  DAQWordBufferPtr ptr = m_pBuffer->getBufferPtr();

  // Seek backwards 10:

  m_pBuffer->Seek(-10, SEEK_END);
  m_pBuffer->ComputeSize();
  CopyHeader(&hdr);
  Assert(hdr.nwds == (4095 - 10));

  // Seek absolute to 50:

  m_pBuffer->Seek(50, SEEK_SET);
  m_pBuffer->ComputeSize();
  CopyHeader(&hdr);
  Assert(hdr.nwds == 50);

  // Seek back 10 more from current:

  m_pBuffer->Seek(-10, SEEK_CUR);
  m_pBuffer->ComputeSize();
  CopyHeader(&hdr);
  Assert(hdr.nwds == 40);

  // Seek back to pointer:

  m_pBuffer->Seek(ptr);
  m_pBuffer->ComputeSize();
  CopyHeader(&hdr);
  Assert(hdr.nwds == (16 + 100)); 
}

