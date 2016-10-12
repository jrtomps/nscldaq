// Template for a test suite.
#include <config.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"

#include <ErrnoException.h>
#include <errno.h>

#include <unistd.h>
#include <CRingMaster.h>

#include <CRingBuffer.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include "testcommon.h"
#include "CErrnoException.h"
#include <os.h>
#include <sys/types.h>
#include <unistd.h>


#include <daqshm.h>

using namespace std;

static std::string
fullName(std::string ring)
{
  ring += "_12";
  return ring;
}

class rmasterTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(rmasterTests);
  CPPUNIT_TEST(creation);
  CPPUNIT_TEST(registration);
  CPPUNIT_TEST(unregister);
  CPPUNIT_TEST(duplicatereg);
  CPPUNIT_TEST(getdata);
  
  // Tests for creation with the /dev/shm file already existing
  
  CPPUNIT_TEST(existsAndIsRing);
  CPPUNIT_TEST(existsAndIsNotRing);

  // Tests for client information.
  
  CPPUNIT_TEST(clientInfo);
  
  CPPUNIT_TEST_SUITE_END();


private:

public:
  void setUp() {
  }
  void tearDown() {
  }
protected:
  void creation();
  void registration();
  void unregister();
  void duplicatereg();
  void getdata();
  
  void existsAndIsRing();
  void existsAndIsNotRing();

  void clientInfo();
};

CPPUNIT_TEST_SUITE_REGISTRATION(rmasterTests);


// Test the ability to locate/connect to the ring master.

void rmasterTests::creation() {
  bool thrown(false);
  try {
    CRingMaster master;		// To local host.
  }
  catch (...) {
    thrown = true;
  }
  ASSERT(!thrown);
}
// Test registration interactions:
// registration indicates a new ring buffer has come into existence.

void rmasterTests::registration() 
{
  // local host can register:
  try {
    CRingMaster localMaster;

    bool thrown = false;
    try {
      localMaster.notifyCreate(uniqueRing("dummyring"));
    }
    catch(...) {
      thrown = true;
    }
    if (thrown) {
      ASSERT(!thrown);
    } else {
      localMaster.notifyDestroy(uniqueRing("dummyring"));
    }

    // non local can't and throws an ENOTSUPP errno exception.

    thrown  = false;
    bool wrongException(false);

    string host = Os::hostname();
    CRingMaster remoteMaster(host); // Anything but localhost is remote.

    int errcode;
    try {
      remoteMaster.notifyCreate(uniqueRing("anewring"));
    }
    catch (CErrnoException &error) {
      errcode = error.ReasonCode();
      thrown = true;
    }
    catch (...) {
      thrown         = true;
      wrongException = true;
    }
    ASSERT(thrown);
    ASSERT(!wrongException);
    EQ(ENOTSUP, errcode);
  } catch (CException& exc) {
    std::cout << "registration exception CException " << exc.WasDoing() << std::endl;
    ASSERT(0);
  }
}
// Ensure we can remove rings.
// 1. Removing a ring that exists should work as localhost
// 2. Removing a ring that exists should fail as remote host.
// 3. Removing a ring that does not exist should fail as localhost.
//
void rmasterTests::unregister()
{
  try {

    string hostString = Os::hostname();
    CRingMaster localMaster;
    CRingMaster remoteMaster(hostString);


    // Add a test ring...

    localMaster.notifyCreate(uniqueRing("testRing"));

    // Should not be able to remove from the remote:

    bool thrown(false);
    bool wrongException(false);
    int  code;
    try {
      remoteMaster.notifyDestroy(uniqueRing("testRing"));
    }
    catch (CErrnoException& error) {
      thrown = true;
      code   = error.ReasonCode();
    }
    catch (...) {
      thrown = true;
      wrongException = false;
    }
    ASSERT(thrown);
    ASSERT(!wrongException);
    EQ(ENOTSUP, code);


    // Should be able to remove as local:

    thrown = false;
    try {
      localMaster.notifyDestroy(uniqueRing("testRing"));
    }
    catch (...) {
      thrown = true;
    }
    ASSERT(!thrown);

    // Removing testRing again should fail, string exception.

    thrown         = false;
    wrongException = false;

    try {
      localMaster.notifyDestroy(uniqueRing("testRing"));
    }
    catch(string msg) {
      thrown = true;
    }
    catch (...) {
      thrown = true;
      wrongException = true;
    }
    ASSERT(thrown);
    ASSERT(!wrongException);

  } catch (CException& exc) {
    std::cout << "registration exception CException " << exc.WasDoing() << std::endl;
    ASSERT(0);
  }
}
// It is an error to register a duplicate ring.

void rmasterTests::duplicatereg()
{
  CRingMaster master;

  master.notifyCreate(uniqueRing("duplicate"));

  bool thrown(false);
  bool wrongException(false);

  try {
    master.notifyCreate(uniqueRing("duplicate"));
  } 
  catch (string msg) {
    thrown = true;
  }
  catch (...) {
    thrown = true;
    wrongException = false;
  }

  ASSERT(thrown);
  ASSERT(!wrongException);
  
  master.notifyDestroy(uniqueRing("duplicate"));
}

// Get data from a remote ring.
// We're going to be give a socket 
// on which in theory we can read data.
//
void
rmasterTests::getdata()
{

  try { CRingBuffer::remove(uniqueRing("remote")); } catch (...) {}
  CRingBuffer::create(uniqueRing("remote"));
  CRingMaster master;
  
  bool thrown(false);
  int socket;
  try {
    socket = master.requestData(uniqueRing("remote"));
  }
  catch (...) {
    thrown = true;
  }
  ASSERT(!thrown);
  ASSERT(socket);


  // Since this is a socket, I should be able to get the peer:

  sleep(2);
  sockaddr_in peer;
  socklen_t         size=sizeof(peer);
  int status = getpeername(socket, (sockaddr*)&peer,  &size);
  EQ(0, status);
  EQ(AF_INET, (int)peer.sin_family);
  string textaddr(inet_ntoa(peer.sin_addr));
  EQ(string("127.0.0.1"), textaddr);

  shutdown(socket, SHUT_RDWR);
  CRingBuffer::remove(uniqueRing("remote"));
}

// It's not legal to make a ring buffer that collides with an existing,
// non ring buffer shared memory region.

void rmasterTests::existsAndIsNotRing()
{
    std::string ringName("notaring");
    std::string shmName("/");
    shmName += ringName;
    
    // Delete without checking status in case it already exists:
    
    CDAQShm::remove(fullName(shmName));
    
    // Create a non ring buffer shared memory
    
    long minsize = sysconf(_SC_PAGESIZE);
    ASSERT(!CDAQShm::create(
	fullName(shmName), minsize,
        CDAQShm::GroupRead | CDAQShm::GroupWrite |
        CDAQShm::OtherRead | CDAQShm::OtherWrite));
    
    // Creating a ring buffer with that name should fail with
    // CErrnoException.
    
    CPPUNIT_ASSERT_THROW(
        CRingBuffer::create(ringName),
        CErrnoException
    );
    
    // Cleanup by getting rid of that shared memory region.
    
    ASSERT(!CDAQShm::remove(fullName(shmName)));
    
}
// If the shared memory exists and is a ring but not known to the
// ring master then we just make it known.

void rmasterTests::existsAndIsRing()
{
    std::string ringName("isaring");
    std::string shmName("/");
    shmName += ringName;
    shmName += "_12";
    
    CDAQShm::remove(shmName);                 // Just in case
    
    // Make a new file and format it as a ring:
    
    long page   = sysconf(_SC_PAGESIZE);
    size_t size = (1024*1024*2 /page) * page; // size is just a multiple of pagesize:

    ASSERT(!CDAQShm::create(
        shmName, size,
        CDAQShm::GroupRead | CDAQShm::GroupWrite |
        CDAQShm::OtherRead | CDAQShm::OtherWrite));
    CRingBuffer::format(ringName, 10);
    
    CPPUNIT_ASSERT_NO_THROW(CRingBuffer::create(ringName));
    CPPUNIT_ASSERT_NO_THROW(CRingBuffer::remove(ringName));
    
    CDAQShm::remove(shmName);             // In case test failed.
    
}

/**
 *  Check that we can get the client info from the ring master.
 *  -  Get our process information (via the OS package).
 *  -  Create a ring.
 *  -  Attach as consumer.
 *  -  Attach as producer.
 *  -  Ask for client information and make sure it's correct.
 * @note - in order to be sure the ring disappears, we do most of the work inside
 *         a try/catch block.
 */
void rmasterTests::clientInfo()
{
  std::string ringname = "clientInfoRing";
  // in case this ring is hanging around:
  
  try { CRingBuffer::remove(ringname); } catch (...) {}
  
  CRingBuffer::create(ringname);
  try {
    pid_t mypid = getpid();
    std::vector<std::string> me = Os::getProcessCommand(mypid);
    
    CRingBuffer producer(ringname, CRingBuffer::producer);
    CRingBuffer consumer(ringname, CRingBuffer::consumer);
    
    CRingMaster master("localhost");
    CRingMaster::ClientCommands clients = master.listClients(ringname);

    EQ(me.size(), clients.s_producer.size());
    for (int i = 0; i < me.size(); i++) {
      EQ(me[i], clients.s_producer[i]);
    }
    
    EQ(size_t(1), clients.s_consumers.size());
    EQ(me.size(), clients.s_consumers[0].size());
    for (int i =0; i < me.size(); i++) {
      EQ(me[i], clients.s_consumers[0][i]);
    }
    
  }
  catch (...) {
    // this also makes use of the fact that test fails are exceptions:
    
    CRingBuffer::remove(ringname);
    throw;
  }
  CRingBuffer::remove(ringname);
}
