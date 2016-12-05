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

#include <daqshm.h>

using namespace std;


class rmasterTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(rmasterTests);
  CPPUNIT_TEST(creation);
  CPPUNIT_TEST(registration_0);
  CPPUNIT_TEST(registration_1);
  CPPUNIT_TEST(registration_2);
  CPPUNIT_TEST(unregister_0);
  CPPUNIT_TEST(unregister_1);
  CPPUNIT_TEST(unregister_2);
  CPPUNIT_TEST(duplicatereg);
  CPPUNIT_TEST(getdata);
  
  // Tests for creation with the /dev/shm file already existing
  
  CPPUNIT_TEST(existsAndIsRing);
  CPPUNIT_TEST(existsAndIsNotRing);
  
  CPPUNIT_TEST_SUITE_END();


private:

public:
  void setUp() {
  }
  void tearDown() {
  }
protected:
  void creation();
  void registration_0();
  void registration_1();
  void registration_2();
  void unregister_0();
  void unregister_1();
  void unregister_2();
  void duplicatereg();
  void getdata();
  
  void existsAndIsRing();
  void existsAndIsNotRing();

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

void rmasterTests::registration_0() 
{
  // local host can register, but only if the shared memory exists. 
  // this will fail because no shared memory has been created 
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
      ASSERT(thrown);
    } else {
      localMaster.notifyDestroy(uniqueRing("dummyring"));
    }
  } catch (CException& exc) {
    std::cout << "registration exception CException " << exc.WasDoing() << std::endl;
    ASSERT(0);
  }
}
    

void rmasterTests::registration_1() { 
    // non local can't and throws an ENOTSUPP errno exception.

  try {
    bool thrown  = false;
    bool wrongException(false);

    char hostname[1000];
    gethostname(hostname, sizeof(hostname));
    string host(hostname);
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

void rmasterTests::registration_2() {
  // local ringmaster can register (i.e. create) if the shared memory already exists...
  //  we will use the CRingBuffer class to test this.
  CPPUNIT_ASSERT_NO_THROW_MESSAGE(
      "Registering ring buffer with existing shared mem", 
      CRingBuffer::create("_mytestring_") );
   
  CRingBuffer::remove("_mytestring_");
}
// Ensure we can remove rings.
// 1. Removing a ring that exists should work as localhost
// 2. Removing a ring that exists should fail as remote host.
// 3. Removing a ring that does not exist should fail as localhost.
//
void rmasterTests::unregister_0()
{
  try {
    char host[1000];
    gethostname(host, sizeof(host));
    CRingMaster localMaster;


    string hostString(host);
    CRingMaster remoteMaster(hostString);


    // Add a test ring...
    CRingBuffer::create(uniqueRing("testRing"));

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
  } catch (CException& exc) {
    std::cout << "unregister exception CException " << exc.WasDoing() << std::endl;
    ASSERT(0);
  }
}

void rmasterTests::unregister_1()
{

    // Should be able to remove as local:
  try {
    CRingBuffer::create(uniqueRing("testRing"));

    CRingMaster localMaster;
    bool thrown = false;
    try {
      // does a local remove and also cleans up the shared memory
      CRingBuffer::remove(uniqueRing("testRing"));
    }
    catch (...) {
      thrown = true;
    }
    ASSERT(!thrown);
  } catch (CException& exc) {
    std::cout << "registration exception CException " << exc.WasDoing() << std::endl;
    ASSERT(0);
  }
}

void rmasterTests::unregister_2()
{
    // Removing a non-existing ring should not fail
    CRingMaster localMaster;

    // does a local remove and also cleans up the shared memory
    CPPUNIT_ASSERT_NO_THROW_MESSAGE( 
        "unregistering nonexistent ring",
        localMaster.notifyDestroy( uniqueRing("testRing") ) );

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

  ASSERT(!thrown);
  
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
    
    CDAQShm::remove(shmName);
    
    // Create a non ring buffer shared memory
    
    long minsize = sysconf(_SC_PAGESIZE);
    ASSERT(!CDAQShm::create(
        shmName, minsize,
        CDAQShm::GroupRead | CDAQShm::GroupWrite |
        CDAQShm::OtherRead | CDAQShm::OtherWrite));
    
    // Creating a ring buffer with that name should fail with
    // CErrnoException.
    
    CPPUNIT_ASSERT_THROW(
        CRingBuffer::create(ringName),
        CErrnoException
    );
    
    // Cleanup by getting rid of that shared memory region.
    
    ASSERT(!CDAQShm::remove(shmName));
    
}
// If the shared memory exists and is a ring but not known to the
// ring master then we just make it known.

void rmasterTests::existsAndIsRing()
{
    std::string ringName("isaring");
    std::string shmName("/");
    shmName += ringName;
    
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
