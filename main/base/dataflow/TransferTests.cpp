// Tests on non-blocking data transfers.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include <CRingBuffer.h>
#include <ringbufint.h>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <string.h>

#include "testcommon.h"
#include <algorithm>

using namespace std;




// Default name of shared memory special file:


static std::string SHM_TESTFILE;

static string fileName(std::string name)
{
  std::string result = name;
  result += "_12";
  return result;
}

static string getFullName()
{
  string fullname(SHM_DIRECTORY);
  fullname += "/";
  fullname += fileName(SHM_TESTFILE);
  return fullname;
}

class XferTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(XferTests);
  CPPUNIT_TEST(simpleput);
  CPPUNIT_TEST(simpleputv);
  CPPUNIT_TEST(wrapput);
  CPPUNIT_TEST(wrapputv);
  CPPUNIT_TEST(edgewrap);
  CPPUNIT_TEST(edgewrapv);
  CPPUNIT_TEST(simpleget);
  CPPUNIT_TEST(wrapget);
  CPPUNIT_TEST(edgewrapget);
  CPPUNIT_TEST(multi);
  CPPUNIT_TEST_SUITE_END();


private:

public:
  // Setup is to create a ring buffer and teardown is to destroy it:
  void setUp() {
    SHM_TESTFILE = uniqueRing("xfertest");
    CRingBuffer::create(SHM_TESTFILE);
  }
  void tearDown() {
    try {
      CRingBuffer::remove(SHM_TESTFILE);
    }
    catch (...) {}

  }
protected:
  void simpleput();
  void simpleputv();
  void wrapput();
  void wrapputv();
  void edgewrap();
  void edgewrapv();
  void simpleget();
  void wrapget();
  void edgewrapget();
  void multi();
};

CPPUNIT_TEST_SUITE_REGISTRATION(XferTests);

// Ensure that after a put the offset has advanced the amount of the put
// and the data is in the buffer.

void XferTests::simpleput() {
  CRingBuffer ring(string(SHM_TESTFILE), CRingBuffer::producer);

  pRingBuffer        pBuffer = reinterpret_cast<pRingBuffer>(mapRingBuffer(fileName(SHM_TESTFILE).c_str()));
  pClientInformation pPut    = &(pBuffer->s_producer);

  // Check the initial format of the put 'pointer'

  pid_t mypid = getpid();
  pid_t ppid = pPut->s_pid;
  EQ(mypid, ppid); 
  EQ(pBuffer->s_header.s_dataOffset, pPut->s_offset);

  // Put 100 bytes of memory... counting pattern.

  char data[100];
  for (int i=0; i < 100; i++) { 
    data[i] = i;
  }

  ring.put(data, sizeof(data));

  // The put pointer should have advanced and the data should be in the ring:

  off_t offset = pPut->s_offset;
  EQ(pBuffer->s_header.s_dataOffset + 100, offset);
  
  int index = pBuffer->s_header.s_dataOffset;
  char* p   = reinterpret_cast<char*>(pBuffer);

  for (int i =0; i < sizeof(data); i++) {
    EQ(i, (int)p[index]);
    index++;
  }
  

  munmap(pBuffer, pBuffer->s_header.s_topOffset+1);

}

// Ensure that after a putv the offset has advanced the amount of the put
// and the data is in the buffer.

void XferTests::simpleputv() {
  CRingBuffer ring(string(SHM_TESTFILE), CRingBuffer::producer);

  pRingBuffer        pBuffer = reinterpret_cast<pRingBuffer>(mapRingBuffer(fileName(SHM_TESTFILE).c_str()));
  pClientInformation pPut    = &(pBuffer->s_producer);

  // Check the initial format of the put 'pointer'

  pid_t mypid = getpid();
  pid_t ppid = pPut->s_pid;
  EQ(mypid, ppid); 
  EQ(pBuffer->s_header.s_dataOffset, pPut->s_offset);

  // Put 100 bytes of memory... counting pattern.

  char data[100];
  for (int i=0; i < 100; i++) { 
    data[i] = i;
  }

  char data2[100];
  for (int i=0; i < 100; i++) { 
    data2[i] = i;
  }

  ring.putv({{data, sizeof(data)}, {data2, sizeof(data2)}});

  // The put pointer should have advanced and the data should be in the ring:

  off_t offset = pPut->s_offset;
  EQ(pBuffer->s_header.s_dataOffset + 200, offset);
  
  int index = pBuffer->s_header.s_dataOffset;
  char* p   = reinterpret_cast<char*>(pBuffer);

  for (int i =0; i < 2*sizeof(data); i++) {
    EQ(i%100, (int)p[index]);
    index++;
  }
  

  munmap(pBuffer, pBuffer->s_header.s_topOffset+1);

}

// Test the put case for a simple wrap.  The data put
// will evenly wrap the buffer.  see edgewrap for 
// edge cases in wrapping
//

void XferTests::wrapput()
{
  CRingBuffer ring(string(SHM_TESTFILE), CRingBuffer::producer);

  pRingBuffer        pBuffer = reinterpret_cast<pRingBuffer>(mapRingBuffer(fileName(SHM_TESTFILE).c_str()));
  pClientInformation pPut    = &(pBuffer->s_producer);


  // Create the counting pattern message:

  char msg[100];
  for (int i=0; i < sizeof(msg); i++) {
    msg[i] = i;
  }
  // Set up the ring buffer so that the message will wrap at the 1/2 way point.
  //

  pRingHeader pHeader = &(pBuffer->s_header);
  off_t startOffset = pHeader->s_topOffset - sizeof(msg)/2; 
  pPut->s_offset = startOffset;

  // there are no clients so we don't have to worry about having space
  // or blocking.
  ring.put(msg, sizeof(msg));

  off_t shouldBe = pHeader->s_dataOffset +  (sizeof(msg) - sizeof(msg)/2) - 1;
  off_t isOffset = pPut->s_offset;
  EQ(shouldBe, isOffset);

  // Check the contents of the ring buffer.

  char* p = reinterpret_cast<char*>(pBuffer);

  // First 1/2:

  for (int i =0; i < sizeof(msg)/2+1; i++) {
    EQ(i, (int)p[startOffset]);
    startOffset++;
  }
  startOffset = pHeader->s_dataOffset; 
  for(int i = sizeof(msg)/2+1; i < sizeof(msg); i++) {
    EQ(i, (int)p[startOffset]);
    startOffset++;
  }

  munmap(pBuffer, pBuffer->s_header.s_topOffset+1);

}

void XferTests::wrapputv()
{
  CRingBuffer ring(string(SHM_TESTFILE), CRingBuffer::producer);

  pRingBuffer        pBuffer = reinterpret_cast<pRingBuffer>(mapRingBuffer(fileName(SHM_TESTFILE).c_str()));
  pClientInformation pPut    = &(pBuffer->s_producer);


  // Create the counting pattern message:

  char msg[100];
  for (int i=0; i < sizeof(msg); i++) {
    msg[i] = i;
  }

  char msg2[100];
  for (int i=0; i < sizeof(msg2); i++) {
    msg2[i] = i;
  }

  // Set up the ring buffer so that the message will wrap at the 1/2 way point.
  //

  pRingHeader pHeader = &(pBuffer->s_header);
  off_t startOffset = pHeader->s_topOffset - sizeof(msg)/2; 
  pPut->s_offset = startOffset;

  // there are no clients so we don't have to worry about having space
  // or blocking.
  ring.putv({{msg, sizeof(msg)}, {msg2, sizeof(msg2)}});

  off_t shouldBe = pHeader->s_dataOffset +  (sizeof(msg) - sizeof(msg)/2) + sizeof(msg2) - 1;
  off_t isOffset = pPut->s_offset;
  EQMSG("offset after put", shouldBe, isOffset);

  // Check the contents of the ring buffer.

  char* p = reinterpret_cast<char*>(pBuffer);

  // First 1/2:

  for (int i =0; i < sizeof(msg)/2+1; i++) {
    EQ(i, (int)p[startOffset]);
    startOffset++;
  }
  startOffset = pHeader->s_dataOffset; 
  for(int i = sizeof(msg)/2+1; i < sizeof(msg); i++) {
    EQ(i, (int)p[startOffset]);
    startOffset++;
  }
  for(int i = 0; i < sizeof(msg2); i++) {
    EQ(i, (int)p[startOffset]);
    startOffset++;
  }

  munmap(pBuffer, pBuffer->s_header.s_topOffset+1);

}
//
// Test a couple of cases.
//  - A put that exactly hits the end of the ring should wrap the pointer.
//  - A put that leaves one byte free should not wrap.
// The more simple wrap case has already been checked in wrapput

void XferTests::edgewrap()
{
  CRingBuffer ring(string(SHM_TESTFILE), CRingBuffer::producer);

  pRingBuffer        pBuffer = reinterpret_cast<pRingBuffer>(mapRingBuffer(fileName(SHM_TESTFILE).c_str()));
  pClientInformation pPut    = &(pBuffer->s_producer);
  pRingHeader pHeader = &(pBuffer->s_header);


  // Create the counting pattern message:

  char msg[100];
  for (int i=0; i < sizeof(msg); i++) {
    msg[i] = i;
  }
  // Set up the put offset so that putting msg will exactly fill the
  // ring.

  off_t msgStart = pHeader->s_topOffset - sizeof(msg) +1 ;
  pPut->s_offset = msgStart;

  ring.put(msg, sizeof(msg));

  EQ(pHeader->s_dataOffset, pPut->s_offset);

  // Check the message:

  char* p = reinterpret_cast<char*>(pBuffer);


  for (int i= 0; i < sizeof(msg); i++) {
    EQ(i, (int)p[msgStart]);
    msgStart++;
  }

  // Put one less and the put pointer should be at the max offset:

  msgStart = pHeader->s_topOffset - sizeof(msg);
  pPut->s_offset = msgStart;
  ring.put(msg, sizeof(msg));
  EQ(pHeader->s_topOffset, pPut->s_offset);

  munmap(pBuffer, pBuffer->s_header.s_topOffset+1);

}

//
// Test a couple of cases.
//  - A put that exactly hits the end of the ring should wrap the pointer.
//  - A put that leaves one byte free should not wrap.
// The more simple wrap case has already been checked in wrapput

void XferTests::edgewrapv()
{
  CRingBuffer ring(string(SHM_TESTFILE), CRingBuffer::producer);

  pRingBuffer        pBuffer = reinterpret_cast<pRingBuffer>(mapRingBuffer(fileName(SHM_TESTFILE).c_str()));
  pClientInformation pPut    = &(pBuffer->s_producer);
  pRingHeader pHeader = &(pBuffer->s_header);


  // Create the counting pattern message:

  char msg[100];
  char msg2[100];

  std::iota(msg, msg+sizeof(msg), 0);
  std::iota(msg2, msg2+sizeof(msg2), 0);

  // Set up the put offset so that putting msg will exactly fill the
  // ring.

  off_t msgStart = pHeader->s_topOffset - sizeof(msg) - sizeof(msg2) +1 ;
  pPut->s_offset = msgStart;

  ring.putv({{msg, sizeof(msg)}, {msg2, sizeof(msg2)}});

  EQ(pHeader->s_dataOffset, pPut->s_offset);

  // Check the message:

  char* p = reinterpret_cast<char*>(pBuffer);


  for (int i= 0; i < sizeof(msg); i++) {
    EQ(i, (int)p[msgStart]);
    msgStart++;
  }

  for (int i= 0; i < sizeof(msg2); i++) {
    EQ(i, (int)p[msgStart]);
    msgStart++;
  }


  // Put one less and the put pointer should be at the max offset:

  msgStart = pHeader->s_topOffset - sizeof(msg)- sizeof(msg2);
  pPut->s_offset = msgStart;
  ring.putv({{msg, sizeof(msg)}, {msg2, sizeof(msg2)}});
  EQ(pHeader->s_topOffset, pPut->s_offset);

  munmap(pBuffer, pBuffer->s_header.s_topOffset+1);

}
// Simple put followed by a get to ensure we can fetch the data
// back out of the ring buffer.

void XferTests::simpleget()
{
  CRingBuffer ring(string(SHM_TESTFILE), CRingBuffer::producer);
  CRingBuffer gring(string(SHM_TESTFILE), CRingBuffer::consumer);

  pRingBuffer        pBuffer = reinterpret_cast<pRingBuffer>(mapRingBuffer(fileName(SHM_TESTFILE).c_str()));
  pClientInformation pPut    = &(pBuffer->s_producer);
  pRingHeader pHeader = &(pBuffer->s_header);

  // We should be the first consumer as there are no consumers and the
  // search strategy is linear:

  pClientInformation pGet = pBuffer->s_consumers;
  pid_t pid = getpid();
  pid_t cpid = pGet->s_pid;
  EQ(pid, cpid);
  EQ(pHeader->s_dataOffset, pGet->s_offset);
  

  char putmsg[100], getmsg[100];
  memset(getmsg, 0, sizeof(getmsg));
  for (int i=0; i < sizeof(putmsg); i++) {
    putmsg[i] = i;
  }

  ring.put(putmsg, sizeof(putmsg));
  size_t nput = gring.get(getmsg, sizeof(getmsg), sizeof(getmsg), 0); // should never block!
  EQ(sizeof(getmsg), nput);
  
  // Check that the pointers are correct:

  EQ(pPut->s_offset, pGet->s_offset);

  // Check the data:

  for (int i =0; i < sizeof(putmsg); i++) {
    EQ(putmsg[i], getmsg[i]);
  }
  munmap(pBuffer, pBuffer->s_header.s_topOffset+1);

}
// Simple wrapping get.  We do the wrapping put in wrapput and
// get the data from there as well.
// - Data should be good in our buffer.
// - Offsets whendone should be the same as the 
//   put offset.
//
void XferTests::wrapget()
{
  CRingBuffer ring(string(SHM_TESTFILE), CRingBuffer::producer);
  CRingBuffer gring(string(SHM_TESTFILE), CRingBuffer::consumer);

  pRingBuffer        pBuffer = reinterpret_cast<pRingBuffer>(mapRingBuffer(fileName(SHM_TESTFILE).c_str()));
  pClientInformation pPut    = &(pBuffer->s_producer);
  pRingHeader pHeader = &(pBuffer->s_header);

  // We should be the first consumer as there are no consumers and the
  // search strategy is linear:

  pClientInformation pGet = pBuffer->s_consumers;

  // Create the messages:

  char putmsg[100], getmsg[100];
  memset(getmsg, 0, sizeof(getmsg));
  for (int i=0; i < sizeof(putmsg); i++) {
    putmsg[i] = i;
  }

  // Set up and put:

  off_t startOffset = pHeader->s_topOffset - sizeof(putmsg)/2; 
  pPut->s_offset = startOffset;
  pGet->s_offset = startOffset;	// empty ring.

  ring.put(putmsg, sizeof(putmsg));

  // Get and check:

  size_t n = gring.get(getmsg, sizeof(putmsg), sizeof(putmsg), 0);
  EQ(sizeof(putmsg), n);

  munmap(pBuffer, pBuffer->s_header.s_topOffset+1);


}

//  Wrapping and and edge cases.


void XferTests::edgewrapget()
{
  CRingBuffer ring(string(SHM_TESTFILE), CRingBuffer::producer);
  CRingBuffer cring(string(SHM_TESTFILE), CRingBuffer::consumer);

  pRingBuffer        pBuffer = reinterpret_cast<pRingBuffer>(mapRingBuffer(fileName(SHM_TESTFILE).c_str()));
  pClientInformation pPut    = &(pBuffer->s_producer);
  pClientInformation pGet    =  pBuffer->s_consumers;
  pRingHeader pHeader = &(pBuffer->s_header);


  // Create the counting pattern message:

  char msg[100], rmsg[100];
  for (int i=0; i < sizeof(msg); i++) {
    msg[i] = i;
  }
  // Set up the put offset so that putting msg will exactly fill the
  // ring.

  off_t msgStart = pHeader->s_topOffset - sizeof(msg) +1 ;
  pPut->s_offset = msgStart;
  pGet->s_offset = msgStart;

  ring.put(msg, sizeof(msg));

  EQ(pHeader->s_dataOffset, pPut->s_offset);

  memset(rmsg, 0, sizeof(rmsg));
  size_t nread = cring.get(rmsg, sizeof(rmsg), sizeof(rmsg), 0);
  EQ(sizeof(rmsg), nread);
  EQ(pHeader->s_dataOffset, pGet->s_offset);

  for(int i =0; i < sizeof(rmsg); i++) {
    EQ(i, (int)rmsg[i]);
  }

  // Put one less and the put pointer should be at the max offset:

  msgStart = pHeader->s_topOffset - sizeof(msg);
  pPut->s_offset = msgStart;
  pGet->s_offset = msgStart;
  ring.put(msg, sizeof(msg));
  EQ(pHeader->s_topOffset, pPut->s_offset);
  
  memset(rmsg, 0, sizeof(rmsg));
  nread = cring.get(rmsg, sizeof(rmsg), sizeof(rmsg), 0);
  EQ(sizeof(rmsg), nread);
  EQ(pHeader->s_topOffset, pGet->s_offset);

  for (int i =0; i < sizeof(rmsg); i++) {
    EQ(i, (int)rmsg[i]);
  }


  munmap(pBuffer, pBuffer->s_header.s_topOffset+1);

}
// Ensure that if there are serveral puts, and then the same
// number of gets, all the data is gotten correctly.
//
void XferTests::multi()
{
  CRingBuffer xmit(string(SHM_TESTFILE), CRingBuffer::producer);
  CRingBuffer recv(string(SHM_TESTFILE), CRingBuffer::consumer);

  // 100 messages from 1 - 100 bytes of counting patterns.

  char sent[100];
  for (int i=0; i < sizeof(sent); i++) {
    sent[i] = i;
    xmit.put(sent, i+1);
  }
  char got[100];
  for  (int i = 1; i <= sizeof(got); i++) {
    memset(got, 0, sizeof(got));
    size_t nrecv = recv.get(got, i, i, 0);
    EQ(i, (int)nrecv);
    for (int  j = 0; j < i; j++) {
      EQ(j, (int)got[j]);
    }
  }
  
}

