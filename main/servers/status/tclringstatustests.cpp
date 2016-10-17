// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include <TCLInterpreter.h>
#include <TCLException.h>
#include <tcl.h>
#include <zmq.hpp>
#include <Asserts.h>
#include <sstream>


std::string uri = "inproc://test";    // Use intra process sockets.

class TclRingStatusTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(TclRingStatusTests);
  CPPUNIT_TEST(construct);
  CPPUNIT_TEST_SUITE_END();


private:
  CTCLInterpreter* m_pInterpObj;
  Tcl_Interp*      m_pInterpRaw;
  zmq::context_t*  m_pZmqContext;
  zmq::socket_t*   m_pReceiver;
public:
  void setUp() {
    // Create the interpreter, fish out the raw one for convenience and load the pkg.
    
    m_pInterpObj = new CTCLInterpreter();
    m_pInterpRaw = m_pInterpObj->getInterpreter();
    Tcl_Init(m_pInterpRaw);
    std::stringstream apath;
    apath << "lappend auto_path " << TCLLIBPATH << std::endl;
    m_pInterpObj->GlobalEval(apath.str());
    const char* version = Tcl_PkgRequire(m_pInterpRaw, "statusMessage", "1.0", 0);
    ASSERT(version);
    
    //  Setup zmq - our receiver socket will be bound to the uri as a SUB socket.
    
    m_pZmqContext = new zmq::context_t(1);
    m_pReceiver   = new zmq::socket_t(*m_pZmqContext, ZMQ_SUB);
    m_pReceiver->bind(uri.c_str());
    
    
  }
  void tearDown() {
    // Get rid of all this stuff - interpreter first so that the socket is
    // destroyed first:
    
    delete m_pInterpObj;
    delete m_pReceiver;
    delete m_pZmqContext;
    
  }
protected:
  void construct();
};

CPPUNIT_TEST_SUITE_REGISTRATION(TclRingStatusTests);

// Constructing a new object means making a new command that will be returned
// by the construction command:

void TclRingStatusTests::construct() {
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
