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

using namespace std;


class rmasterTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(rmasterTests);
  CPPUNIT_TEST(creation);
  CPPUNIT_TEST(registration);
  CPPUNIT_TEST(unregister);
  CPPUNIT_TEST(duplicatereg);
  CPPUNIT_TEST(getdata);
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

  CRingMaster localMaster;
  
  bool thrown = false;
  try {
    localMaster.notifyCreate("dummyring");
  }
  catch(...) {
    thrown = true;
  }
  if (thrown) {
  ASSERT(!thrown);
  } else {
    localMaster.notifyDestroy("dummyring");
  }

  // non local can't and throws an ENOTSUPP errno exception.

  thrown  = false;
  bool wrongException(false);

  char hostname[1000];
  gethostname(hostname, sizeof(hostname));
  string host(hostname);
  CRingMaster remoteMaster(host); // Anything but localhost is remote.

  int errcode;
  try {
    remoteMaster.notifyCreate("anewring");
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
}
// Ensure we can remove rings.
// 1. Removing a ring that exists should work as localhost
// 2. Removing a ring that exists should fail as remote host.
// 3. Removing a ring that does not exist should fail as localhost.
//
void
rmasterTests::unregister()
{
  char host[1000];
  gethostname(host, sizeof(host));
  CRingMaster localMaster;


  string hostString(host);
  CRingMaster remoteMaster(hostString);


  // Add a test ring...

  localMaster.notifyCreate("testRing");
  
  // Should not be able to remove from the remote:

  bool thrown(false);
  bool wrongException(false);
  int  code;
  try {
    remoteMaster.notifyDestroy(string("testRing"));
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
    localMaster.notifyDestroy(string("testRing"));
  }
  catch (...) {
    thrown = true;
  }
  ASSERT(!thrown);

  // Removing testRing again should fail, string exception.

  thrown         = false;
  wrongException = false;

  try {
    localMaster.notifyDestroy(string("testRing"));
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

}
// It is an error to register a duplicate ring.

void rmasterTests::duplicatereg()
{
  CRingMaster master;

  master.notifyCreate("duplicate");

  bool thrown(false);
  bool wrongException(false);

  try {
    master.notifyCreate("duplicate");
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
  
  master.notifyDestroy("duplicate");
}

// Get data from a remote ring.
// We're going to be give a socket 
// on which in theory we can read data.
//
void
rmasterTests::getdata()
{

  try { CRingBuffer::remove("remote"); } catch (...) {}
  CRingBuffer::create("remote");
  CRingMaster master;
  
  bool thrown(false);
  int socket;
  try {
    socket = master.requestData("remote");
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
  CRingBuffer::remove("remote");
}
