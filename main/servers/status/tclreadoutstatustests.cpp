// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include <tcl.h>
#include <TCLInterpreter.h>
#include <CTCLReadoutStatistics.h>
#include <CTCLRingStatistics.h>
#include <TCLException.h>
#include <zmq.hpp>
#include <Asserts.h>
#include <sstream>
#include <os.h>
#include <testutils.h>

static std::string uri = "inproc://test";    // Use intra process sockets.

class TclReadoutStatsTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(TclReadoutStatsTests);
  
  // Tests for wrapped object construction:
  
  CPPUNIT_TEST(construct);
  CPPUNIT_TEST(constructWithApp);
  CPPUNIT_TEST(constructNeedUri);
  CPPUNIT_TEST(constructTooManyWords);
  
  // tests for wrapped object destruction:
  
  CPPUNIT_TEST(destroy);
  CPPUNIT_TEST(destroyNoSuch);

  // Tests that check message send/formatting:
  
  CPPUNIT_TEST(beginRun);
  CPPUNIT_TEST(statistics);
  
  CPPUNIT_TEST_SUITE_END();


private:
  CTCLInterpreter* m_pInterpObj;
  CTCLReadoutStatistics* m_pStatsCmd;
  Tcl_Interp*      m_pInterpRaw;
  
  zmq::context_t*    m_pZmqContext;
  zmq::socket_t*   m_pReceiver;
public:
  void setUp() {
   // Create the interpreter, fish out the raw one for convenience and load the pkg.
    
    m_pInterpObj = new CTCLInterpreter();
    m_pInterpRaw = m_pInterpObj->getInterpreter();
    Tcl_Init(m_pInterpRaw);
    m_pStatsCmd = new CTCLReadoutStatistics(*m_pInterpObj);
    m_pStatsCmd->enableTesting();
    
    //  Setup zmq - our receiver socket will be bound to the uri as a SUB socket.
    
    m_pZmqContext = &CTCLRingStatistics::m_zmqContext;
    m_pReceiver   = new zmq::socket_t(*m_pZmqContext, ZMQ_PULL);
    m_pReceiver->bind(uri.c_str());
  }
  void tearDown() {
    // Get rid of all this stuff - interpreter first so that the socket is
    // destroyed first:
    
    delete m_pStatsCmd;
    delete m_pInterpObj;
    delete m_pReceiver;
  }
protected:
  void construct();
  void constructWithApp();
  void constructNeedUri();
  void constructTooManyWords();
  
  void destroy();
  void destroyNoSuch();
  
  void beginRun();
  void statistics();
};

CPPUNIT_TEST_SUITE_REGISTRATION(TclReadoutStatsTests);

// Constructing a new readout status object creates a new command.

void TclReadoutStatsTests::construct() {
  std::stringstream construct;
  construct << "ReadoutStatistics create " << uri;
  std::string newCmd;
  CPPUNIT_ASSERT_NO_THROW(
    newCmd = m_pInterpObj->Eval(construct.str())
  );
  
  // There should be a new command:
  
  std::stringstream info;
  info << "info commands " << newCmd;
  
  EQ(newCmd, m_pInterpObj->Eval(info.str()));
}

// It's also possible to construct an object with an application name:

void TclReadoutStatsTests::constructWithApp()
{
  std::stringstream construct;
  construct << "ReadoutStatistics create " << uri;
  construct << " MyAppName";
  std::string newCmd;
  
  CPPUNIT_ASSERT_NO_THROW(
    newCmd = m_pInterpObj->Eval(construct.str())
  );
  
  // There should be a new command:
  
  std::stringstream info;
  info << "info commands " << newCmd;
  
  EQ(newCmd, m_pInterpObj->Eval(info.str()));
}

// Construction does require a URI else the command fails which, in tcl++
// results in a CTCLException being thrown:

void TclReadoutStatsTests::constructNeedUri()
{
    CPPUNIT_ASSERT_THROW(
      m_pInterpObj->Eval("ReadoutStatistics create"),
      CTCLException
    );
}
// Construction can only take at most two parameters after the create
// subcommand:

void TclReadoutStatsTests::constructTooManyWords()
{
  CPPUNIT_ASSERT_THROW(
    m_pInterpObj->Eval("ReadoutStatistics create tcp://localhost:12345 Mytest extra"),
    CTCLException
  );
}

// After we destroy an object, its command no longer exists:

void TclReadoutStatsTests::destroy()
{
  std::stringstream construct;
  construct << "ReadoutStatistics create " << uri;
  std::string newCmd;
  CPPUNIT_ASSERT_NO_THROW(
    newCmd = m_pInterpObj->Eval(construct.str())
  );
  
  // There should be a new command:
  
  std::stringstream info;
  info << "info commands " << newCmd;
  
  EQ(newCmd, m_pInterpObj->Eval(info.str()));
  
  // Destroy the command:
  
  std::stringstream destroyCommand;
  destroyCommand << "ReadoutStatistics destroy " << newCmd;
  CPPUNIT_ASSERT_NO_THROW(
    m_pInterpObj->Eval(destroyCommand.str())
  );
  
  EQ(std::string(""), m_pInterpObj->Eval(info.str()));
}

// Attempting to destroy a nonexistent wrapped object is an error:

void TclReadoutStatsTests::destroyNoSuch()
{
  CPPUNIT_ASSERT_THROW(
    m_pInterpObj->Eval("RingStatistics destroy junk"),
    CTCLException
  );
}

// The beginRun subcommand will send a two part message which contains a header
// and a run ident part.

void TclReadoutStatsTests::beginRun()
{
  
  std::string app("MyAppName");
  std::stringstream construct;
  construct << "ReadoutStatistics create " << uri;
  construct << " " << app;
  std::string newCmd;
  
  CPPUNIT_ASSERT_NO_THROW(
    newCmd = m_pInterpObj->Eval(construct.str())
  );
  
  // Emit a begin run message:
  
  std::stringstream beginCommand;
  beginCommand << newCmd << " beginRun 1234 {This is a new title}";
  CPPUNIT_ASSERT_NO_THROW(
    m_pInterpObj->Eval(beginCommand.str())
  );
  
  // Receive the message on m_pReceiver:
  
  zmq::message_t hdr;
  zmq::message_t runId;
  
  uint64_t haveMore(0);
  size_t   s(sizeof(uint64_t));
  
  m_pReceiver->recv(&hdr);
  m_pReceiver->getsockopt(ZMQ_RCVMORE, &haveMore, &s);
  ASSERT(haveMore);
  
  m_pReceiver->recv(&runId);
  m_pReceiver->getsockopt(ZMQ_RCVMORE, &haveMore, &s);
  ASSERT(!haveMore);
  
  //  Now look at the message contents:
       // Header:
       
  CStatusDefinitions::Header* pHeader =
    reinterpret_cast<CStatusDefinitions::Header*>(hdr.data());
  
  EQ(CStatusDefinitions::MessageTypes::READOUT_STATISTICS, pHeader->s_type);
  EQ(CStatusDefinitions::SeverityLevels::INFO,             pHeader->s_severity);
  EQ(app, std::string(pHeader->s_application));
  EQ(Os::hostname(), std::string(pHeader->s_source));
  
      // Run Id:
      
  CStatusDefinitions::ReadoutStatRunInfo* pInfo =
    reinterpret_cast<CStatusDefinitions::ReadoutStatRunInfo*>(runId.data());
  
  EQ(uint32_t(1234), pInfo->s_runNumber);
  EQ(std::string("This is a new title"), std::string(pInfo->s_title));
}

// Send a statistics record note that we first have to send a run id record.
// that will be sent and absorbed as the contents of that have already been
// tested.

void TclReadoutStatsTests::statistics()
{
  std::string app("MyAppName");
  std::stringstream construct;
  construct << "ReadoutStatistics create " << uri;
  construct << " " << app;
  std::string newCmd;
  
  CPPUNIT_ASSERT_NO_THROW(
    newCmd = m_pInterpObj->Eval(construct.str())
  );
  
  // Emit a begin run message:
  
  std::stringstream beginCommand;
  beginCommand << newCmd << " beginRun 1234 {This is a new title}";
  CPPUNIT_ASSERT_NO_THROW(
    m_pInterpObj->Eval(beginCommand.str())
  );
  
  // Receive the message on m_pReceiver:
  
  zmq::message_t hdr;
  zmq::message_t runId;
  
  uint64_t haveMore(0);
  size_t   s(sizeof(uint64_t));
  
  m_pReceiver->recv(&hdr);
  m_pReceiver->getsockopt(ZMQ_RCVMORE, &haveMore, &s);
  ASSERT(haveMore);
  
  m_pReceiver->recv(&runId);
  m_pReceiver->getsockopt(ZMQ_RCVMORE, &haveMore, &s);
  ASSERT(!haveMore);
  
  // Now we're going to send the statistics out:
  
  std::stringstream statsCommand;
  statsCommand << newCmd << " emitStatistics 12 100 5000";
  CPPUNIT_ASSERT_NO_THROW(
    m_pInterpObj->Eval(statsCommand.str())
  );
  
  // Absorb the header and id:
  
  zmq::message_t hdr2;
  zmq::message_t runid2;
  zmq::message_t stats;
  
  m_pReceiver->recv(&hdr2);
  m_pReceiver->getsockopt(ZMQ_RCVMORE, &haveMore, &s);
  ASSERT(haveMore);
  
  m_pReceiver->recv(&runid2);
  m_pReceiver->getsockopt(ZMQ_RCVMORE, &haveMore, &s);
  ASSERT(haveMore);
  
  // recieve the statistics, and last segment
  
  m_pReceiver->recv(&stats);
  m_pReceiver->getsockopt(ZMQ_RCVMORE, &haveMore, &s);
  ASSERT(!haveMore);
  
  // Look at the statistics info.  The tod and elapsed time
  // are not really something we can reliably test.
  
  CStatusDefinitions::ReadoutStatCounters* pStats =
    reinterpret_cast<CStatusDefinitions::ReadoutStatCounters*>(stats.data());
  EQ(uint64_t(12), pStats->s_triggers);
  EQ(uint64_t(100), pStats->s_events);
  EQ(uint64_t(5000), pStats->s_bytes);
}