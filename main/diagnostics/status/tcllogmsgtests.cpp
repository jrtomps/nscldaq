// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include "CStatusMessage.h"
#include "CTCLRingStatistics.h"
#include "CTCLLogMessage.h"
#include <TclUtilities.h>

#include <TCLInterpreter.h>
#include <TCLObject.h>
#include <TCLException.h>
#include <os.h>
#include <zmq.hpp>
#include <testutils.h>
#include <sstream>

std::string uri = "inproc://construct";
std::string app = "TestApp";




class TclLogTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(TclLogTests);
  // Construction tests:
  
  CPPUNIT_TEST(construct);
  CPPUNIT_TEST(constructNeedApp);
  CPPUNIT_TEST(constructTooMany);

  //
  
  CPPUNIT_TEST(destroy);
  CPPUNIT_TEST(destroyNoSuch);
  
  // Log message
  
  CPPUNIT_TEST(logMessage);
  CPPUNIT_TEST_SUITE_END();


private:
  CTCLInterpreter* m_pInterpObj;
  CTCLLogMessage * m_pObject;
  Tcl_Interp*      m_pInterpRaw;
  
  zmq::context_t*  m_pZmqContext;
  zmq::socket_t*   m_pReceiver;
public:
  void setUp() {
    // Create the interpreter, fish out the raw one for convenience and load the pkg.
    
    m_pInterpObj = new CTCLInterpreter();
    m_pInterpRaw = m_pInterpObj->getInterpreter();
    Tcl_Init(m_pInterpRaw);
    m_pObject = new CTCLLogMessage(*m_pInterpObj);

    
    //  Setup zmq - our receiver socket will be bound to the uri as a SUB socket.
    
    m_pZmqContext = &CStatusDefinitions::ZmqContext::getInstance();
    m_pReceiver   = new zmq::socket_t(*m_pZmqContext, ZMQ_PULL);
    m_pReceiver->bind(uri.c_str());
  }
  void tearDown() {
    // Get rid of all this stuff - interpreter first so that the socket is
    // destroyed first:
    
    delete m_pObject;
    delete m_pInterpObj;
    delete m_pReceiver;
    CStatusDefinitions::ZmqContext::reset();
  }
protected:
  void construct();
  void constructNeedApp();
  void constructTooMany();
  
  void destroy();
  void destroyNoSuch();
  
  void logMessage();
  
};

CPPUNIT_TEST_SUITE_REGISTRATION(TclLogTests);

// Constructing a wrapped object creates a new command:

void TclLogTests::construct() {
  std::string newCmd;
  std::stringstream constCommand;
  constCommand << "LogMessage create " << uri << " " << app;
  CPPUNIT_ASSERT_NO_THROW(
    newCmd = m_pInterpObj->Eval(constCommand.str())
  );
  
  std::stringstream infoCmd;
  infoCmd << "info commands " << newCmd;
  EQ(newCmd, m_pInterpObj->Eval(infoCmd.str()));
}

// Constructing requires two parameters else its an error  which results
// in a CTCLException.

void TclLogTests::constructNeedApp()
{

  std::stringstream constCommand;
  constCommand << "LogMessage create " << uri ;
  CPPUNIT_ASSERT_THROW(
    m_pInterpObj->Eval(constCommand.str()),
    CTCLException
  );
}

// Construction can  have too many parameters:

void TclLogTests::constructTooMany()
{
  std::stringstream constCommand;
  constCommand << "LogMessage create " << uri << " " << app << " extra";
  CPPUNIT_ASSERT_THROW(
    m_pInterpObj->Eval(constCommand.str()),
    CTCLException
  );
}
// Destruction removes command:

void TclLogTests::destroy()
{
  std::string newCmd;
  std::stringstream constCommand;
  constCommand << "LogMessage create " << uri << " " << app;
  CPPUNIT_ASSERT_NO_THROW(
    newCmd = m_pInterpObj->Eval(constCommand.str())
  );
  
  std::stringstream destroyCommand;
  destroyCommand << "LogMessage destroy " << newCmd;
  CPPUNIT_ASSERT_NO_THROW(
    m_pInterpObj->Eval(destroyCommand.str())
  );
  
  std::stringstream infoCmd;
  infoCmd << "info commands " << newCmd;
  EQ(std::string(""), m_pInterpObj->Eval(infoCmd.str())); // no commands match.
}

// Destruction of a command that does not exist throws:

void TclLogTests::destroyNoSuch()
{
  CPPUNIT_ASSERT_THROW(
    m_pInterpObj->Eval("LogMessage destroy no-such-object"),
    CTCLException
  );
}

// See if we can make a good log message:

void TclLogTests::logMessage()
{

  std::string newCmd;
  std::stringstream constCommand;
  constCommand << "LogMessage create " << uri << " " << app;
  CPPUNIT_ASSERT_NO_THROW(
    newCmd = m_pInterpObj->Eval(constCommand.str())
  );
  
  std::stringstream logCommand;
  logCommand << newCmd << " Log DEBUG {Testing the log}";
  
  CPPUNIT_ASSERT_NO_THROW(
    m_pInterpObj->Eval(logCommand.str())   // Should send the messages.
  );
  
  zmq::message_t hdr;
  zmq::message_t body;
  
  int64_t more(0);
  size_t  s(sizeof(more));
  
  m_pReceiver->recv(&hdr);
  m_pReceiver->getsockopt(ZMQ_RCVMORE, &more, &s);
  ASSERT(more);
  
  m_pReceiver->recv(&body);
  m_pReceiver->getsockopt(ZMQ_RCVMORE, &more, &s);
  ASSERT(!more);

  // Analyze the header:
  
  CStatusDefinitions::Header* pHeader =
    reinterpret_cast<CStatusDefinitions::Header*>(hdr.data());
  EQ(CStatusDefinitions::MessageTypes::LOG_MESSAGE, pHeader->s_type);
  EQ(CStatusDefinitions::SeverityLevels::DEBUG, pHeader->s_severity);
  EQ(app, std::string(pHeader->s_application));
  EQ(Os::hostname(), std::string(pHeader->s_source));
  
  // Analyze the body:
  
  CStatusDefinitions::LogMessageBody* pBody =
    reinterpret_cast<CStatusDefinitions::LogMessageBody*>(body.data());
  EQ(std::string("Testing the log"), std::string(pBody->s_message));
}
