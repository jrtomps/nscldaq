// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"


#include "CVarMgrServerApi.h"
#include <CVariableDb.h>
#include <CPortManager.h>
#include <os.h>
#include "CStateTransitionMonitor.h"

#include <iostream>

static const std::string serviceName("vardb-request");
static const std::string serverName("vardbServer");

class changeMonitorTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(changeMonitorTests);
  CPPUNIT_TEST(defaultParent);
  CPPUNIT_TEST(changedParent);
  
  CPPUNIT_TEST(allPrograms1Default);
  CPPUNIT_TEST(allProgramsSeveralDefault);
  CPPUNIT_TEST(allPrograms1Changed);
  CPPUNIT_TEST(allProgramsSeveralChanged);
  
  CPPUNIT_TEST(activePrograms1Default);
  CPPUNIT_TEST(inactiveProgramsDefault);
  CPPUNIT_TEST(activeProgramsSeveralDefault);
  CPPUNIT_TEST(activeProgramsSomeInactiveDefault);
  
  CPPUNIT_TEST(activePrograms1Changed);
  CPPUNIT_TEST(inactiveProgramsChanged);
  CPPUNIT_TEST(activeProgramsSeveralChanged);
  CPPUNIT_TEST(activeProgramsSomeInactiveChanged);

  CPPUNIT_TEST(standaloneDefault);
  CPPUNIT_TEST(notStandaloneDefault);
  CPPUNIT_TEST(saloneNoSuchDefault);
  CPPUNIT_TEST(standaloneChanged);
  CPPUNIT_TEST(notStandaloneChanged);
  CPPUNIT_TEST(saloneNoSuchChanged);
  
  CPPUNIT_TEST(enabled);
  CPPUNIT_TEST(notEnabled);
  CPPUNIT_TEST(enabledChanged);
  CPPUNIT_TEST(notEnabledChanged);
  
  CPPUNIT_TEST(transitionTimeout);
  CPPUNIT_TEST(modifiedTranstionTimeout);
  CPPUNIT_TEST(setTimeout);
  
  // Global and program state changes:
 
  CPPUNIT_TEST(noNotification);
  CPPUNIT_TEST(notification1);
  CPPUNIT_TEST(notificationMany);
  CPPUNIT_TEST(notificationLimited);
  
  CPPUNIT_TEST(prNotification1);
  CPPUNIT_TEST(prNotificationMany);
  CPPUNIT_TEST(prNotificationLimited);
  CPPUNIT_TEST(mixedNotifications);
  
  // Program joins

  CPPUNIT_TEST(programJoins);
  CPPUNIT_TEST(programMultiJoins);
  
  // Program leaves:
  
  CPPUNIT_TEST(programLeaves);
  CPPUNIT_TEST(programMultiLeaves);
  CPPUNIT_TEST(programJoinLeaves);
  
  CPPUNIT_TEST(varChanges);
  CPPUNIT_TEST_SUITE_END();

protected:
    void defaultParent();
    void changedParent();

    void allPrograms1Default();
    void allProgramsSeveralDefault();
    void allPrograms1Changed();
    void allProgramsSeveralChanged();
    
    void activePrograms1Default();
    void inactiveProgramsDefault();
    void activeProgramsSeveralDefault();
    void activeProgramsSomeInactiveDefault();
    
    void activePrograms1Changed();
    void inactiveProgramsChanged();
    void activeProgramsSeveralChanged();
    void activeProgramsSomeInactiveChanged();
    
    void standaloneDefault();
    void notStandaloneDefault();
    void saloneNoSuchDefault();
    void standaloneChanged();
    void notStandaloneChanged();
    void saloneNoSuchChanged();
    
    void enabled();
    void notEnabled();
    void enabledChanged();
    void notEnabledChanged();
    
    void transitionTimeout();
    void modifiedTranstionTimeout();
    void setTimeout();
    
    void noNotification();
    void notification1();
    void notificationMany();
    void notificationLimited();
    
    void prNotification1();
    void prNotificationMany();
    void prNotificationLimited();
    void mixedNotifications();
    
    void programJoins();
    void programMultiJoins();
    
    void programLeaves();
    void programMultiLeaves();
    void programJoinLeaves();
    
    void varChanges();
    
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
    void makeProgram(const char* parent, const char* name);
    void removeProgram(const char* parent, const char* name);
};


// Utility methods:

/**
 * removeProgram
 *    Destroy the variables and directory associated with a program.
 *
 *  @param parent - the directory that holds the program.
 *  @param name   - the program's name.
 */
void
changeMonitorTests::removeProgram(const char* parent, const char* name)
{
    std::string wd = m_pApi->getwd();
    
    // Compute the directory of the program and kill off the variables inside:
    
    std::string dir = parent;
    dir += "/";
    dir += name;
    
    m_pApi->cd(dir.c_str());
    
    // Kill the variables:
    
    
    m_pApi->rmvar("State");
    m_pApi->rmvar("enable");
    m_pApi->rmvar("standalone");
    m_pApi->rmvar("path");
    m_pApi->rmvar("host");
    m_pApi->rmvar("outring");
    m_pApi->rmvar("inring");
    
    // Kill the directory:
    
    m_pApi->cd("..");
    m_pApi->rmdir(name);
    
    m_pApi->cd(wd.c_str());
}

/**
 * makeProgram
 *    Make the variables needed to describe a program:
 *
 *  @param parent - parent directory name.
 *  @param name   - Name of the program.
 */
void
changeMonitorTests::makeProgram(const char* parent, const char* name)
{
    std::string programDir = parent;
    programDir += "/";
    programDir += name;
    
    m_pApi->mkdir(programDir.c_str());
    std::string wd = m_pApi->getwd();
    m_pApi->cd(programDir.c_str());
    
    
    // For simplicity all programs look alike:
    
        
    
    m_pApi->declare("State", "string", "Ready");
    m_pApi->declare("enable", "string", "true");
    m_pApi->declare("standalone", "string", "false");
    m_pApi->declare("path", "string", "/bin/false");
    m_pApi->declare("host", "string", "localhost");
    m_pApi->declare("outring", "string", "output");
    m_pApi->declare("inring", "string", "tcp://localhost/fox");

    
    m_pApi->cd(wd.c_str());
}

/**
 * populateDb
 *    Populate the /RunState and /RunState/test directories sufficiently
 *     for this test.
 *     m_pApi is assumed to be already created/connected:
 */
void
changeMonitorTests::populateDb()
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
    
    makeProgram("RunState", "test");
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
changeMonitorTests::startServer()
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
changeMonitorTests::getRequestPort()
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
void changeMonitorTests::stopServer()
{
    int exitStatus;
    kill(m_serverPid, SIGKILL);
    waitpid(m_serverPid, &exitStatus, 0);
    
}

CPPUNIT_TEST_SUITE_REGISTRATION(changeMonitorTests);

void changeMonitorTests::defaultParent() {
    CStateTransitionMonitor mon("tcp://localhost", "tcp://localhost");
    EQ(std::string("/RunState"), mon.programParentDir());
}

void changeMonitorTests::changedParent()
{
    m_pApi->mkdir("/programs");
    m_pApi->set("/RunState/ReadoutParentDir", "/programs");
    
    CStateTransitionMonitor mon("tcp://localhost", "tcp://localhost");
    EQ(std::string("/programs"), mon.programParentDir());
}

// Program enumeration

//  Without doing anything we should have a single program named 'test'.

void changeMonitorTests::allPrograms1Default()
{
    CStateTransitionMonitor mon("tcp://localhost", "tcp://localhost");
    std::vector<std::string> pgms = mon.allPrograms();
    
    EQ(size_t(1), pgms.size());
    EQ(std::string("test"), pgms[0]);
}

// Two programs in the default dir:

void changeMonitorTests::allProgramsSeveralDefault()
{
    makeProgram("/RunState", "zzz");
    CStateTransitionMonitor mon("tcp://localhost", "tcp://localhost");
    std::vector<std::string> pgms = mon.allPrograms();
    
    EQ(size_t(2), pgms.size());
    EQ(std::string("test"), pgms[0]);
    EQ(std::string("zzz"), pgms[1]);
}

// one program but in a different directory from the default:

void changeMonitorTests::allPrograms1Changed()
{
    m_pApi->mkdir("/programs");
    makeProgram("/programs", "atest");
    m_pApi->set("/RunState/ReadoutParentDir", "/programs");
    
    CStateTransitionMonitor mon("tcp://localhost", "tcp://localhost");
    std::vector<std::string> pgms = mon.allPrograms();
    
    EQ(size_t(1), pgms.size());
    EQ(std::string("atest"), pgms[0]);
}
// two programs but not in the default dir:

void changeMonitorTests::allProgramsSeveralChanged()
{
    m_pApi->mkdir("/programs");
    makeProgram("/programs", "atest");
    makeProgram("/programs", "zztest");
    m_pApi->set("/RunState/ReadoutParentDir", "/programs");
    
    CStateTransitionMonitor mon("tcp://localhost", "tcp://localhost");
    std::vector<std::string> pgms = mon.allPrograms();
    
    EQ(size_t(2), pgms.size());
    EQ(std::string("atest"), pgms[0]);
    EQ(std::string("zztest"), pgms[1]);
}

// A program in the default dir that is active:

void changeMonitorTests::activePrograms1Default()
{
    CStateTransitionMonitor mon("tcp://localhost", "tcp://localhost");
    std::vector<std::string> pgms = mon.activePrograms();
    
    EQ(size_t(1), pgms.size());
    EQ(std::string("test"), pgms[0]);
}

// No active programs either due to disabled or standalone:

void changeMonitorTests::inactiveProgramsDefault()
{
    m_pApi->set("/RunState/test/enable", "false");   // Disabled
    
    CStateTransitionMonitor mon("tcp://localhost", "tcp://localhost");
    std::vector<std::string> pgms = mon.activePrograms();
    
    EQ(size_t(0), pgms.size());
    
    m_pApi->set("/RunState/test/enable", "true");  // Enabled but...
    m_pApi->set("/RunState/test/standalone", "true"); // Standalone:
    
    pgms = mon.activePrograms();
    EQ(size_t(0), pgms.size());
}
// Several programs (2) all of them standalone:

void changeMonitorTests::activeProgramsSeveralDefault()
{
    makeProgram("/RunState", "anotherTest");
    
    CStateTransitionMonitor mon("tcp://localhost", "tcp://localhost");
    std::vector<std::string> pgms = mon.activePrograms();
    
    EQ(size_t(2), pgms.size());
    EQ(std::string("anotherTest"), pgms[0]);   // They come out alpha.
    EQ(std::string("test"), pgms[1]);
    
}
// Several programs filter out the disabled:

void changeMonitorTests::activeProgramsSomeInactiveDefault()
{
    makeProgram("/RunState", "anotherTest");
    makeProgram("/RunState", "zzztest");
    
    m_pApi->set("/RunState/test/enable", "false");
    
    CStateTransitionMonitor mon("tcp://localhost", "tcp://localhost");
    std::vector<std::string> pgms = mon.activePrograms();
    
    EQ(size_t(2), pgms.size());
    EQ(std::string("anotherTest"), pgms[0]);   // They come out alpha.
    EQ(std::string("zzztest"), pgms[1]);
}

// 1 active program but not in the default dir:

void changeMonitorTests::activePrograms1Changed()
{
    m_pApi->set("/RunState/ReadoutParentDir", "/programs");
    m_pApi->mkdir("/programs");
    
    makeProgram("/programs", "atest");
    
    CStateTransitionMonitor mon("tcp://localhost", "tcp://localhost");
    std::vector<std::string> pgms = mon.activePrograms();
    EQ(size_t(1), pgms.size());
    EQ(std::string("atest"), pgms[0]);
}

// 1 program, inactive in non default dir:

void changeMonitorTests::inactiveProgramsChanged()
{
    m_pApi->set("/RunState/ReadoutParentDir", "/programs");
    m_pApi->mkdir("/programs");
    
    makeProgram("/programs", "atest");
    m_pApi->set("/programs/atest/enable", "false");  // Disable.
    
    CStateTransitionMonitor mon("tcp://localhost", "tcp://localhost");
    std::vector<std::string> pgms = mon.activePrograms();
    EQ(size_t(0), pgms.size());
}

// Several active programs in non default dir:

void changeMonitorTests::activeProgramsSeveralChanged()
{
    m_pApi->set("/RunState/ReadoutParentDir", "/programs");
    m_pApi->mkdir("/programs");
    
    makeProgram("/programs", "atest");
    makeProgram("/programs", "btest");
    makeProgram("/programs", "ztest");
    
    CStateTransitionMonitor mon("tcp://localhost", "tcp://localhost");
    std::vector<std::string> pgms = mon.activePrograms();
    EQ(size_t(3), pgms.size());
    EQ(std::string("atest"), pgms[0]);
    EQ(std::string("btest"), pgms[1]);
    EQ(std::string("ztest"), pgms[2]);
}
// Now some of those are inactive:

void changeMonitorTests::activeProgramsSomeInactiveChanged()
{
    m_pApi->set("/RunState/ReadoutParentDir", "/programs");
    m_pApi->mkdir("/programs");
    
    makeProgram("/programs", "atest");
    makeProgram("/programs", "btest");
    makeProgram("/programs", "ztest");
    
    // Disbable btest:
    
    m_pApi->set("/programs/btest/enable", "false");
    
    CStateTransitionMonitor mon("tcp://localhost", "tcp://localhost");
    std::vector<std::string> pgms = mon.activePrograms();
    EQ(size_t(2), pgms.size());
    EQ(std::string("atest"), pgms[0]);
    EQ(std::string("ztest"), pgms[1]);
}

// Test isStandalone()

void changeMonitorTests::standaloneDefault()
{
    m_pApi->set("/RunState/test/standalone", "true");
    
    CStateTransitionMonitor mon("tcp://localhost", "tcp://localhost");
    ASSERT(mon.isStandalone("test"));
}

void changeMonitorTests::notStandaloneDefault()
{
    CStateTransitionMonitor mon("tcp://localhost", "tcp://localhost");
    ASSERT(!mon.isStandalone("test"));
    
}
void changeMonitorTests::saloneNoSuchDefault()
{
    CStateTransitionMonitor mon("tcp://localhost", "tcp://localhost");
    CPPUNIT_ASSERT_THROW(
        mon.isStandalone("junk"),
        std::runtime_error
    );
}
// Now the same tests when programs are not in /RunState

void changeMonitorTests::standaloneChanged()
{
    m_pApi->set("/RunState/ReadoutParentDir", "/programs");
    m_pApi->mkdir("/programs");
    makeProgram("/programs", "atest");
    
    m_pApi->set("/programs/atest/standalone", "true");
    
    CStateTransitionMonitor mon("tcp://localhost", "tcp://localhost");
    ASSERT(mon.isStandalone("atest"));
}

void changeMonitorTests::notStandaloneChanged()
{
    m_pApi->set("/RunState/ReadoutParentDir", "/programs");
    m_pApi->mkdir("/programs");
    makeProgram("/programs", "atest");
    
    CStateTransitionMonitor mon("tcp://localhost", "tcp://localhost");
    ASSERT(!mon.isStandalone("atest"));
}
void changeMonitorTests::saloneNoSuchChanged()
{
    m_pApi->set("/RunState/ReadoutParentDir", "/programs");
    m_pApi->mkdir("/programs");
    
    CStateTransitionMonitor mon("tcp://localhost", "tcp://localhost");
    CPPUNIT_ASSERT_THROW(
        mon.isStandalone("test"),
        std::runtime_error
    );
  
}


// Our programs get built enabled:

void changeMonitorTests::enabled()
{
    CStateTransitionMonitor mon("tcp://localhost", "tcp://localhost");
    ASSERT(mon.isEnabled("test"));
}
// But they could get disabled:

void changeMonitorTests::notEnabled()
{
    m_pApi->set("RunState/test/enable", "false");
    
    CStateTransitionMonitor mon("tcp://localhost", "tcp://localhost");
    ASSERT(!mon.isEnabled("test"));
}
//  They could wind up in a different directory:

void changeMonitorTests::enabledChanged()
{
    m_pApi->set("/RunState/ReadoutParentDir", "/programs");
    m_pApi->mkdir("/programs");
    makeProgram("/programs", "atest");
    
    CStateTransitionMonitor mon("tcp://localhost", "tcp://localhost");
    ASSERT(mon.isEnabled("atest"));
}
// And maybe disabled there too:

void changeMonitorTests::notEnabledChanged()
{
    m_pApi->set("/RunState/ReadoutParentDir", "/programs");
    m_pApi->mkdir("/programs");
    makeProgram("/programs", "atest");
    
    m_pApi->set("/programs/atest/enable", "false");
    
    CStateTransitionMonitor mon("tcp://localhost", "tcp://localhost");
    ASSERT(!mon.isEnabled("atest"));
}
// Unmodified we set the transition timeout to 60.

void changeMonitorTests::transitionTimeout()
{
     CStateTransitionMonitor mon("tcp://localhost", "tcp://localhost");
     EQ(60, mon.transitionTimeout());
}

void changeMonitorTests::modifiedTranstionTimeout()
{
    m_pApi->set("/RunState/Timeout", "30");

    CStateTransitionMonitor mon("tcp://localhost", "tcp://localhost");
    EQ(30, mon.transitionTimeout());    
}
void changeMonitorTests::setTimeout()
{
    CStateTransitionMonitor mon("tcp://localhost", "tcp://localhost");
    mon.setTransitionTimeout(30);
    EQ(30, mon.transitionTimeout());
}
// Notification tests

// Not standalone - no state change -> no notification (timeout).

void changeMonitorTests::noNotification()
{
    CStateTransitionMonitor mon("tcp://localhost", "tcp://localhost");
    std::vector<CStateTransitionMonitor::Notification> nots =
        mon.getNotifications(-1, 500);
    EQ(size_t(0), nots.size());
}

// One state change in the global state:

void changeMonitorTests::notification1()
{
    
    CStateTransitionMonitor mon("tcp://localhost", "tcp://localhost");
    m_pApi->set("/RunState/State", "Readying");
    usleep(500*1000);
    
    
    std::vector<CStateTransitionMonitor::Notification> nots =
        mon.getNotifications(-1, 500);
    EQ(size_t(1), nots.size());
    EQ(CStateTransitionMonitor::GlobalStateChange, nots[0].s_type);
    EQ(std::string("Readying"), nots[0].s_state);
    

    
}
// Notifications can be queued:

void changeMonitorTests::notificationMany()
{
    CStateTransitionMonitor mon("tcp://localhost", "tcp://localhost");
    m_pApi->set("/RunState/State", "Readying");
    m_pApi->set("/RunState/State", "Ready");
    
    usleep(500*1000);             // let this all trickle through;
    
    std::vector<CStateTransitionMonitor::Notification> nots =
        mon.getNotifications(-1, 500);
    EQ(size_t(2), nots.size());
    EQ(CStateTransitionMonitor::GlobalStateChange, nots[0].s_type);
    EQ(std::string("Readying"), nots[0].s_state);
    
    EQ(CStateTransitionMonitor::GlobalStateChange, nots[1].s_type);
    EQ(std::string("Ready"), nots[1].s_state);
}

// We can control the maximum number of notifications delivered:

void changeMonitorTests::notificationLimited()
{
    CStateTransitionMonitor mon("tcp://localhost", "tcp://localhost");
    m_pApi->set("/RunState/State", "Readying");
    m_pApi->set("/RunState/State", "Ready");
    
    usleep(500*1000);             // let this all trickle through;
    
    std::vector<CStateTransitionMonitor::Notification> nots =
        mon.getNotifications(1, 500);
    EQ(size_t(1), nots.size());
    EQ(CStateTransitionMonitor::GlobalStateChange, nots[0].s_type);
    EQ(std::string("Readying"), nots[0].s_state);
}
// Notification that  program, changed state:

void changeMonitorTests::prNotification1()
{
    CStateTransitionMonitor mon("tcp://localhost", "tcp://localhost");
    
    m_pApi->set("/RunState/test/State", "Readying");
    usleep(500*1000);             // let this all trickle through;
    std::vector<CStateTransitionMonitor::Notification> nots =
        mon.getNotifications(1, 500);
        
    EQ(size_t(1), nots.size());
    EQ(CStateTransitionMonitor::ProgramStateChange, nots[0].s_type);
    EQ(std::string("Readying"), nots[0].s_state);
    EQ(std::string("test"), nots[0].s_program);
    
}
// Several program change notifications can queue up... not limited
// to one program.  but they shouild come out time ordered:

void changeMonitorTests::prNotificationMany()
{
    makeProgram("RunState", "another");

    CStateTransitionMonitor mon("tcp://localhost", "tcp://localhost");
    
    m_pApi->set("/RunState/test/State", "Readying");
    m_pApi->set("/RunState/another/State", "Readying");
    m_pApi->set("/RunState/test/State", "Ready");
    
    usleep(500*1000);             // let this all trickle through;
    std::vector<CStateTransitionMonitor::Notification> nots =
        mon.getNotifications(-1, 500);
        
    EQ(size_t(3), nots.size());         // Should get three notifications.
    
    // First is test -> Readying:
    
    EQ(CStateTransitionMonitor::ProgramStateChange, nots[0].s_type);
    EQ(std::string("Readying"), nots[0].s_state);
    EQ(std::string("test"),      nots[0].s_program);
    
    // Then another -> Readying
    
    EQ(CStateTransitionMonitor::ProgramStateChange, nots[1].s_type);
    EQ(std::string("Readying"), nots[1].s_state);
    EQ(std::string("another"),      nots[1].s_program);
    
    // Lastly test -> Ready.
    
    EQ(CStateTransitionMonitor::ProgramStateChange, nots[2].s_type);
    EQ(std::string("Ready"),    nots[2].s_state);
    EQ(std::string("test"),      nots[2].s_program);
    
}

// program notifications participate in message limits:

void changeMonitorTests::prNotificationLimited()
{
    makeProgram("RunState", "another");

    CStateTransitionMonitor mon("tcp://localhost", "tcp://localhost");
    
    m_pApi->set("/RunState/test/State", "Readying");
    m_pApi->set("/RunState/another/State", "Readying");
    m_pApi->set("/RunState/test/State", "Ready");
    
    usleep(500*1000);             // let this all trickle through;
    std::vector<CStateTransitionMonitor::Notification> nots =
        mon.getNotifications(2, 500);
        
    EQ(size_t(2), nots.size());         // Should get three notifications.
    
    // First is test -> Readying:
    
    EQ(CStateTransitionMonitor::ProgramStateChange, nots[0].s_type);
    EQ(std::string("Readying"), nots[0].s_state);
    EQ(std::string("test"),      nots[0].s_program);
    
    // Then another -> Readying
    
    EQ(CStateTransitionMonitor::ProgramStateChange, nots[1].s_type);
    EQ(std::string("Readying"), nots[1].s_state);
    EQ(std::string("another"),      nots[1].s_program);
    
}
// Global and program state notifications can both appear:

void changeMonitorTests::mixedNotifications()
{
    CStateTransitionMonitor mon("tcp://localhost", "tcp://localhost");
    
    m_pApi->set("/RunState/test/State", "Readying");
    m_pApi->set("/RunState/State", "Readying");
    m_pApi->set("/RunState/test/State", "Ready");
    
    usleep(500*1000);             // let this all trickle through;
    std::vector<CStateTransitionMonitor::Notification> nots =
        mon.getNotifications(-1, 500);
        
    EQ(size_t(3), nots.size());         // Should get three notifications.
    
    // First is test -> Readying:
    
    EQ(CStateTransitionMonitor::ProgramStateChange, nots[0].s_type);
    EQ(std::string("Readying"), nots[0].s_state);
    EQ(std::string("test"),      nots[0].s_program);
    
    // Then global state -> Readying
    
    EQ(CStateTransitionMonitor::GlobalStateChange, nots[1].s_type);
    EQ(std::string("Readying"), nots[1].s_state);
    
    
    // Lastly test -> Ready.
    
    EQ(CStateTransitionMonitor::ProgramStateChange, nots[2].s_type);
    EQ(std::string("Ready"),    nots[2].s_state);
    EQ(std::string("test"),      nots[2].s_program);    
}
// Program joins the system.

void changeMonitorTests::programJoins()
{
    CStateTransitionMonitor mon("tcp://localhost", "tcp://localhost");
    
    makeProgram("RunState", "new");
    
    usleep(500*1000);
    
    std::vector<CStateTransitionMonitor::Notification> nots =
        mon.getNotifications(-1, 500);
        
    EQ(size_t(1), nots.size());         // Should get three notifications.
    
    
    EQ(CStateTransitionMonitor::ProgramJoins, nots[0].s_type);
    EQ(std::string("new"), nots[0].s_program);
}
// Several programs join the system:

void changeMonitorTests::programMultiJoins()
{
    CStateTransitionMonitor mon("tcp://localhost", "tcp://localhost");
    
    makeProgram("RunState", "new");
    makeProgram("RunState", "newer");
    
    usleep(500*1000);
    
    std::vector<CStateTransitionMonitor::Notification> nots =
        mon.getNotifications(-1, 500);
        
    EQ(size_t(2), nots.size());         // Should get three notifications.
    
    
    EQ(CStateTransitionMonitor::ProgramJoins, nots[0].s_type);
    EQ(std::string("new"), nots[0].s_program);

    EQ(CStateTransitionMonitor::ProgramJoins, nots[1].s_type);
    EQ(std::string("newer"), nots[1].s_program);

}

// Program leaves the system:

void changeMonitorTests::programLeaves()
{
    makeProgram("RunState", "new");
    makeProgram("RunState", "newer");
    
    CStateTransitionMonitor mon("tcp://localhost", "tcp://localhost");
    removeProgram("/RunState", "new");
    usleep(500*1000);
    
    std::vector<CStateTransitionMonitor::Notification> nots =
        mon.getNotifications(-1, 500);
        
    EQ(size_t(1), nots.size());         // Should get three notifications.
    EQ(CStateTransitionMonitor::ProgramLeaves, nots[0].s_type);
    EQ(std::string("new"), nots[0].s_program);
    
}
// several programs leave:

void changeMonitorTests::programMultiLeaves()
{
    makeProgram("RunState", "new");
    makeProgram("RunState", "newer");
    
    CStateTransitionMonitor mon("tcp://localhost", "tcp://localhost");
    removeProgram("/RunState", "new");
    removeProgram("/RunState", "newer");
    usleep(500*1000);
    
    std::vector<CStateTransitionMonitor::Notification> nots =
        mon.getNotifications(-1, 500);
        
    EQ(size_t(2), nots.size());         // Should get three notifications.
    EQ(CStateTransitionMonitor::ProgramLeaves, nots[0].s_type);
    EQ(std::string("new"), nots[0].s_program);
    

    EQ(CStateTransitionMonitor::ProgramLeaves, nots[1].s_type);
    EQ(std::string("newer"), nots[1].s_program);
}
// programs join and leave:

void changeMonitorTests::programJoinLeaves()
{
     makeProgram("RunState", "new");
     CStateTransitionMonitor mon("tcp://localhost", "tcp://localhost");
     removeProgram("/RunState", "new");
     makeProgram("RunState", "newer");
     
     usleep(500*100);
     
     std::vector<CStateTransitionMonitor::Notification> nots =
        mon.getNotifications(-1, 500);
        
    EQ(size_t(2), nots.size());         // Should get three notifications.
    EQ(CStateTransitionMonitor::ProgramLeaves, nots[0].s_type);
    EQ(std::string("new"), nots[0].s_program);
    
    EQ(CStateTransitionMonitor::ProgramJoins, nots[1].s_type);
    EQ(std::string("newer"), nots[1].s_program);

}
// Variable changes:


void changeMonitorTests::varChanges()
{
    CStateTransitionMonitor mon("tcp://localhost", "tcp://localhost");
    m_pApi->set("/RunState/RunNumber", "1234");
    usleep(500*100);
      
    std::vector<CStateTransitionMonitor::Notification> nots =
        mon.getNotifications(-1, 500);
    EQ(size_t(1), nots.size());
    EQ(CStateTransitionMonitor::VarChanged, nots[0].s_type);
    EQ(std::string("/RunState/RunNumber"), nots[0].s_state);
    EQ(std::string("1234"), nots[0].s_program);
}
