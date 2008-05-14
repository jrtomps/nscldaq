// Test the static member functions of CRingBuffer.cpp

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include <CRingBuffer.h>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>

#include "ringbufint.h"

using namespace std;

//  Default the directory that has shared memory special files
//  to the one used by linux.
//  This can be overridden for other systems and places.
//

#ifndef SHM_DIRECTORY
#define SHM_DIRECTORY "/dev/shm"
#endif

// Default name of shared memory special file:

#ifndef SHM_TESTFILE
#define SHM_TESTFILE "statictest"
#endif


static string getFullName()
{
  string fullname(SHM_DIRECTORY);
  fullname += "/";
  fullname += SHM_TESTFILE;
  return fullname;
}

class StaticRingTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(StaticRingTest);
  CPPUNIT_TEST(defaults);
  CPPUNIT_TEST(create);
  CPPUNIT_TEST(format);
  CPPUNIT_TEST_SUITE_END();




private:

public:
  void setUp() {
  }
  void tearDown() {
    // hopefully this gets called on failure too:
    // get rid of any shared memory file that might be lying around.
    
    string fullname  = getFullName();
    unlink(fullname.c_str());
  }
protected:
  void defaults();
  void create();
  void format();
};

CPPUNIT_TEST_SUITE_REGISTRATION(StaticRingTest);




// Test ability to manipulate the defaults.

void StaticRingTest::defaults()
{
  size_t originalSize =  CRingBuffer::getDefaultRingSize();
  size_t originalNCons = CRingBuffer::getDefaultMaxConsumers();

  CRingBuffer::setDefaultRingSize(200*1024*1024); // 200Mbytes
  EQMSG("Check size change", (size_t)(200*1024*1024), CRingBuffer::getDefaultRingSize());
  
  CRingBuffer::setDefaultRingSize(originalSize); // do no harm in the test.

  CRingBuffer::setDefaultMaxConsumers(250);
  EQMSG("Check max consumer change", (size_t)(250), CRingBuffer::getDefaultMaxConsumers());

  CRingBuffer::setDefaultMaxConsumers(originalNCons);
}

// test creation of a ring buffer.

void StaticRingTest::create() {
  CRingBuffer::create(string(SHM_TESTFILE));
  
  string fullname = getFullName();


  struct stat buf;
  int status = stat(fullname.c_str(), &buf);
  if(status == -1) {
    FAIL("Unable to stat shm special file");
  }

  // figure out the size... it should match what we  got from the stat.


  size_t data = CRingBuffer::getDefaultRingSize();
  size_t ncons= CRingBuffer::getDefaultMaxConsumers() + 1; // (+1 for the producer).
  off_t  total= data + ncons*sizeof(ClientInformation) + sizeof(RingHeader);

  // Align to pagesize:

  long pagesize = sysconf(_SC_PAGESIZE);

  size_t pages = (total + pagesize -1)/pagesize;
  total        = pages*pagesize;

  EQMSG("Size check on special file", total, buf.st_size);
  

  // mode should be 0666 (modulo special file bits).

  EQMSG("mode check", (mode_t)0666, buf.st_mode & 0777);
  
  // Get rid of the special file.

  unlink(fullname.c_str());
  
}

// Check that a ring is well formatted:

void StaticRingTest::format()
{
  CRingBuffer::create(string(SHM_TESTFILE)); // also formats.

  // independently map and check the header...

  int fd = shm_open(SHM_TESTFILE, O_RDONLY, 0666);
  ASSERT(fd >= 0);
  struct stat buf;

  fstat(fd, &buf);

  void* map = mmap(0, buf.st_size, PROT_READ, MAP_SHARED,
		   fd, 0);
  close(fd);

  ASSERT(map != MAP_FAILED);
  
  
  pRingBuffer        pRing         = reinterpret_cast<pRingBuffer>(map);
  pRingHeader        pHeader       = &(pRing->s_header);
  pClientInformation pProducer     = &(pRing->s_producer);
  pClientInformation pConsumers    = (pRing->s_consumers);

  // Verify the header fields :

  EQ(CRingBuffer::getDefaultMaxConsumers(), pHeader->s_maxConsumer);
  EQ(sizeof(RingHeader), (unsigned int)pHeader->s_producerInfo);
  EQ(sizeof(RingHeader)+sizeof(ClientInformation), (unsigned int)pHeader->s_firstConsumer);
  EQ(sizeof(RingHeader) + (pHeader->s_maxConsumer+1)*sizeof(ClientInformation),
     (unsigned int)pHeader->s_dataOffset);
  EQ(buf.st_size - pHeader->s_dataOffset, (long int)pHeader->s_dataBytes);
  EQ(buf.st_size -1, pHeader->s_topOffset);


  munmap(map, buf.st_size);
  
  
}
