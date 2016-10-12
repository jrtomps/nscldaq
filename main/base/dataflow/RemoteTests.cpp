// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include <CRingMaster.h>
#include <CRingBuffer.h>
#include <CRemoteAccess.h>
#include <string.h>
#include "testcommon.h"
#include <os.h>

using namespace std;

class RemoteTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(RemoteTests);
  CPPUNIT_TEST(sizeTest);
  CPPUNIT_TEST(consumerTest);
  CPPUNIT_TEST(mindataTest);
  CPPUNIT_TEST(timeoutTest);
  CPPUNIT_TEST(urllocal);
  CPPUNIT_TEST(urlremote);
  CPPUNIT_TEST(proxyName);
  CPPUNIT_TEST_SUITE_END();


private:

public:
  void setUp() {
    CRingBuffer::create(uniqueRing("urllocal"));
  }
  void tearDown() {
    CRingBuffer::remove(uniqueRing("urllocal"));  
  }
protected:
  void sizeTest();
  void consumerTest();
  void mindataTest();
  void timeoutTest();
  void urllocal();
  void urlremote();
  void proxyName();
};

CPPUNIT_TEST_SUITE_REGISTRATION(RemoteTests);

// Set/get proxy ring size:

void RemoteTests::sizeTest() 
{
  size_t value = CRingAccess::getProxyRingSize();
  size_t old   = CRingAccess::setProxyRingSize(value*2);
  EQ(old, value);
  EQ(value*2, CRingAccess::getProxyRingSize());

  // Make this all idempotent by restoring the value:

  CRingAccess::setProxyRingSize(value);
}

// Set get maxconsumers.

void RemoteTests::consumerTest()
{
  size_t value = CRingAccess::getProxyMaxConsumers();
  size_t old   = CRingAccess::setProxyMaxConsumers(value+10);
  EQ(value, old);
  EQ(value+10, CRingAccess::getProxyMaxConsumers());
  CRingAccess::setProxyMaxConsumers(value);
}
// set get mindata:

void RemoteTests::mindataTest()
{
  size_t value = CRingAccess::getMinData();
  size_t old   = CRingAccess::setMinData(value/2);
  EQ(value, old);
  EQ(value/2, CRingAccess::getMinData());
  
  CRingAccess::setMinData(value);
}
// Get/set timeout:

void
RemoteTests::timeoutTest()
{
  unsigned value = CRingAccess::getTimeout();
  unsigned old   = CRingAccess::setTimeout(value*2);
  EQ(value, old);
  EQ(value*2, CRingAccess::getTimeout());
  CRingAccess::setTimeout(value);
}

// Open a local ring using the remote access software.
// - Create a local ring.
// - Attach as producer.
// - Attach via URL.
// - Produce some data,
// - Consume some data
// - check data matches.
// - destroy the ring.

void
RemoteTests::urllocal()
{


  
  string hostName(Os::hostname());
	      


  // Encapsulate in a try block so we can show taht
  // exceptions were thrown, and also ensure we can
  // continue and delete the ring.
  //
  bool caught = false;
  CRingBuffer* pConsumer(0);
  try {
    std::string ring = uniqueRing("urllocal");
    std::string uri  = "tcp://localhost/";
    uri += ring;
    CRingBuffer  producer(ring, CRingBuffer::producer);
    
    pConsumer = CRingAccess::daqConsumeFrom(uri);
   
    ASSERT(pConsumer);

    char putBuffer[100];
    char getBuffer[100];
    memset(getBuffer, 0, sizeof(getBuffer));
    for (int i =0; i < sizeof(putBuffer); i++) {
      putBuffer[i]  = i;
    }
    
    producer.put(putBuffer, sizeof(putBuffer));
    size_t n = pConsumer->get(getBuffer, sizeof(getBuffer), 1);
    EQ(sizeof(getBuffer), n);
    
    for(int i =0; i < sizeof(getBuffer); i++) {
      EQ(putBuffer[i], getBuffer[i]);
    }
    
  }
  catch(...) {
    
    caught = true;
  }
  if(pConsumer) {
    delete pConsumer;
  }

  ASSERT(!caught);

}
// same as urllocal but use for the host name hostname()
// so that we force the ringmaster to think this is a
// remote connection

void
RemoteTests::urlremote()
{


  string hostName(Os::hostname());
	      


  // Encapsulate in a try block so we can show taht
  // exceptions were thrown, and also ensure we can
  // continue and delete the ring.
  //
  bool caught = false;
  CRingBuffer* pConsumer(0);
  try {
    std::string ring = uniqueRing("urllocal");
    
    CRingBuffer  producer(ring, CRingBuffer::producer);
    string url("tcp://");
    url += hostName;
    url += "/";
    url += ring;
    pConsumer = CRingAccess::daqConsumeFrom(url);
   
    sleep(1);			//  Let everything settle in and start.
    ASSERT(pConsumer);

    char putBuffer[100];
    char getBuffer[100];
    memset(getBuffer, 0, sizeof(getBuffer));
    for (int i =0; i < sizeof(putBuffer); i++) {
      putBuffer[i]  = i;
    }
    
    producer.put(putBuffer, sizeof(putBuffer));
    size_t n = pConsumer->get(getBuffer, sizeof(getBuffer), 1);
    EQ(sizeof(getBuffer), n);
    
    for(int i =0; i < sizeof(getBuffer); i++) {
      EQ(putBuffer[i], getBuffer[i]);
    }
    
  }
  catch(...) {
    
    caught = true;
    delete pConsumer;
  }

  ASSERT(!caught);
  delete pConsumer;		// Destroy the proxy ring.
}

// Test that the right proxy ring names get created.


void
RemoteTests::proxyName()
{
  
  // Since this could run outside the nscl
  // 1. hand in a fdqn
  // 2. hand  in an existing known publicly visible node.
  //
  std::string proxy = CRingAccess::computeLocalRingName("aring", "www.nscl.msu.edu");
  
  EQ(std::string("aring@www.nscl.msu.edu"), proxy);
}