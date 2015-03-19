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
#include <limits.h>

#include <CVariableDb.h>
#include <CVarDirTree.h>
#include <CVariable.h>
#include <CSqliteException.h>

#include "CVarMgrApiFactory.h"

#include "CVarMgrFileApi.h"
#include "CVarMgrServerApi.h"
#include <typeinfo>


static const std::string serviceName("vardb-request");
static const std::string serverName("vardbServer");


class FactoryTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(FactoryTests);
  CPPUNIT_TEST(createFileApi);
  CPPUNIT_TEST(createFileApiBadPath);
  
  CPPUNIT_TEST(createServerByPortnumOK);
  CPPUNIT_TEST(createServerByPortnumBadHost);
#ifdef CONNECT_PING_IMPLEMENTED  
  CPPUNIT_TEST(createServerByPortnumBadPort);
#endif

  CPPUNIT_TEST(createServerByServiceOk);
  CPPUNIT_TEST(createServerByServiceBadSvc);
  
  CPPUNIT_TEST(createServerDefaultSvc);
  
  CPPUNIT_TEST(createURIFile);
  CPPUNIT_TEST(createURIServerPort);
  CPPUNIT_TEST(createURIServerServiceName);
  CPPUNIT_TEST(createURIServerServiceAndPort);
  CPPUNIT_TEST(createURIServerDefaultService);
  CPPUNIT_TEST_SUITE_END();

protected:
  void createFileApi();
  void createFileApiBadPath();

  void createServerByPortnumOK();
  void createServerByPortnumBadHost();
  void createServerByPortnumBadPort();
  
  void createServerByServiceOk();
  void createServerByServiceBadSvc();
  void createServerDefaultSvc();
  
  void createURIFile();
  void createURIServerPort();
  void createURIServerServiceName();
  void createURIServerServiceAndPort();
  void createURIServerDefaultService();
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
FactoryTests::startServer()
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
FactoryTests::getRequestPort()
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
void FactoryTests::stopServer()
{
    int exitStatus;
    kill(m_serverPid, SIGKILL);
    waitpid(m_serverPid, &exitStatus, 0);
    
}

CPPUNIT_TEST_SUITE_REGISTRATION(FactoryTests);

/**
 * Create a file api given a path (m_tempFile).
 */
void FactoryTests::createFileApi() {
    CVarMgrApi* pApi = CVarMgrApiFactory::createFileApi(m_tempFile);
    ASSERT(pApi);
    ASSERT(dynamic_cast<CVarMgrFileApi*>(pApi));
    
    delete pApi;
}
/**
 * create a file api given a bad path.  This should throw.
 */
void FactoryTests::createFileApiBadPath()
{
    CPPUNIT_ASSERT_THROW(
        CVarMgrApiFactory::createFileApi("/no/such/file/for/vardb.db"),
        CSqliteException
    );
}
// Create a server api with everything peachy:

void FactoryTests::createServerByPortnumOK()
{
    CVarMgrApi* pApi = CVarMgrApiFactory::createServerApi("localhost", m_serverRequestPort);
    ASSERT(pApi);
    ASSERT(dynamic_cast<CVarMgrServerApi*>(pApi));
    
    delete pApi;
}
// Create server API with bad host:

void FactoryTests::createServerByPortnumBadHost()
{
    CPPUNIT_ASSERT_THROW(
        CVarMgrApiFactory::createServerApi("junk.nscl.msu.edu", m_serverRequestPort),
        zmq::error_t
    );
}
// create server api with bad port
void FactoryTests::createServerByPortnumBadPort()
{
    CPPUNIT_ASSERT_THROW(
        CVarMgrApiFactory::createServerApi("localhost", 99999),    // Probably refused.
        zmq::error_t
    );
}

// Create a server api using the service name

void FactoryTests::createServerByServiceOk()
{
    CVarMgrApi* pApi = CVarMgrApiFactory::createServerApi("localhost", serviceName);
    ASSERT(pApi);
    ASSERT(dynamic_cast<CVarMgrServerApi*>(pApi));
    
    delete pApi;
}
// Create server API with no such service throws

void FactoryTests::createServerByServiceBadSvc()
{
    CPPUNIT_ASSERT_THROW(
        CVarMgrApiFactory::createServerApi("localhost", "no such service"),
        CVarMgrApi::CException
    );
}
void FactoryTests::createServerDefaultSvc()
{
    CVarMgrApi* pApi = CVarMgrApiFactory::createServerApi("localhost");
    ASSERT(pApi);
    ASSERT(dynamic_cast<CVarMgrServerApi*>(pApi));
    
    delete pApi;
}

// Create URI whith a valid file:/// protocol.

void FactoryTests::createURIFile()
{
    std::string uri = "file://";
    char wd[PATH_MAX+1];
    uri += getwd(wd);
    uri += "/";
    uri += m_tempFile;
    
    CVarMgrApi* pApi = CVarMgrApiFactory::create(uri);
    ASSERT(pApi);
    ASSERT(dynamic_cast<CVarMgrFileApi*>(pApi));
    
}

// Server with numeric port specification (tcp://localhost:1234 e.g.)

void FactoryTests::createURIServerPort()
{
    std::string uri="tcp://localhost:";
    char portNum[20];
    sprintf(portNum, "%d", m_serverRequestPort);
    uri += portNum;
    
    CVarMgrApi* pApi = CVarMgrApiFactory::create(uri);
    ASSERT(pApi);
    ASSERT(dynamic_cast<CVarMgrServerApi*>(pApi));
}
// Server with named service:

void FactoryTests::createURIServerServiceName()
{
    std::string uri ="tcp://localhost/";
    uri += serviceName;
    
    CVarMgrApi* pApi = CVarMgrApiFactory::create(uri);
    ASSERT(pApi);
    ASSERT(dynamic_cast<CVarMgrServerApi*>(pApi));
}
// not allowed to have both server and port:

void FactoryTests::createURIServerServiceAndPort()
{
    std::string uri="tcp://localhost:";
    char portNum[20];
    sprintf(portNum, "%d", m_serverRequestPort);
    uri += portNum;
    uri += "/";
    uri += serviceName;
    
    CPPUNIT_ASSERT_THROW(
        CVarMgrApiFactory::create(uri),
        CVarMgrApi::CException
    );
}
// Empty port and path -> default

void FactoryTests::createURIServerDefaultService()
{
    std::string uri="tcp://localhost";
    
    CVarMgrApi* pApi = CVarMgrApiFactory::create(uri);
    ASSERT(pApi);
    ASSERT(dynamic_cast<CVarMgrServerApi*>(pApi));
}