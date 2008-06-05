// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include <ErrnoException.h>

#include <CRingBuffer.h>
#include <ringbufint.h>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/wait.h>

#include "testcommon.h"

using namespace std;




// Default name of shared memory special file:

#ifndef SHM_TESTFILE
#define SHM_TESTFILE "blocktest"
#endif



class BlockTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(BlockTest);
  CPPUNIT_TEST(reader);
  CPPUNIT_TEST(blocktilwrite);
  CPPUNIT_TEST(blocktilread);
  CPPUNIT_TEST_SUITE_END();


private:

public:
  void setUp() {
    CRingBuffer::create(string(SHM_TESTFILE));
  }
  void tearDown() {
    try {
      CRingBuffer::remove(SHM_TESTFILE);
    }
    catch(...) {
    }

  }
protected:
  void reader();
  void blocktilwrite();
  void blocktilread();
};

CPPUNIT_TEST_SUITE_REGISTRATION(BlockTest);

//
// If there's no data, the reader should bock.

void BlockTest::reader() {
  CRingBuffer ring(string(SHM_TESTFILE));
  char buffer[100];

  // Non blocking...but no data..

  size_t nbytes = ring.get(buffer, sizeof(buffer), 1, 0);
  EQ((size_t)0, nbytes);

  // This should block for a couple of seconds and then give no data:

  nbytes = ring.get(buffer, sizeof(buffer), 1, 2);
  EQ((size_t)0, nbytes);


}
// 
// The reader should be able to block until  a write operation.
//

void BlockTest::blocktilwrite()
{

  pid_t pid = fork();

  if (pid) {			// parent
    char buffer[100];
    sleep(1);
    CRingBuffer ring(string(SHM_TESTFILE));
    
    memset(buffer, 0, sizeof(buffer));
    size_t  nread = ring.get(buffer, sizeof(buffer), sizeof(buffer));	// Wait as long as needed .. up to 136 years.
    EQ(sizeof(buffer), nread);
    for (int i =0; i< sizeof(buffer); i++) {
      EQ(i, (int)buffer[i]);
    }
  }
  else {			// child.
    {
      CRingBuffer ring(string(SHM_TESTFILE), CRingBuffer::producer);
      CRingBuffer::Usage used = ring.getUsage();
      while (used.s_consumers.size() == 0) {
	sleep(1);
	used = ring.getUsage();
      }
      
      char buffer[100];
      for (int i =0; i < 100; i ++) {
	buffer[i] = i;
      }
      ring.put(buffer, sizeof(buffer));
      
    }
    
    exit(0);			// othewise we'll double report the test results to date.
  }
  int status;
  waitpid(pid, &status, 0);
  ASSERT(WIFEXITED(status));
  EQ(0, WEXITSTATUS(status));

}
// Artificially set things up so that the writer should block.
// attempt a write and ensure that it blocks until a read happens.
//

void BlockTest::blocktilread()
{
  CRingBuffer prod(string(SHM_TESTFILE), CRingBuffer::producer);
  CRingBuffer g(string(SHM_TESTFILE));

  char buffer[1024];
  for (int i =0; i < 1024; i++) {
    buffer[i] = i;
  }

  // Write until full..

  size_t nwritten;
  while ((nwritten = prod.put(buffer, sizeof(buffer), 0)) == sizeof(buffer)) {
  }
  

  pid_t pid = fork();

  // This is a bit of a cheat... consumer handles can't really be inheritied correctly
  // as they will have the wrong pid.. but the ring buffer is too stupid to know that.
  // I need to do this, however else the producer will never block because there's no
  // get pointers to run up against.
  //

  if (pid) {			// Parent...
    nwritten = prod.put(buffer, sizeof(buffer), 0);
    EQ((size_t)0, nwritten);	// should not be able to...
    nwritten = prod.put(buffer,sizeof(buffer));	// allow to block...
    EQ(sizeof(buffer), nwritten);
  }
  else {			// Child
    sleep(4);			// Wait a bit to force a block.
    char buffer[1024];
    g.get(buffer, sizeof(buffer));
    
    exit(0);
  }

  int status;
  waitpid(pid, &status, 0);
  ASSERT(WIFEXITED(status));
  EQ(0, WEXITSTATUS(status));  

}
