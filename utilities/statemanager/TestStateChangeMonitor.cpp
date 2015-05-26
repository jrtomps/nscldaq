// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"

#include "Asserts.h"
#include <CPortManager.h>
#include <CPortManagerException.h>
#include <os.h>
#include <string>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <iostream>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

#include "CVarMgrServerApi.h"
#include <CVariableDb.h>

#include "CStateClientApi.h"


static const std::string serviceName("vardb-request");
static const std::string serverName("vardbServer");

class ScmonTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(ScmonTests);
  CPPUNIT_TEST(goodurls);
  CPPUNIT_TEST(badrequrl);
  CPPUNIT_TEST(badsuburl);
  CPPUNIT_TEST(badprogram);
  CPPUNIT_TEST(initialstate);
  CPPUNIT_TEST(initialstateStandalone);
  CPPUNIT_TEST(initialStandaloneFalse);
  CPPUNIT_TEST(initialStandaloneTrue);
  
  // Getters that interact directly with the server:
  
  CPPUNIT_TEST(getInitialTitle);
  CPPUNIT_TEST(getModifiedTitle);
  CPPUNIT_TEST(getInitialRunNum);
  CPPUNIT_TEST(getModifiedRunNum);
  CPPUNIT_TEST(getInitialRecording);
  CPPUNIT_TEST(getModifiedRecording);
  CPPUNIT_TEST(getinRing);
  CPPUNIT_TEST(getModifiedInRing);
  CPPUNIT_TEST(getoutRing);
  CPPUNIT_TEST(getModifiedOutRing);
  CPPUNIT_TEST(getInitialEnable);
  CPPUNIT_TEST(getModifiedEnable);
  
  
  // Now test changes spotted by the monitor thread.
  //   Remember standalone is monitored.
  
  CPPUNIT_TEST(noTransition);
  CPPUNIT_TEST(transition);
  CPPUNIT_TEST(multitransition);
  CPPUNIT_TEST(saloneNoTransition);
  CPPUNIT_TEST(saloneGlobalTransition);
  CPPUNIT_TEST(saloneTransition);
  
  // We can initiate state changes.
  
  CPPUNIT_TEST(changeState);
  CPPUNIT_TEST(changeStateWithNotify);
  
  CPPUNIT_TEST_SUITE_END();

protected:
  void goodurls();
  void badrequrl();
  void badsuburl();
  void badprogram();
  void initialstate();
  void initialstateStandalone();
  void initialStandaloneFalse();
  void initialStandaloneTrue();
  
  void getInitialTitle();
  void getModifiedTitle();
  void getInitialRunNum();
  void getModifiedRunNum();
  void getInitialRecording();
  void getModifiedRecording();
  void getinRing();
  void getModifiedInRing();
  void getoutRing();
  void getModifiedOutRing();
  void getInitialEnable();
  void getModifiedEnable();
  
  void noTransition();
  void transition();
  void multitransition();
  void saloneNoTransition();
  void saloneGlobalTransition();
  void saloneTransition();
  
  void changeState();
  void changeStateWithNotify();
private:
    pid_t m_serverPid;
    int m_serverRequestPort;
    char m_tempFile[200];
    CVarMgrServerApi* m_pApi;
    int m_fd;
    

public:
  void setUp() {
    // Set up the temp file as an empty db:
    
    strcpy(m_tempFile, "testvardbXXXXXX");
    m_fd = mkstemp(m_tempFile);
    if(m_fd == -1) {
        throw std::runtime_error(strerror(errno));
    }
    CVariableDb::create(m_tempFile);
    
    // Start the server and figure out the port:
    
    m_serverPid         = startServer();
    m_serverRequestPort = getRequestPort();
    
    // Create an API object for that port:
    
    m_pApi = new CVarMgrServerApi("localhost", m_serverRequestPort);
    populateDb();
  }
  void tearDown() {
    delete m_pApi;
    stopServer();
    
    unlink(m_tempFile);
  }
private:
    pid_t startServer();
    int getRequestPort();
    void stopServer();
    void populateDb();
};


// Utility methods:

/**
 * populateDb
 *    Populate the /RunState and /RunState/test directories sufficiently
 *     for this test.
 *     m_pApi is assumed to be already created/connected:
 */
void
ScmonTests::populateDb()
{
    // /RunState:
    
    m_pApi->mkdir("/RunState");
    m_pApi->declare("/RunState/RunNumber", "integer");
    m_pApi->declare("/RunState/Title", "string");
    m_pApi->declare("/RunState/State","string", "0Initial");   // Good enough for testing.
    m_pApi->declare("/RunState/Recording", "string", "false");
    m_pApi->declare("/RunState/Timeout", "integer", "60");
    m_pApi->declare("/RunState/ReadoutParentDir", "string");
    
    // /RunState/test  - program named test.
    
    m_pApi->mkdir("/RunState/test");
    m_pApi->declare("/RunState/test/State", "string", "Ready");
    m_pApi->declare("/RunState/test/enable", "string", "true");
    m_pApi->declare("/RunState/test/standalone", "string", "false");
    m_pApi->declare("/RunState/test/path", "string", "/bin/false");
    m_pApi->declare("/RunState/test/host", "string", "localhost");
    m_pApi->declare("/RunState/test/outring", "string", "output");
    m_pApi->declare("/RunState/test/inring", "string", "tcp://localhost/fox");
}

/**
 *  startServer
 *     start thedatabase server.  We can assume the pre-processor constant
 *     BINDIR points to the binary directory in which the server lives.
 *     The server itself is named 'vardbServer' in that directory.
 *
 * @return int - server pid (return from fork).  Failure to fork results
 *               in exit with error status/message.
 *               Failure to exec will be deteced by getRequestPort timing
 *               out on service location.
 */
pid_t
ScmonTests::startServer()
{
    pid_t pid = fork();
    if (pid == -1) {
        perror("Failed to fork ");
        exit(-1);             // Should make test fail.
    }
    // Parent process.
    
    if (pid != 0) {
        return pid;
    }
    // Child process:
    
    std::string serverPath = BINDIR;
    serverPath += "/";
    serverPath += serverName;
    
    // Need to provide the --database switch
    
    
    if (execl(serverPath.c_str(), serverPath.c_str(), "--database", m_tempFile, NULL) ==  -1) {
        perror("Exec of server failed");
        exit(-1);                  // Exec failed.
    }
    return 0;                 // In case compiler cares.
}
/**
 * getRequestPort
 *    Attempt to translate the request port. We try to locate the
 *    port every 100ms or so.  If not able to do so after 3 seconds,
 *    we assume the server failed to start properly and exit with
 *    an error message and error status.
 *
 *  @return int - port number bound to the server's varmgr-request service.
 *  @note For testing the server is always in localhost.
 */
int
ScmonTests::getRequestPort()
{
    CPortManager mgr("localhost");
    int msWaited = 0;
    std::string userName = Os::whoami();
    
    while (msWaited < 3000) {
        std::vector<CPortManager::portInfo> services = mgr.getPortUsage();
        for (int i =0; i < services.size(); i++) {
            if ((services[i].s_User == userName) &&
                (services[i].s_Application == serviceName)
            ) {
                return services[i].s_Port;
            }
        }
        usleep(100000);    // 100ms.
        msWaited += 100;
    }
    std::cerr << "Unable to locate server request port -- likely server died?\n";
    exit(-1);
}
/**
 * stopServer
 *     sigkill the server.
 */
void ScmonTests::stopServer()
{
    int exitStatus;
    kill(m_serverPid, SIGKILL);
    waitpid(m_serverPid, &exitStatus, 0);
    
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScmonTests);

// No exceptions when we use good URIs:
// server is running in localhost:

void ScmonTests::goodurls() {
    CPPUNIT_ASSERT_NO_THROW(
        CStateClientApi("tcp://localhost", "tcp://localhost", "test")
    );
}

// Two versions of bad uris:
// URIs must use tcp protocol and the service must exist.

void ScmonTests::badrequrl()
{
    CPPUNIT_ASSERT_THROW(
        CStateClientApi("file:///bin/false", "tcp://localhost", "test"),
        CStateClientApi::CException
    );
    CPPUNIT_ASSERT_THROW(
        CStateClientApi(
            "tcp://localhost/no-such-service", "tcp://localhost", "test"
        ),
        CStateClientApi::CException
    );
}

// Same as above but for the subscription URI.

void ScmonTests::badsuburl()
{
    CPPUNIT_ASSERT_THROW(
        CStateClientApi("tcp://localhost", "file:///bin/false", "test"),
        CStateClientApi::CException
    );
    CPPUNIT_ASSERT_THROW(
        CStateClientApi(
            "tcp://localhost", "tcp://localhost/no-such-services", "test"
        ),
        CStateClientApi::CException
    );
}

// If the program is not in existence, this will throw too.
// we'll test this by using something other than 'test' and by
// moving the base directory too:

void ScmonTests::badprogram()
{
    CPPUNIT_ASSERT_THROW(
        CStateClientApi("tcp://localhost", "tcp://localhost", "george"),
        CStateClientApi::CException
    );
    
    // Move the base of the programs to somewhere else:
    
    m_pApi->mkdir("/programs");
    m_pApi->set("/RunState/ReadoutParentDir", "/programs");
    
    // There's no test there:
    
    CPPUNIT_ASSERT_THROW(
        CStateClientApi("tcp://localhost", "tcp://localhost", "test"),
        CStateClientApi::CException
    );
}

// On construction the initial state must have been gotten:

// For not standalone:

void ScmonTests::initialstate()
{
    CStateClientApi api("tcp://localhost", "tcp://localhost", "test");
    EQ(std::string("0Initial"), api.getState());
}

// switching to standalone mode:

void ScmonTests::initialstateStandalone()
{
    m_pApi->set("/RunState/test/standalone", "true");
    CStateClientApi api("tcp://localhost", "tcp://localhost", "test");
   
    
    EQ(std::string("Ready"), api.getState());
}

// Check that the standalone flag is correct when we are not standalone (default)

void ScmonTests::initialStandaloneFalse()
{
    CStateClientApi api("tcp://localhost", "tcp://localhost", "test");
    ASSERT(! api.isStandalone());
}
// true:

void ScmonTests::initialStandaloneTrue()
{
    m_pApi->set("/RunState/test/standalone", "true");
    CStateClientApi api("tcp://localhost", "tcp://localhost", "test");
    ASSERT(api.isStandalone());
    
}

// Title as initialized by SetUp is an empty string:

void ScmonTests::getInitialTitle()
{
    CStateClientApi api("tcp://localhost", "tcp://localhost", "test");
    EQ(std::string(""), api.title());
}

// Title when we modify it:

void ScmonTests::getModifiedTitle()
{
    CStateClientApi api("tcp://localhost", "tcp://localhost", "test");
    EQ(std::string(""), api.title());
    
    std::string newTitle = "This is a new title";
    m_pApi->set("/RunState/Title", newTitle.c_str());
    EQ(newTitle, api.title());
}

// Initial run number is 0:

void ScmonTests::getInitialRunNum()
{
    CStateClientApi api("tcp://localhost", "tcp://localhost", "test");
    EQ(0, api.runNumber());
    
}
// Modified runnumber:

void ScmonTests::getModifiedRunNum()
{
    CStateClientApi api("tcp://localhost", "tcp://localhost", "test");
    m_pApi->set("/RunState/RunNumber", "1234");
    EQ(1234, api.runNumber());
}

// Recording gflag:

void ScmonTests::getInitialRecording()
{
    CStateClientApi api("tcp://localhost", "tcp://localhost", "test");
    EQ(false, api.recording());
}

void ScmonTests::getModifiedRecording()
{
    m_pApi->set("/RunState/Recording", "true");
    CStateClientApi api("tcp://localhost", "tcp://localhost", "test");
    
    EQ(true, api.recording());
}

void ScmonTests::getinRing()
{
    CStateClientApi api("tcp://localhost", "tcp://localhost", "test");
    EQ(std::string("tcp://localhost/fox"), api.inring());
}

void ScmonTests::getModifiedInRing()
{
    std::string inringName = "tcp://localhost/fox1";
    m_pApi->set("/RunState/test/inring", inringName.c_str());
    
    CStateClientApi api("tcp://localhost", "tcp://localhost", "test");
    EQ(inringName, api.inring());
}

// Output ring:

void ScmonTests::getoutRing()
{
    CStateClientApi api("tcp://localhost", "tcp://localhost", "test");
    EQ(std::string("output"), api.outring());
}
void ScmonTests::getModifiedOutRing()
{
    m_pApi->set("/RunState/test/outring", "outring");
    
    CStateClientApi api("tcp://localhost", "tcp://localhost", "test");
    EQ(std::string("outring"), api.outring());
}


// Enable flag.

void ScmonTests::getInitialEnable()
{
    CStateClientApi api("tcp://localhost", "tcp://localhost", "test");
    EQ(true, api.isEnabled());
}

void ScmonTests::getModifiedEnable()
{
    m_pApi->set("/RunState/test/enable", "false");
    
    CStateClientApi api("tcp://localhost", "tcp://localhost", "test");
    EQ(false, api.isEnabled());
}

// Transition notification queue:

// If there are no transitions, waitTransition returns false.

void ScmonTests::noTransition()
{
    CStateClientApi api("tcp://localhost", "tcp://localhost", "test");
    
    std::string newState;
    
    ASSERT(! api.waitTransition(newState, 100));
}
// Transition from 0Initial to Readying:

void ScmonTests::transition()
{
    CStateClientApi api("tcp://localhost", "tcp://localhost", "test");
    
    m_pApi->set("/RunState/State", "Readying");
    
    std::string newState;
    ASSERT(api.waitTransition(newState, 1000));   // should know within a second.
    EQ(std::string("Readying"), newState);
}

// For multiple transitions we should get the last one:

void ScmonTests::multitransition()
{
    CStateClientApi api("tcp://localhost", "tcp://localhost", "test");
    
    m_pApi->set("/RunState/State", "Readying");
    m_pApi->set("/RunState/State", "Ready");
    usleep(100);                                   // Give the monitor thread time to see
    std::string newState;                          // both notifications...
    ASSERT(api.waitTransition(newState, 1000));   // should know within a second.
    
    EQ(std::string("Ready"), newState);
}

// Standalone with no transitions should still update the m_standalone var:

void ScmonTests::saloneNoTransition()
{
    CStateClientApi api("tcp://localhost", "tcp://localhost", "test");
    
    m_pApi->set("/RunState/test/standalone", "true");
    std::string newState;
    ASSERT(!api.waitTransition(newState, 1000));
    
    EQ(true, api.isStandalone());
}
// Standalone mode should ignore global transitions:

void ScmonTests::saloneGlobalTransition()
{
    CStateClientApi api("tcp://localhost", "tcp://localhost", "test");
    
    m_pApi->set("/RunState/test/standalone", "true");
    m_pApi->set("/RunState/State", "Readying");
    
    std::string newState;
    ASSERT(!api.waitTransition(newState, 1000));
    
    EQ(true, api.isStandalone());
}

// On the other hand transitions on the local state variable won't be ignored
// in local mode:

void ScmonTests::saloneTransition()
{
    CStateClientApi api("tcp://localhost", "tcp://localhost", "test");
    
    m_pApi->set("/RunState/test/standalone", "true");
    m_pApi->set("/RunState/test/State", "NotReady");
    
    std::string newState;
    ASSERT(api.waitTransition(newState, 1000));
    EQ(std::string("NotReady"), newState);
    EQ(true, api.isStandalone());
}

// State changes should not only modify the state but the cached state.

void ScmonTests::changeState()
{
    CStateClientApi api("tcp://localhost", "tcp://localhost", "test");
    api.setState("Active");
    
    EQ(std::string("Active"), m_pApi->get("/RunState/test/State"));
    EQ(std::string("Active"), api.getState());
}

// State changes in standalone mode get notified...regardess of source.

void ScmonTests::changeStateWithNotify()
{
    CStateClientApi api("tcp://localhost", "tcp://localhost", "test");
    m_pApi->set("/RunState/test/standalone", "true");
    
    api.setState("Active");
    
    std::string newState;
    ASSERT(api.waitTransition(newState, 1000));
    EQ(std::string("Active"), newState);
}
/*------------------------------------------------------------------------*/
// Cause we have libtcl++  -- need to get rid of this somehow.
void* gpTCLApplication(0);


