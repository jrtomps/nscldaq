// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"


#include "CVarMgrServerApi.h"
#include <CVariableDb.h>
#include <CPortManager.h>
#include <os.h>
#include "CStateTransitionMonitor.h"


static const std::string serviceName("vardb-request");
static const std::string serverName("vardbServer");

class changeMonitorTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(changeMonitorTests);
  CPPUNIT_TEST(defaultParent);
  CPPUNIT_TEST(changedParent);
  
  CPPUNIT_TEST(allPrograms1Default);
  CPPUNIT_TEST(allProgramsSeveralDefault);
  //CPPUNIT_TEST(allPrograms1Changed);
  //CPPUNIT_TEST(allProgramsSeveralChanged);
  CPPUNIT_TEST_SUITE_END();

protected:
    void defaultParent();
    void changedParent();

    void allPrograms1Default();
    void allProgramsSeveralDefault();
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
};


// Utility methods:

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