// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include <stdio.h> 		// popen is so convenient...
#include <unistd.h>
#include <stdlib.h>
#include "Asserts.h"
#include "CRingBuffer.h"
#include "CConnectivity.h"
#include "os.h"

class ConnectivityTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(ConnectivityTest);

  // Tests for getProducers.

  CPPUNIT_TEST(noProducer1);
  CPPUNIT_TEST(noProducer2);
  CPPUNIT_TEST(noProducer3);
  CPPUNIT_TEST(fakeProducer);

  // Tests for getConsumers:

  CPPUNIT_TEST(noConsumer1);
  CPPUNIT_TEST(noConsumer2);
  CPPUNIT_TEST(noConsumer3);
  CPPUNIT_TEST(consumer);

  // Tests for connectivity walker
  // Note we can really only test
  // that localhost becomes what we want.
  
  CPPUNIT_TEST(walker);
  CPPUNIT_TEST_SUITE_END();

private:
  CConnectivity* m_pConnectivity;
public:
  void setUp() {
    m_pConnectivity = new CConnectivity("localhost");
  }
  void tearDown() {
    delete m_pConnectivity;
    m_pConnectivity = 0;
  }
protected:
  void noProducer1();
  void noProducer2();
  void noProducer3();
  void fakeProducer();

  void noConsumer1();
  void noConsumer2();
  void noConsumer3();
  void consumer();

  void walker();
private:
  std::string makeFakeProxy(const char* ring, const char* host);
  std::string ringToHost(std::string ring);
  std::string makeStdinCommand(std::string ring);
  FILE*       startFakeProducer(std::string ring);
  void        createConsumer(std::string cmdline);
};

CPPUNIT_TEST_SUITE_REGISTRATION(ConnectivityTest);
/*----------------------------------------------------------
 *  Utility support functions.
 */

/**
 * makeFakeProxy
 *   Given a ring and host, creates a fake proxy ring.
 *   Note that no consumer is created.
 * @param ring - 'remote' ring name.
 * @param host - 'remote; host.
 * @return std::string - full name of the ring created.
 */
std::string
ConnectivityTest::makeFakeProxy(const char* ring, const char* host)
{
  std::string ringName = ring;
  ringName            += '@';
  ringName            += host;
  try {
    CRingBuffer::remove(ringName);      // in case it already exists
  } catch(...) {}
  CRingBuffer::create(ringName);
  return ringName;
}
/**
 * ringToHost 
 *   Extract the hostname from a poxy ring name.
 *
 * @param ring - name of the ring (ring@hostname).
 * @return std::string
 */
std::string
ConnectivityTest::ringToHost(std::string ring)
{
  size_t atIsAt = ring.find("@");
  size_t hostIsAt = atIsAt + 1;
  return ring.substr(hostIsAt);
}
/**
 * makeStdinCommand
 *   Make the command string for a proxy ring producer.
 *
 * @param ring - name of the proxy ring (ring@host).
 * @return std::string - the full command string.
 * @note   The compilation defined BINDIR to be the installation dir
 *         of stdintoring.
 */
std::string
ConnectivityTest::makeStdinCommand(std::string ring)
{
  std::string command = BINDIR;
  command            += "/stdintoring ";
  command            += ring;
  command            += " ";
  command            += ringToHost(ring);
  return command;
}
/**
 * startFakeProducer
 *   Starts stdinToRing on our ring in a way that make is look
 *   like it's a proxy producer.  In fact the stdin of stdinToRing
 *   will be a pipe, not the network.
 *
 *  @param ring - name of the ring stdinToRing will produce for.
 *  @return FILE* - file object open on the stdin of stdinToRing.
 */
FILE*
ConnectivityTest::startFakeProducer(std::string ring)
{
  std::string command = makeStdinCommand(ring);
return popen(command.c_str(), "w");
}
/**
 * createConsumer
 *   Really just fork/exec's a consumer program.
 *   the BINDIR symbol is assumed to be where the consumer lives:
 *
 * @param commands - command line string.
 */
void
ConnectivityTest::createConsumer(std::string consumer)
{
  std::string program = BINDIR;
  program            += "/";
  program            += consumer;
  popen(program.c_str(), "r");
  sleep(1);			// Give the consumer start time.
  
}
/*----------------------------------------------------------
 * Tests:
 */
// If there are no rings (assuming this is a pristine system)
// there are no remote producers.
void ConnectivityTest::noProducer1() {
  std::vector<std::string> p = m_pConnectivity->getProducers();
  EQ(size_t(0), p.size());
}
// If there's a ring but it does not match the name profile of
// name@host there's no producer

void ConnectivityTest::noProducer2()
{
  try { CRingBuffer::remove("ron"); } catch(...) {} // /in cas it exists.
  CRingBuffer::create("ron");

  // since cppunit throws errors for assertion failures,
  // the try/catch below ensures teardown of the ring.

  try {
    std::vector<std::string> p = m_pConnectivity->getProducers();
    EQ(size_t(0), p.size());
  }
  catch (...) {
    CRingBuffer::remove("ron");
    throw;                         // Rethrow the exceptiob,.
  }
  // Be nice to have a finally spec.

  CRingBuffer::remove("ron");
}
//   If there's a ring of the right name but no producer attached,
//   still no joy:

void ConnectivityTest::noProducer3()
{
  std::string ringName = makeFakeProxy("ron", "spdaq20.nscl.msu.edu");
  try {
    std::vector<std::string> p = m_pConnectivity->getProducers();
    EQ(size_t(0), p.size());
  }
  catch (...) {
    CRingBuffer::remove(ringName);
    throw;
  }
  CRingBuffer::remove(ringName);
}
//
//  Creating a ring with the correctly named fake producer
//  should give a hit:
//
void
ConnectivityTest::fakeProducer()
{
  std::string ring = makeFakeProxy("ron", "spdaq20.nscl.msu.edu");
  FILE* inPipe = nullptr;
  try {
    inPipe = startFakeProducer(ring);
    sleep(1);			// Wait for it to establish.
    std::vector<std::string> p = m_pConnectivity->getProducers();
    EQ(size_t(1), p.size());
    EQ(std::string("spdaq20.nscl.msu.edu"), p[0]);
    
  }
  catch (...) {
    CRingBuffer::remove(ring);
    throw;
  }
  CRingBuffer::remove(ring);	// Should get stdintoring to exit via kill.
  if (inPipe) pclose(inPipe);
}
//  No rings means no consumers hoisting data out of the host:

void
ConnectivityTest::noConsumer1()
{
  std::vector<std::string> result = m_pConnectivity->getConsumers();

  EQ(size_t(0), result.size());
}

//  Rings with no consumers also don't return any:

void
ConnectivityTest::noConsumer2()
{
  CRingBuffer::create("ron");
  try {
    std::vector<std::string> result = m_pConnectivity->getConsumers();
    EQ(size_t(0), result.size());
  }
  catch (...) {
    CRingBuffer::remove("ron");
    throw;
  }
  CRingBuffer::remove("ron");
}
//  Consumer's exist but they are not the right type:

void
ConnectivityTest::noConsumer3()
{
  CRingBuffer::create("ron");
  try {
    createConsumer("ringselector --formatted ron"); // Has enough params.
    std::vector<std::string> c = m_pConnectivity->getConsumers();
    EQ(size_t(0), c.size());
  }
  catch (...) {
    CRingBuffer::remove("ron");
    throw;
  } 
  CRingBuffer::remove("ron");
}
//  try with anactual consumer:

void 
ConnectivityTest::consumer()
{
  CRingBuffer::create("ron");
  try {
    createConsumer("ringtostdout  ron spdaq19.nscl.msu.edu"); // Has enough params.
    std::vector<std::string> c = m_pConnectivity->getConsumers();
    EQ(size_t(1), c.size());
    EQ(std::string("spdaq19.nscl.msu.edu"), c[0]);
  }
  catch (...) {
    CRingBuffer::remove("ron");
    throw;
  } 
  CRingBuffer::remove("ron");
}
// We should find our fqdn in the walk:

void
ConnectivityTest::walker()
{
  char host[1000];
  gethostname(host, sizeof(host));
  std::string fq = Os::getfqdn(host);
  std::vector<std::string> clients = m_pConnectivity->getAllParticipants();
  EQ(size_t(1), clients.size());
  EQ(Os::getfqdn(host), fq);
}
