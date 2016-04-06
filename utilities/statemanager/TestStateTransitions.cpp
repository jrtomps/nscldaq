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
#include <stdexcept>

#include "CVarMgrServerApi.h"
#include <CVariableDb.h>

#include "CStateClientApi.h"
#include "CStateManager.h"

#include <CSynchronizedThread.h>


static const std::string serviceName("vardb-request");
static const std::string serverName("vardbServer");

/*-----------------------------------------------------------------------*/

// This class is a 'program' that can execute a limited
// set of transitions:

class Program : public CSynchronizedThread
{
    std::vector<std::string> m_transitions;
    std::string              m_program;
    CStateClientApi*         m_pApi;
public:
    Program(
        std::string program,std::vector<std::string> transitions
    );
    ~Program();
    
    //  run blocks until this finishes:
    
    virtual void init();
    
    // Thread body:
    
    virtual void operator()();
};

Program::Program(
        std::string program,std::vector<std::string> transitions
) :
    m_transitions(transitions),
    m_program(program),
    m_pApi(0)
{}

Program::~Program()
{
    delete m_pApi;
}
// init creates the api which starts the message pump going.

void
Program::init()
{
    m_pApi = new CStateClientApi(
        "tcp://localhost", "tcp://localhost", m_program.c_str()
    );
}
// operator() -- wait for a global state transition then execute
//               our transition list:

void
Program::operator()()
{
    std::string newState;
    m_pApi->waitTransition(newState);
    
    for (int i = 0; i < m_transitions.size(); i++) {
        m_pApi->setState(m_transitions[i]);
    }
}
/*-------------------------------------------------------------------*/
// This class provides a way to record callbacks from waitTransition:

class CallbackRecorder
{
public:
    std::vector<std::string>  m_states;
public:
    void recordCallback(std::string state);
    static void Callback(
        CStateManager& mgr, std::string program, std::string state, void* cb
    );
};

void CallbackRecorder::recordCallback(std::string state)
{
    m_states.push_back(state);
}
void CallbackRecorder::Callback(
    CStateManager& mgr, std::string program, std::string state, void* cb
)
{
    CallbackRecorder* pThis = static_cast<CallbackRecorder*>(cb);
    pThis->recordCallback(state);
}

/*---------------------------------------------------------------------*/
// Class to record backlog messages.

class BacklogRecorder
{
public:
    std::vector<CStateTransitionMonitor::Notification> m_notifications;
    
    static void Callback(
        CStateManager& mgr, CStateTransitionMonitor::Notification notification,
        void* cd
    ) {
        BacklogRecorder* pThis = static_cast<BacklogRecorder*>(cd);
        pThis->m_notifications.push_back(notification);
    }
};

/*--------------------------------------------------------------------*/



class TestStateTransitions : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(TestStateTransitions);
  
  // Title
  
  CPPUNIT_TEST(getTitle);
  CPPUNIT_TEST(setTitle);
  
  CPPUNIT_TEST(getTimeout);
  CPPUNIT_TEST(setTimeout);
  
  CPPUNIT_TEST(getRecording);
  CPPUNIT_TEST(setRecording);
  
  CPPUNIT_TEST(getRunNumber);
  CPPUNIT_TEST(setRunNumber);
  
  // State transitions
  
  CPPUNIT_TEST(waitAlreadyThere);
  CPPUNIT_TEST(waitNotReady);
  CPPUNIT_TEST(waitNotReadyCallback);
  CPPUNIT_TEST(waitNotReadyTimeout);
  CPPUNIT_TEST(waitReadyingToReady);
  CPPUNIT_TEST(waitBeginningToActive);
  CPPUNIT_TEST(waitPausingToPaused);
  CPPUNIT_TEST(waitEndingToReady);
  CPPUNIT_TEST(waitResumingToActive);

  CPPUNIT_TEST(backlogNone);
  CPPUNIT_TEST(backlogOne);
  CPPUNIT_TEST(backlogSeveral);
  CPPUNIT_TEST(backlogPurges);
  
  CPPUNIT_TEST(isActiveYes);
  CPPUNIT_TEST(isActiveNoStandalone);
  CPPUNIT_TEST(isActiveNoDisabled);
  CPPUNIT_TEST(isActiveNoBoth);
  CPPUNIT_TEST(isActiveNoX);

  CPPUNIT_TEST(setProgramState);
  CPPUNIT_TEST(setProgramStateBadState);
  CPPUNIT_TEST(setProgramStateNoX);

  CPPUNIT_TEST(getProgramState);
  CPPUNIT_TEST(getProgramStateNox);
  CPPUNIT_TEST_SUITE_END();
  

protected:
  void getTitle();
  void setTitle();
  
  void getTimeout();
  void setTimeout();
    
  void getRecording();
  void setRecording();
  
  void setRunNumber();
  void getRunNumber();
  
 // Tests for waitTransition:
  
  void waitAlreadyThere();
  void waitNotReady();
  void waitNotReadyCallback();
  void waitNotReadyTimeout();
  void waitBeginningToActive();
  void waitReadyingToReady();
  void waitPausingToPaused();
  void waitEndingToReady();
  void waitResumingToActive();
  
  void backlogNone();
  void backlogOne();
  void backlogSeveral();
  void backlogPurges();
  
  void isActiveYes();
  void isActiveNoStandalone();
  void isActiveNoDisabled();
  void isActiveNoBoth();
  void isActiveNoX();
  
  void setProgramState();
  void setProgramStateBadState();
  void setProgramStateNoX();

  void getProgramState();
  void getProgramStateNox();
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
TestStateTransitions::populateDb()
{
    // Define the data types:
    
    CVarMgrApi::EnumValues tf;
    
    
    
   
    
    CVarMgrApi::StateMap transitions;
    m_pApi->addTransition(transitions, "0Initial", "NotReady");
    
    m_pApi->addTransition(transitions, "NotReady", "0Initial");
    m_pApi->addTransition(transitions, "NotReady", "Readying");
    
    m_pApi->addTransition(transitions, "Readying", "NotReady");
    m_pApi->addTransition(transitions, "Readying", "Ready");
    
    m_pApi->addTransition(transitions, "Ready", "Beginning");
    m_pApi->addTransition(transitions, "Ready", "NotReady");
    
    m_pApi->addTransition(transitions, "Beginning", "Active");
    m_pApi->addTransition(transitions, "Beginning", "NotReady");
    
    m_pApi->addTransition(transitions, "Active", "Pausing");
    m_pApi->addTransition(transitions, "Active", "Ending");
    m_pApi->addTransition(transitions, "Active", "NotReady");
    
    m_pApi->addTransition(transitions, "Pausing", "Paused");
    m_pApi->addTransition(transitions, "Pausing", "NotReady");
    
    m_pApi->addTransition(transitions, "Paused", "Ending");
    m_pApi->addTransition(transitions, "Paused", "Resuming");
    m_pApi->addTransition(transitions, "Paused", "NotReady");
    
    m_pApi->addTransition(transitions, "Resuming", "Active");
    m_pApi->addTransition(transitions, "Resuming", "NotReady");
    
    m_pApi->addTransition(transitions, "Ending", "Ready");
    m_pApi->addTransition(transitions, "Ending", "NotReady");
    
    m_pApi->defineStateMachine("RunStateMachine", transitions);
    
    // /RunState:
    
    m_pApi->mkdir("/RunState");
    m_pApi->declare("/RunState/RunNumber", "integer");
    m_pApi->declare("/RunState/Title", "string");
    m_pApi->declare("/RunState/State","RunStateMachine", "0Initial");   // Good enough for testing.
    m_pApi->declare("/RunState/Recording", "bool", "false");
    m_pApi->declare("/RunState/Timeout", "integer", "60");
    m_pApi->declare("/RunState/ReadoutParentDir", "string");
    
    // /RunState/test  - program named test.
    
    m_pApi->mkdir("/RunState/test");
    m_pApi->declare("/RunState/test/State", "RunStateMachine", "0Initial");
    m_pApi->declare("/RunState/test/enable", "bool", "true");
    m_pApi->declare("/RunState/test/standalone", "bool", "false");
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
TestStateTransitions::startServer()
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
TestStateTransitions::getRequestPort()
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
void TestStateTransitions::stopServer()
{
    int exitStatus;
    kill(m_serverPid, SIGKILL);
    waitpid(m_serverPid, &exitStatus, 0);
    
}



CPPUNIT_TEST_SUITE_REGISTRATION(TestStateTransitions);


void* gpTCLApplication(0);


// The tests:

// tests for waitTransition:

// Waiting we're not transitioning will timeout.

void TestStateTransitions::waitAlreadyThere()
{
    // Dial down the timeout to a second:
    
    m_pApi->set("/RunState/Timeout", "1");
    
    CStateManager sm("tcp://localhost", "tcp://localhost");
    CPPUNIT_ASSERT_THROW(
        sm.waitTransition(),
        std::runtime_error
    );
}
// Waiting for transition to NotReady from anything:
//  * Use forked process to watch global state and set
//    local state.
//  * Should not timeout
//  * Shoult not throw.
//  * Programs should reflect final state (NotReady).
//
void TestStateTransitions::waitNotReady()
{
    std::vector<std::string> programTrans;
    programTrans.push_back("NotReady");
    
    Program p("test", programTrans);
    p.start();           // It's listeningn ow.

    CStateManager sm("tcp://localhost", "tcp://localhost");
    sm.setGlobalState("NotReady");
    CPPUNIT_ASSERT_NO_THROW(
        sm.waitTransition()          // Wait for programs.
    );
    
    std::vector<std::pair<std::string, std::string> > states =
        sm.getParticipantStates();
    EQ(std::string("NotReady"), states[0].second);
    
    p.join();                   // Reap the thread.

}

// Callbacks work:

void TestStateTransitions::waitNotReadyCallback()
{
    std::vector<std::string> programTrans;
    programTrans.push_back("NotReady");

    CallbackRecorder r;
    
    Program p("test", programTrans);
    p.start();           // It's listeningn ow.

    CStateManager sm("tcp://localhost", "tcp://localhost");
    sm.setGlobalState("NotReady");

    CPPUNIT_ASSERT_NO_THROW(
        sm.waitTransition(CallbackRecorder::Callback, &r)          // Wait for programs.
    );
    
    std::vector<std::pair<std::string, std::string> > states =
        sm.getParticipantStates();
    EQ(std::string("NotReady"), states[0].second);
    
    p.join();                   // Reap the thread.
    
    EQ(size_t(1), r.m_states.size());
    EQ(std::string("NotReady"), r.m_states[0]);
    
}
// If the program is never started (crashed) we get a timeout

void TestStateTransitions::waitNotReadyTimeout()
{
    m_pApi->set("/RunState/Timeout", "1");  // Dial down the timeout.
    CStateManager sm("tcp://localhost", "tcp://localhost");
    sm.setGlobalState("NotReady");

    CPPUNIT_ASSERT_THROW(
        sm.waitTransition(),
        std::runtime_error
    );
}


// Compound transition xxx -> Readying -> Ready.
//  That would trigger a global transition to Ready as well.

void TestStateTransitions::waitReadyingToReady()
{
    
    // Prep the state machines:
    
    m_pApi->set("/RunState/State", "NotReady");  
    m_pApi->set("/RunState/test/State", "NotReady");


    std::vector<std::string> programTrans;
    programTrans.push_back("Readying");
    programTrans.push_back("Ready");

    CallbackRecorder r;
    
    Program p("test", programTrans);
    p.start();           // It's listeningn ow.

    CStateManager sm("tcp://localhost", "tcp://localhost");
    sm.setGlobalState("Readying");

    CPPUNIT_ASSERT_NO_THROW(
        sm.waitTransition(CallbackRecorder::Callback, &r)          // Wait for programs.
    );
    p.join();                   // Reap the thread.
    
    // Program is Ready.
    
    std::vector<std::pair<std::string, std::string> > states =
        sm.getParticipantStates();
    EQ(std::string("Ready"), states[0].second);
    
    // Global state is Ready.
    
    EQ(std::string("Ready"), sm.getGlobalState());
    
    // Recorded -> Readying -> Ready
    
    EQ(size_t(2), r.m_states.size());
    EQ(std::string("Readying"), r.m_states[0]);
    EQ(std::string("Ready"),    r.m_states[1]);
}
// Ready -> Beginning -> Active.

void TestStateTransitions::waitBeginningToActive()
{
    // Prep the state machines:
    
    m_pApi->set("/RunState/State", "NotReady");  
    m_pApi->set("/RunState/test/State", "NotReady");
    m_pApi->set("/RunState/State", "Readying");
    m_pApi->set("/RunState/test/State", "Readying");
    m_pApi->set("/RunState/State", "Ready");
    m_pApi->set("/RunState/test/State", "Ready");
    
    // program transitions:
    
    std::vector<std::string> programTrans;
    programTrans.push_back("Beginning");


    CallbackRecorder r;
    
    Program p("test", programTrans);
    p.start();           // It's listeningn ow.

    CStateManager sm("tcp://localhost", "tcp://localhost");
    sm.setGlobalState("Beginning");

    CPPUNIT_ASSERT_NO_THROW(
        sm.waitTransition(CallbackRecorder::Callback, &r)          // Wait for programs.
    );
    p.join();                   // Reap the thread.
    
    // Program is Ready.
    
    std::vector<std::pair<std::string, std::string> > states =
        sm.getParticipantStates();
    EQ(std::string("Beginning"), states[0].second);
    
    // Global state is Ready.
    
    EQ(std::string("Beginning"), sm.getGlobalState());
    
    // Recorded -> Readying -> Ready
    
    EQ(size_t(1), r.m_states.size());
    EQ(std::string("Beginning"), r.m_states[0]);
}

void TestStateTransitions::waitPausingToPaused()
{
    // Prep the state machines:
    
    m_pApi->set("/RunState/State", "NotReady");  
    m_pApi->set("/RunState/test/State", "NotReady");
    m_pApi->set("/RunState/State", "Readying");
    m_pApi->set("/RunState/test/State", "Readying");
    m_pApi->set("/RunState/State", "Ready");
    m_pApi->set("/RunState/test/State", "Ready");
    m_pApi->set("/RunState/State", "Beginning");
    m_pApi->set("/RunState/test/State", "Beginning");
    m_pApi->set("/RunState/State", "Active");
    m_pApi->set("/RunState/test/State", "Active");
    
// program transitions:
    
    std::vector<std::string> programTrans;
    programTrans.push_back("Pausing");
    programTrans.push_back("Paused");

    CallbackRecorder r;
    
    Program p("test", programTrans);
    p.start();           // It's listeningn ow.

    CStateManager sm("tcp://localhost", "tcp://localhost");
    sm.setGlobalState("Pausing");

    CPPUNIT_ASSERT_NO_THROW(
        sm.waitTransition(CallbackRecorder::Callback, &r)          // Wait for programs.
    );
    p.join();                   // Reap the thread.
    
    // Program is Ready.
    
    std::vector<std::pair<std::string, std::string> > states =
        sm.getParticipantStates();
    EQ(std::string("Paused"), states[0].second);
    
    // Global state is Ready.
    
    EQ(std::string("Paused"), sm.getGlobalState());
    
    // Recorded -> Readying -> Ready
    
    EQ(size_t(2), r.m_states.size());
    EQ(std::string("Pausing"), r.m_states[0]);
    EQ(std::string("Paused"),    r.m_states[1]);     

}
void TestStateTransitions::waitEndingToReady()
{
    // Prep the state machines:
    
    m_pApi->set("/RunState/State", "NotReady");  
    m_pApi->set("/RunState/test/State", "NotReady");
    m_pApi->set("/RunState/State", "Readying");
    m_pApi->set("/RunState/test/State", "Readying");
    m_pApi->set("/RunState/State", "Ready");
    m_pApi->set("/RunState/test/State", "Ready");
    m_pApi->set("/RunState/State", "Beginning");
    m_pApi->set("/RunState/test/State", "Beginning");
    m_pApi->set("/RunState/State", "Active");
    m_pApi->set("/RunState/test/State", "Active");
    
    // program transitions:
    
    std::vector<std::string> programTrans;
    programTrans.push_back("Ending");

    CallbackRecorder r;
    
    Program p("test", programTrans);
    p.start();           // It's listeningn ow.

    CStateManager sm("tcp://localhost", "tcp://localhost");
    sm.setGlobalState("Ending");

    CPPUNIT_ASSERT_NO_THROW(
        sm.waitTransition(CallbackRecorder::Callback, &r)          // Wait for programs.
    );
    p.join();                   // Reap the thread.
    
    // Program is Ready.
    
    std::vector<std::pair<std::string, std::string> > states =
        sm.getParticipantStates();
    EQ(std::string("Ending"), states[0].second);
    
    // Global state is Ready.
    
    EQ(std::string("Ending"), sm.getGlobalState());
    
    // Recorded -> Readying -> Ready
    
    EQ(size_t(1), r.m_states.size());
    EQ(std::string("Ending"), r.m_states[0]);

}
void TestStateTransitions::waitResumingToActive()
{
    // Prep the state machines:
    
    m_pApi->set("/RunState/State", "NotReady");  
    m_pApi->set("/RunState/test/State", "NotReady");
    m_pApi->set("/RunState/State", "Readying");
    m_pApi->set("/RunState/test/State", "Readying");
    m_pApi->set("/RunState/State", "Ready");
    m_pApi->set("/RunState/test/State", "Ready");
    m_pApi->set("/RunState/State", "Beginning");
    m_pApi->set("/RunState/test/State", "Beginning");
    m_pApi->set("/RunState/State", "Active");
    m_pApi->set("/RunState/test/State", "Active");
    m_pApi->set("/RunState/State", "Pausing");
    m_pApi->set("/RunState/test/State", "Pausing");
    m_pApi->set("/RunState/State", "Paused");
    m_pApi->set("/RunState/test/State", "Paused");
    
    // program transitions:
    
    std::vector<std::string> programTrans;
    programTrans.push_back("Resuming");
    programTrans.push_back("Active");

    CallbackRecorder r;
    
    Program p("test", programTrans);
    p.start();           // It's listeningn ow.

    CStateManager sm("tcp://localhost", "tcp://localhost");
    sm.setGlobalState("Resuming");

    CPPUNIT_ASSERT_NO_THROW(
        sm.waitTransition(CallbackRecorder::Callback, &r)          // Wait for programs.
    );
    p.join();                   // Reap the thread.
    
    // Program is Ready.
    
    std::vector<std::pair<std::string, std::string> > states =
        sm.getParticipantStates();
    EQ(std::string("Active"), states[0].second);
    
    // Global state is Ready.
    
    EQ(std::string("Active"), sm.getGlobalState());
    
    // Recorded -> Readying -> Ready
    
    EQ(size_t(2), r.m_states.size());
    EQ(std::string("Resuming"), r.m_states[0]);
    EQ(std::string("Active"),    r.m_states[1]);     
    
}
//   Tests for processMessages


// Call processMessages with no backlog:

void TestStateTransitions::backlogNone()
{
    BacklogRecorder r;
    CStateManager sm("tcp://localhost", "tcp://localhost");
    
    sm.processMessages(BacklogRecorder::Callback, &r);
    
    EQ(size_t(0), r.m_notifications.size());
    
}
// One backlog message:

void TestStateTransitions::backlogOne()
{
    BacklogRecorder r;
    CStateManager sm("tcp://localhost", "tcp://localhost"); //subscribes.
    
    m_pApi->set("/RunState/test/State", "NotReady");  // Program transition.
    
    usleep(5000);                                    // Let messages get sent.
    sm.processMessages(BacklogRecorder::Callback, &r);
    
    EQ(size_t(1), r.m_notifications.size());
    CStateTransitionMonitor::Notification n = r.m_notifications[0];
    
    EQ(CStateTransitionMonitor::ProgramStateChange, n.s_type);
    EQ(std::string("NotReady"), n.s_state);
    EQ(std::string("test"),     n.s_program);
}
// Several backlog messages:

void TestStateTransitions::backlogSeveral()
{
    BacklogRecorder r;
    CStateManager sm("tcp://localhost", "tcp://localhost"); //subscribes.
    
    m_pApi->set("/RunState/State", "NotReady");
    m_pApi->set("/RunState/test/State", "NotReady");  // Program transition.
    usleep(5000);                                     // Let messages get sent.
    sm.processMessages(BacklogRecorder::Callback, &r);
    
    EQ(size_t(2), r.m_notifications.size());
    
    CStateTransitionMonitor::Notification n = r.m_notifications[0];
    EQ(CStateTransitionMonitor::GlobalStateChange, n.s_type);
    EQ(std::string("NotReady"), n.s_state);

    // s_program is meaningless for globals
    
    // next message:
     
    n = r.m_notifications[1];
    EQ(CStateTransitionMonitor::ProgramStateChange, n.s_type);
    EQ(std::string("NotReady"), n.s_state);
    EQ(std::string("test"),     n.s_program);
}
// After processing a backlog, there are no more:

void TestStateTransitions::backlogPurges()
{
    BacklogRecorder r;
    CStateManager sm("tcp://localhost", "tcp://localhost"); //subscribes.
    
    m_pApi->set("/RunState/State", "NotReady");
    m_pApi->set("/RunState/test/State", "NotReady");  // Program transition.
    usleep(5000);                                     // Let messages get sent.
    sm.processMessages();
    
    // Now the backlog queue shoudl be empty:
    
    sm.processMessages(BacklogRecorder::Callback, &r);
    
    EQ(size_t(0), r.m_notifications.size());
}
//  Tests for isActive:

void TestStateTransitions::isActiveYes()
{
    CStateManager sm("tcp://localhost", "tcp://localhost"); //subscribes.
    
    // Test is active:
    
    ASSERT(sm.isActive("test"));
}
void TestStateTransitions::isActiveNoStandalone()
{
    m_pApi->set("/RunState/test/standalone", "true"); // not active.
    
    CStateManager sm("tcp://localhost", "tcp://localhost"); //subscribes.
    ASSERT(!sm.isActive("test"));
}
void TestStateTransitions::isActiveNoDisabled()
{
    m_pApi->set("/RunState/test/enable", "false");
    CStateManager sm("tcp://localhost", "tcp://localhost"); //subscribes.
    ASSERT(!sm.isActive("test"));
}

void TestStateTransitions::isActiveNoBoth()
{
    m_pApi->set("/RunState/test/enable", "false");
    m_pApi->set("/RunState/test/standalone", "true");
    
    CStateManager sm("tcp://localhost", "tcp://localhost"); //subscribes.
    ASSERT(!sm.isActive("test"));
}

void TestStateTransitions::isActiveNoX()
{
    CStateManager sm("tcp://localhost", "tcp://localhost"); //subscribes.
    CPPUNIT_ASSERT_THROW(
        sm.isActive("mytest"),
        std::runtime_error
    );
}

// Tests for set program individual state - intended for
// use with standalone programs but there's no policy enforcement
// at this level.

void TestStateTransitions::setProgramState()
{
    CStateManager sm("tcp://localhost", "tcp://localhost"); //subscribes.
    sm.setProgramState("test", "NotReady");
    EQ(std::string("NotReady"), m_pApi->get("RunState/test/State"));
}

void TestStateTransitions::setProgramStateBadState()
{
    CStateManager sm("tcp://localhost", "tcp://localhost"); //subscribes.
    CPPUNIT_ASSERT_THROW(
        sm.setProgramState("test", "Active"),  // invalid transition.
        std::runtime_error
    );

}
void TestStateTransitions::setProgramStateNoX()
{
    CStateManager sm("tcp://localhost", "tcp://localhost"); //subscribes.
    CPPUNIT_ASSERT_THROW(
        sm.setProgramState("atest", "0Initial"),
        std::runtime_error
    );
}
// getProgramState tests:

void TestStateTransitions::getProgramState()
{
    CStateManager sm("tcp://localhost", "tcp://localhost"); //subscribes.
    EQ(std::string("0Initial"), sm.getProgramState("test"));
    sm.setProgramState("test", "NotReady");
    EQ(std::string("NotReady"), sm.getProgramState("test"));
}
void TestStateTransitions::getProgramStateNox()
{
    CStateManager sm("tcp://localhost", "tcp://localhost"); //subscribes.
    CPPUNIT_ASSERT_THROW(
        sm.getProgramState("mytest"),
        std::runtime_error
    );
}


// Tests for the title overloads:

void TestStateTransitions::getTitle()
{
    const char* t = "This is a title";
    m_pApi->set("/RunState/Title", t);
    CStateManager sm("tcp://localhost", "tcp://localhost");
    EQ(std::string(t), sm.title());
    
}

void TestStateTransitions::setTitle()
{
    const char* t = "This is a title";
    CStateManager sm("tcp://localhost", "tcp://localhost");
    sm.title(t);
    
    EQ(std::string(t), sm.title());
}

// Tests for the timeout overload:

void TestStateTransitions::getTimeout()
{
    unsigned int newTo = 123;
    char newToString[100];
    sprintf(newToString, "%d", newTo);
    m_pApi->set("/RunState/Timeout", newToString);
    
    CStateManager sm("tcp://localhost", "tcp://localhost");
    EQ(newTo, sm.timeout());
}

void TestStateTransitions::setTimeout()
{
    unsigned int newTo = 123;
    CStateManager sm("tcp://localhost", "tcp://localhost");
    sm.timeout(newTo);
    EQ(newTo, sm.timeout());
}

void TestStateTransitions::getRecording()
{
    CStateManager sm("tcp://localhost", "tcp://localhost");
    
    m_pApi->set("/RunState/Recording", "false");
    ASSERT(! sm.recording());
    
    m_pApi->set("RunState/Recording", "true");
    ASSERT(sm.recording());
}

void TestStateTransitions::setRecording()
{
    CStateManager sm("tcp://localhost", "tcp://localhost");
    sm.recording(false);
    ASSERT(!sm.recording());
    sm.recording(true);
    ASSERT(sm.recording());
}


void TestStateTransitions::getRunNumber()
{
    m_pApi->set("/RunState/RunNumber", "1234");
    CStateManager sm("tcp://localhost", "tcp://localhost");
    
    EQ(unsigned(1234), sm.runNumber());
}
void TestStateTransitions::setRunNumber()
{
    CStateManager sm("tcp://localhost", "tcp://localhost");
    unsigned now = sm.runNumber();
    sm.runNumber(now+1);
    EQ(now+1, sm.runNumber());
}