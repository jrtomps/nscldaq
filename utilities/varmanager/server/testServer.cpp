// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
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

#include <CVariableDb.h>
#include <CVarDirTree.h>
#include <CVariable.h>

#include "CVarMgrServerApi.h"

static const std::string serviceName("vardb-request");
static const std::string serverName("vardbServer");

// test the server api.  At this time it's assumed the server is robust...
// at least that it will start correctly and respond correctly.

class ServerApiTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(ServerApiTests);
  CPPUNIT_TEST(initialGetwd);
  
  // mkdir tests
  
  CPPUNIT_TEST(mkdirok);
  CPPUNIT_TEST(mkdirbad);
  
  CPPUNIT_TEST(rmdirok);
  CPPUNIT_TEST(rmdirbad);
  CPPUNIT_TEST(rmdirnotempty);
  
  CPPUNIT_TEST(declareNoInitial);
  CPPUNIT_TEST(declareInitial);
  CPPUNIT_TEST(declareInvalidPath);
  CPPUNIT_TEST(declareInvalidType);
  //CPPUNIT_TEST(declareDuplicatePath);
  //CPPUNIT_TEST(declareBadInitialValue);
  
  CPPUNIT_TEST_SUITE_END();

protected:
  void initialGetwd();
  void mkdirok();
  void mkdirbad();
  void rmdirok();
  void rmdirbad();
  void rmdirnotempty();
  void declareNoInitial();
  void declareInitial();
  void declareInvalidPath();
  void declareInvalidType();

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
    
};

// Utility methods:

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
ServerApiTests::startServer()
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
ServerApiTests::getRequestPort()
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
void ServerApiTests::stopServer()
{
    int exitStatus;
    kill(m_serverPid, SIGKILL);
    waitpid(m_serverPid, &exitStatus, 0);
    
}

CPPUNIT_TEST_SUITE_REGISTRATION(ServerApiTests);

// After creation the api should have the wd as "/"

void ServerApiTests::initialGetwd() {
    EQ(std::string("/"), m_pApi->getwd());
}

// After creating a directory it should be there:

void ServerApiTests::mkdirok()
{
    m_pApi->mkdir("/testing");
    
    CVariableDb db(m_tempFile);
    CVarDirTree dirs(db);
    
    std::vector<CVarDirTree::DirInfo> dirList = dirs.ls();
    
    EQ(size_t(1), dirList.size());
    EQ(std::string("testing"), dirList[0].s_name);
    
    
}
// Creating a directory already exists.
// results in  std::exception.

void ServerApiTests::mkdirbad()
{
    m_pApi->mkdir("/testing");
    CPPUNIT_ASSERT_THROW(
        m_pApi->mkdir("/testing"),
        CVarMgrApi::CException
    );
}

// rmdir an existing empty dir is ok:

void
ServerApiTests::rmdirok()
{
    m_pApi->mkdir("/testing");
    m_pApi->rmdir("/testing");
    
    CVariableDb db(m_tempFile);
    CVarDirTree dirs(db);
    
    std::vector<CVarDirTree::DirInfo> dirList = dirs.ls();
    
    EQ(size_t(0), dirList.size());
}
// Rmdir a directory that does not exist is bad:

void
ServerApiTests::rmdirbad()
{
    CPPUNIT_ASSERT_THROW(
        m_pApi->rmdir("/testing"),
        CVarMgrApi::CException
    );
}
// rmdir a non-empty dir fails too:

void
ServerApiTests::rmdirnotempty()
{
    m_pApi->mkdir("/testing/one");
    CPPUNIT_ASSERT_THROW(
        m_pApi->rmdir("/testing"),
        CVarMgrApi::CException
    );
}
// declare create a variable with no initial value.

void ServerApiTests::declareNoInitial()
{
    m_pApi->declare("/myvar", "integer");
    
    CVariableDb db(m_tempFile);
    CVarDirTree dir(db);
    std::vector<CVariable::VarInfo> info = CVariable::list(&db, dir, "/");
    EQ(size_t(1), info.size());
    EQ(std::string("integer"), info[0].s_type);
    EQ(std::string("myvar"), info[0].s_name);
    CVariable var(db, "/myvar");
    
    EQ(std::string("0"), var.get());
    
}
// Declare with an initial value:

void ServerApiTests::declareInitial()
{
    m_pApi->declare("/myvar", "integer", "1234");
    
    CVariableDb db(m_tempFile);
    CVarDirTree dir(db);
    std::vector<CVariable::VarInfo> info = CVariable::list(&db, dir, "/");
    EQ(size_t(1), info.size());
    EQ(std::string("integer"), info[0].s_type);
    EQ(std::string("myvar"), info[0].s_name);
    CVariable var(db, "/myvar");
    
    EQ(std::string("1234"), var.get());
}

// Declare a var into a non-exstent directory:

void ServerApiTests::declareInvalidPath()
{
    CPPUNIT_ASSERT_THROW(
        m_pApi->declare("/no/such/path/myvar", "integer"),
        CVarMgrApi::CException
    );
}
// Declare a variable with a nonexistent data type:

void ServerApiTests::declareInvalidType()
{
    CPPUNIT_ASSERT_THROW(
        m_pApi->declare("/myvar", "rock-star"),
        CVarMgrApi::CException
    );
}