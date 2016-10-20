// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>

#include <tcl.h>
#include <TCLInterpreter.h>
#include <CTCLStateChangeMessage.h>
#include <CTCLRingStatistics.h>
#include <TCLException.h>
#include <zmq.hpp>
#include <Asserts.h>
#include <sstream>
#include <os.h>
#include <testutils.h>

static std::string uri = "inproc://test";    // Use intra process sockets.
static std::string app = "MyApplication";



class TclStateMsgTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(TclStateMsgTests);
  // Construction tests.
  
  CPPUNIT_TEST(construct);
  CPPUNIT_TEST(constructNoApp);
  CPPUNIT_TEST(constructTooMany);
  
  // Destruction tests:
  
  CPPUNIT_TEST(destroy);
  CPPUNIT_TEST(destroyNoSuch);
  
  // Sending messsages.
  
  CPPUNIT_TEST(log);
  CPPUNIT_TEST(logNoToState);
  CPPUNIT_TEST(logTooMany);
  
  CPPUNIT_TEST_SUITE_END();


private:
  CTCLInterpreter*        m_pInterpObj;
  CTCLStateChangeMessage* m_pCommand;
  Tcl_Interp*             m_pInterpRaw;
  
  zmq::context_t*         m_pZmqContext;
  zmq::socket_t*          m_pReceiver;
public:
  void setUp() {
   // Create the interpreter, fish out the raw one for convenience and load the pkg.
    
    m_pInterpObj = new CTCLInterpreter();
    m_pInterpRaw = m_pInterpObj->getInterpreter();
    Tcl_Init(m_pInterpRaw);
    m_pCommand= new CTCLStateChangeMessage(*m_pInterpObj);
    m_pCommand->enableTesting();
    
    //  Setup zmq - our receiver socket will be bound to the uri as a SUB socket.
    
    m_pZmqContext = &CTCLRingStatistics::m_zmqContext;
    m_pReceiver   = new zmq::socket_t(*m_pZmqContext, ZMQ_PULL);
    m_pReceiver->bind(uri.c_str());

  }
  void tearDown() {
    delete m_pCommand;
    delete m_pInterpObj;
    delete m_pReceiver;
  }
protected:
  void construct();
  void constructNoApp();
  void constructTooMany();
  
  void destroy();
  void destroyNoSuch();
  
  void log();
  void logNoToState();
  void logTooMany();
};

CPPUNIT_TEST_SUITE_REGISTRATION(TclStateMsgTests);


// Constructing a new item creates a new command

void TclStateMsgTests::construct() {
  std::string newCmd;
  std::stringstream construct;
  construct << "StateChange create " << uri << " " << app;
  
  CPPUNIT_ASSERT_NO_THROW(
    newCmd = m_pInterpObj->Eval(construct.str())
  );
  
  std::stringstream info;
  info << "info commands " << newCmd;
  EQ(newCmd, m_pInterpObj->Eval(info.str()));
}
// Constructing without an app gives a CTCLException.

void TclStateMsgTests::constructNoApp()
{
  std::stringstream construct;
  construct << "StateChange create " << uri;
  
  CPPUNIT_ASSERT_THROW(
    m_pInterpObj->Eval(construct.str()),
    CTCLException
  );
}

// Constructing with extra parameters also gives an exception:

void TclStateMsgTests::constructTooMany()
{
  std::stringstream construct;
  construct << "StateChange create " << uri << " " << app << " extraparam";
  
  CPPUNIT_ASSERT_THROW(
    m_pInterpObj->Eval(construct.str()),
    CTCLException
  );
}
// Destruction should remove a command name.

void TclStateMsgTests::destroy()
{
  std::string newCmd;
  std::stringstream construct;
  construct << "StateChange create " << uri << " " << app;
  
  CPPUNIT_ASSERT_NO_THROW(
    newCmd = m_pInterpObj->Eval(construct.str())
  );
  
  std::stringstream info;
  info << "info commands " << newCmd;
  EQ(newCmd, m_pInterpObj->Eval(info.str()));
  
  std::stringstream destroy;
  destroy << "StateChange destroy " << newCmd;
  CPPUNIT_ASSERT_NO_THROW(
    m_pInterpObj->Eval(destroy.str())
  );
  
  // The command should no longer be there:
  
  EQ(std::string(""), m_pInterpObj->Eval(info.str()));
}

// Destroying a command that does not exist is an error:

void TclStateMsgTests::destroyNoSuch()
{
  CPPUNIT_ASSERT_THROW(
    m_pInterpObj->Eval("StateChange destroy no-such-object"),
    CTCLException
  );
}

// Produce s state change message from foo to bar.  Should get a message
// with a header and a StateChangeBody.

void TclStateMsgTests::log()
{
  std::string newCmd;
  std::stringstream construct;
  construct << "StateChange create " << uri << " " << app;
  
  CPPUNIT_ASSERT_NO_THROW(
    newCmd = m_pInterpObj->Eval(construct.str())
  );
  
  std::stringstream log;
  log << newCmd << " logChange foo bar";
  
  CPPUNIT_ASSERT_NO_THROW(
    m_pInterpObj->Eval(log.str())
  );
  
  // Read the message parts:
  
  zmq::message_t hdr;
  zmq::message_t body;
  int64_t        more(0);
  size_t         s(sizeof(more));
  
  m_pReceiver->recv(&hdr);
  m_pReceiver->getsockopt(ZMQ_RCVMORE, &more, &s);
  ASSERT(more);
  
  m_pReceiver->recv(&body);
  m_pReceiver->getsockopt(ZMQ_RCVMORE, &more, &s);
  ASSERT(!more);
  
  // Check the header.  Since everyone uses a common format header we'll just
  // check the type:
  
  CStatusDefinitions::Header* pHdr =
    reinterpret_cast<CStatusDefinitions::Header*>(hdr.data());
    
  EQ(CStatusDefinitions::MessageTypes::STATE_CHANGE, pHdr->s_type);
  
  // Check the body:
  
  CStatusDefinitions::StateChangeBody* pBody =
    reinterpret_cast<CStatusDefinitions::StateChangeBody*>(body.data());
    
  EQ(std::string("foo"), std::string(pBody->s_leaving));
  EQ(std::string("bar"), std::string(pBody->s_entering));
}
// If there are too few parameters that's an error.

void TclStateMsgTests::logNoToState()
{
  std::string newCmd;
  std::stringstream construct;
  construct << "StateChange create " << uri << " " << app;
  
  CPPUNIT_ASSERT_NO_THROW(
    newCmd = m_pInterpObj->Eval(construct.str())
  );
  
  std::stringstream log;
  log << newCmd << " logChange foo";
  
  CPPUNIT_ASSERT_THROW(
    m_pInterpObj->Eval(log.str()),
    CTCLException
  );
}


// Too many parameters to a log fails too:

void TclStateMsgTests::logTooMany()
{
  std::string newCmd;
  std::stringstream construct;
  construct << "StateChange create " << uri << " " << app;
  
  CPPUNIT_ASSERT_NO_THROW(
    newCmd = m_pInterpObj->Eval(construct.str())
  );
  
  std::stringstream log;
  log << newCmd << " logChange foo bar extra-parameter";
  
  CPPUNIT_ASSERT_THROW(
    m_pInterpObj->Eval(log.str()),
    CTCLException
  ); 
}