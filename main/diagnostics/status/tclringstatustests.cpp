// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include <TCLInterpreter.h>
#include <TCLException.h>
#include <CTCLRingStatistics.h>
#include <TclUtilities.h>

#include <tcl.h>
#include <zmq.hpp>
#include <Asserts.h>
#include <sstream>
#include <os.h>
#include <stdlib.h>
#include <testutils.h>
#include <stdexcept>


#define private public
#include "CStatusMessage.h"
#undef private


static std::string uri = "inproc://test";    // Use intra process sockets.

class TclRingStatisticsTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(TclRingStatisticsTests);
  CPPUNIT_TEST(construct);
  CPPUNIT_TEST(constructWithApp);
  CPPUNIT_TEST(construct2few);
  CPPUNIT_TEST(construct2many);
  
  CPPUNIT_TEST(destruct);
  CPPUNIT_TEST(destructNoSuch);
  
  // These tests actually send messages.
  
  CPPUNIT_TEST(minimal);
  CPPUNIT_TEST(producer);
  CPPUNIT_TEST(consumers);
  CPPUNIT_TEST_SUITE_END();


private:
  CTCLInterpreter* m_pInterpObj;
  CTCLRingStatistics* m_pStatsCmd;
  Tcl_Interp*      m_pInterpRaw;
  
  zmq::context_t*    m_pZmqContext;
  zmq::socket_t*   m_pReceiver;
public:
  void setUp() {
    // Create the interpreter, fish out the raw one for convenience and load the pkg.
    
    m_pInterpObj = new CTCLInterpreter();
    m_pInterpRaw = m_pInterpObj->getInterpreter();
    Tcl_Init(m_pInterpRaw);
    m_pStatsCmd = new CTCLRingStatistics(*m_pInterpObj);
    
    //  Setup zmq - our receiver socket will be bound to the uri as a SUB socket.
    
    m_pZmqContext = &CStatusDefinitions::ZmqContext::getInstance();
    m_pReceiver   = new zmq::socket_t(*m_pZmqContext, ZMQ_PULL);
    m_pReceiver->bind(uri.c_str());
    
    
  }
  void tearDown() {
    // Get rid of all this stuff - interpreter first so that the socket is
    // destroyed first:
    
    delete m_pStatsCmd;
    delete m_pInterpObj;
    delete m_pReceiver;
    CStatusDefinitions::ZmqContext::reset();
  }
protected:
  void construct();
  void constructWithApp();
  void construct2few();
  void construct2many();
  
  void destruct();
  void destructNoSuch();
  
  void minimal();
  void producer();
  void consumers();
};

CPPUNIT_TEST_SUITE_REGISTRATION(TclRingStatisticsTests);

// Constructing a new object means making a new command that will be returned
// by the construction command:

void TclRingStatisticsTests::construct() {
  std::stringstream cmd;
  cmd << "RingStatistics create " << uri;
  std::string newCmd;
  CPPUNIT_ASSERT_NO_THROW(
     newCmd = m_pInterpObj->Eval(cmd.str())
  );
  
  // newCmd should be a command in the interpreter:
  
  std::stringstream checkCmd;
  checkCmd << "info command " << newCmd;
  EQ(newCmd, m_pInterpObj->Eval(checkCmd.str()));
  
}

// Can also construct with an appname:

void TclRingStatisticsTests::constructWithApp()
{
  std::stringstream cmd;
  cmd << "RingStatistics create " << uri << " MyApp";
  std::string newCmd;
  CPPUNIT_ASSERT_NO_THROW(
    newCmd = m_pInterpObj->Eval(cmd.str())
  );
  // newCmd should be a command:
  
  std::stringstream checkCmd;
  checkCmd << "info command " << newCmd;
  EQ(newCmd, m_pInterpObj->Eval(checkCmd.str()));
}


// Constructing with too few parameters is an error:

void TclRingStatisticsTests::construct2few()
{
  CPPUNIT_ASSERT_THROW(
    m_pInterpObj->Eval("RingStatistics create"),
    CTCLException
  );
}
// Constructing with too many parameters an error.

void TclRingStatisticsTests::construct2many()
{
  CPPUNIT_ASSERT_THROW(
    m_pInterpObj->Eval("RingStatistics create inproc://test myap extra"),
    CTCLException
  );
}

// A constructed item should be destroyable:

void TclRingStatisticsTests::destruct()
{
std::stringstream cmd;
  cmd << "RingStatistics create " << uri;
  std::string newCmd;
  CPPUNIT_ASSERT_NO_THROW(
     newCmd = m_pInterpObj->Eval(cmd.str())
  );
  
  // newCmd should be a command in the interpreter:
  
  std::stringstream checkCmd;
  checkCmd << "info command " << newCmd;
  EQ(newCmd, m_pInterpObj->Eval(checkCmd.str()));
  
  // Destroy the command:
  
  std::string destroyCommand = "RingStatistics destroy ";
  destroyCommand            += newCmd;
  CPPUNIT_ASSERT_NO_THROW(
    m_pInterpObj->Eval(destroyCommand)
  );
  
  // The command should be gone:
  
  EQ(std::string(""), m_pInterpObj->Eval(checkCmd.str()));
}

// Invalid argument if destroying a command that does not exist... gets
// converted into TCL_ERROR which converts to CTCLException:

void TclRingStatisticsTests::destructNoSuch()
{
  CPPUNIT_ASSERT_THROW(
    m_pInterpObj->Eval("RingStatistics destroy junk"),
    CTCLException
  );
}

// Minimal message is a header segment with a ring id segment.

void TclRingStatisticsTests::minimal()
{
  std::stringstream cmd;
  cmd << "RingStatistics create " << uri;
  std::string newCmd = m_pInterpObj->Eval(cmd.str()); // Default appname.  
  
  // Start the message:
  
  std::stringstream startMsg;
  startMsg << newCmd << " startMessage " << "aring";
  CPPUNIT_ASSERT_NO_THROW(
    m_pInterpObj->Eval(startMsg.str())
  );
  
  // End the message:
  
  std::stringstream endMsg;
  endMsg << newCmd << " endMessage";
  CPPUNIT_ASSERT_NO_THROW(
    m_pInterpObj->Eval(endMsg.str())
  );
  
  // We should be able to receive hdr and ring id message parts:
  
  zmq::message_t hMsg;
  m_pReceiver->recv(&hMsg);
  int64_t haveMore(0);
  size_t size(sizeof(haveMore));
  m_pReceiver->getsockopt(ZMQ_RCVMORE, &haveMore, &size);
  ASSERT(haveMore);
  
  zmq::message_t rIdMsg;
  m_pReceiver->recv(&rIdMsg);
  m_pReceiver->getsockopt(ZMQ_RCVMORE, &haveMore, &size);
  ASSERT(!haveMore);
  
  // Analyze the messages themselves.
 
 // The header message part:
 
  CStatusDefinitions::Header* pHeader =
    reinterpret_cast<CStatusDefinitions::Header*>(hMsg.data());
  EQ(CStatusDefinitions::MessageTypes::RING_STATISTICS, pHeader->s_type);
  EQ(CStatusDefinitions::SeverityLevels::INFO,          pHeader->s_severity);
  std::string app = pHeader->s_application;
  EQ(std::string("RingStatDaemon"), app);
  EQ(Os::hostname(), std::string(pHeader->s_source));
  
  // The ring id message part:
  
  CStatusDefinitions::RingStatIdentification* pRingId =
    reinterpret_cast<CStatusDefinitions::RingStatIdentification*>(rIdMsg.data());
  EQ(std::string("aring"), std::string(pRingId->s_ringName));
  
}
// Let's add a producer to the message:
// That should give us three message parts with the last one identifying the
// producer.

void TclRingStatisticsTests::producer()
{
  std::stringstream cmd;
  cmd << "RingStatistics create " << uri;
  std::string newCmd = m_pInterpObj->Eval(cmd.str()); // Default appname.  
  
  // Start the message:
  
  std::stringstream startMsg;
  startMsg << newCmd << " startMessage " << "aring";
  CPPUNIT_ASSERT_NO_THROW(
    m_pInterpObj->Eval(startMsg.str())
  );
  // Add the producer:
  
  std::stringstream addProducer;
  addProducer << newCmd << " addProducer [list a b c] 1234 5678 6";
  CPPUNIT_ASSERT_NO_THROW(
    m_pInterpObj->Eval(addProducer.str())
  );
  
  // End the message:
  
  std::stringstream endMsg;
  endMsg << newCmd << " endMessage";
  CPPUNIT_ASSERT_NO_THROW(
    m_pInterpObj->Eval(endMsg.str())
  );
  // Get the messages... we're only going to assert there's more data for
  // the header and ring id.. the prior test tested their contents:
  
  zmq::message_t hdr;
  m_pReceiver->recv(&hdr);
  uint64_t haveMore(0);
  size_t  s(sizeof(haveMore));
  
  m_pReceiver->getsockopt(ZMQ_RCVMORE, &haveMore, &s);
  ASSERT(haveMore);
  
  zmq::message_t rid;
  m_pReceiver->recv(&rid);
  m_pReceiver->getsockopt(ZMQ_RCVMORE, &haveMore, &s);
  ASSERT(haveMore);
  
  // Get the message containing the producer statistics:
  
  zmq::message_t producerMsg;
  m_pReceiver->recv(&producerMsg);
  m_pReceiver->getsockopt(ZMQ_RCVMORE, &haveMore, &s);
  ASSERT(!haveMore);
  
  
  // Analyze the producer:
  
  CStatusDefinitions::RingStatClient* pClient =
    reinterpret_cast<CStatusDefinitions::RingStatClient*>(producerMsg.data());
  
  EQ(uint64_t(1234), pClient->s_operations);
  EQ(uint64_t(5678), pClient->s_bytes);
  ASSERT(pClient->s_isProducer);
  std::vector<std::string> command = {
      "a", "b", "c"
  };
  EQ(command, marshallVector(pClient->s_command));
}

// Put a couple of consumers in the message.

void TclRingStatisticsTests::consumers()
{
    std::stringstream cmd;
    cmd << "RingStatistics create " << uri;
    std::string newCmd = m_pInterpObj->Eval(cmd.str()); // Default appname.  
    
    // Start the message:
    
    std::stringstream startMsg;
    startMsg << newCmd << " startMessage " << "aring";
    CPPUNIT_ASSERT_NO_THROW(
      m_pInterpObj->Eval(startMsg.str())
    );
    // Add two consumers.
    
    std::stringstream addConsumer1;
    addConsumer1 << newCmd << " addConsumer [list x y z p d q] 666 999 12345 10";
    CPPUNIT_ASSERT_NO_THROW(
      m_pInterpObj->Eval(addConsumer1.str())
    );
    

    std::stringstream addConsumer2;
    addConsumer2 << newCmd << " addConsumer [list a b c d] 10 1000 555 20";
    CPPUNIT_ASSERT_NO_THROW(
      m_pInterpObj->Eval(addConsumer2.str())
    );

    
    // End the message:
    
    std::stringstream endMsg;
    endMsg << newCmd << " endMessage";
    CPPUNIT_ASSERT_NO_THROW(
      m_pInterpObj->Eval(endMsg.str())
    );
    // In receiving the message we only worry about having the right number of
    // message parts..  in a bit we'll analyze the consumer client messages.
    
    zmq::message_t hdr;
    zmq::message_t ringid;
    zmq::message_t cons1;
    zmq::message_t cons2;
    
    uint64_t haveMore(0);
    size_t   s(sizeof(haveMore));
    
    m_pReceiver->recv(&hdr);
    m_pReceiver->getsockopt(ZMQ_RCVMORE, &haveMore, &s);
    ASSERT(haveMore);
    
    m_pReceiver->recv(&ringid);
    m_pReceiver->getsockopt(ZMQ_RCVMORE, &haveMore, &s);
    ASSERT(haveMore);
    
    m_pReceiver->recv(&cons1);            // First consumer
    m_pReceiver->getsockopt(ZMQ_RCVMORE, &haveMore, &s);
    ASSERT(haveMore);
    
    m_pReceiver->recv(&cons2);           // second consumer -- last segment.
    m_pReceiver->getsockopt(ZMQ_RCVMORE, &haveMore, &s);
    ASSERT(!haveMore);
    
    // Analyze contents of cons1:
    
    CStatusDefinitions::RingStatClient* pClient =
      reinterpret_cast<CStatusDefinitions::RingStatClient*>(cons1.data());
    EQ(uint64_t(666), pClient->s_operations);
    EQ(uint64_t(999), pClient->s_bytes);
    ASSERT(!pClient->s_isProducer);
    std::vector<std::string> cons1Command={"x", "y", "z", "p", "d", "q"};
    EQ(cons1Command, marshallVector(pClient->s_command));
    
    // Analyze contents of cons2:
    
    pClient = reinterpret_cast<CStatusDefinitions::RingStatClient*>(cons2.data());
    EQ(uint64_t(10), pClient->s_operations);
    EQ(uint64_t(1000), pClient->s_bytes);
    ASSERT(!pClient->s_isProducer);
    std::vector<std::string> cons2Command = {"a", "b", "c", "d"};
    EQ(cons2Command, marshallVector(pClient->s_command));
    
}

