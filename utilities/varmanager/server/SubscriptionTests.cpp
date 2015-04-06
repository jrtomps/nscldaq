// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include <zmq.hpp>

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

#include "CVarMgrSubscriptions.h"



static const std::string requestServiceName("vardb-request");
static const std::string subServiceName("vardb-changes");
static const std::string serverName("vardbServer");

class SubTests : public CppUnit::TestFixture {
    // Construction
    
  CPPUNIT_TEST_SUITE(SubTests);
  CPPUNIT_TEST(constructPortOk);
  CPPUNIT_TEST(constructPortBadHost);
  CPPUNIT_TEST(constructSvcOk);
  CPPUNIT_TEST(constructSvcBadSvc);
  CPPUNIT_TEST(constructSvcBadHost);
  
    // Selectors:

  CPPUNIT_TEST(getSocket);
  CPPUNIT_TEST(getFd);
  
    // New subscriptions:
    
  CPPUNIT_TEST(subOk);
  CPPUNIT_TEST(subDupFail);

    // remove subscriptions.
    
  CPPUNIT_TEST(unsubOk);
  CPPUNIT_TEST(unsubNoSuch);
  CPPUNIT_TEST(unsubRemoves);

    // waitmsg.
    
  CPPUNIT_TEST(waitNoMsg);
  CPPUNIT_TEST(waitMsg);

  CPPUNIT_TEST(readable);
  CPPUNIT_TEST(notreadable);

  CPPUNIT_TEST(read);
  
  CPPUNIT_TEST(notifier);
  CPPUNIT_TEST_SUITE_END();
protected:
  void constructPortOk();
  void constructPortBadHost();
  void constructSvcOk();
  void constructSvcBadSvc();
  void constructSvcBadHost();
  
  void getSocket();
  void getFd();
  
  void subOk();
  void subDupFail();
  
  void unsubOk();
  void unsubNoSuch();
  void unsubRemoves();
  
  void waitNoMsg();
  void waitMsg();
  
  void readable();
  void notreadable();
  
  void read();
  
  void notifier();
private:
    pid_t m_serverPid;
    int m_serverRequestPort;
    int m_serverSubPort;
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
    m_serverRequestPort = getRequestPort(requestServiceName);
    m_serverSubPort     = getRequestPort(subServiceName);
    
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
    int getRequestPort(std::string serviceName);
    void stopServer();
    
};

// Private utilities:



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
SubTests::startServer()
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
 *  @param serviceName - name of the service to translate
 *  @return int - port number bound to the server's varmgr-request service.
 *  @note For testing the server is always in localhost.
 */
int
SubTests::getRequestPort(std::string serviceName)
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
void SubTests::stopServer()
{
    int exitStatus;
    kill(m_serverPid, SIGKILL);
    waitpid(m_serverPid, &exitStatus, 0);
    
}

CPPUNIT_TEST_SUITE_REGISTRATION(SubTests);




void SubTests::constructPortOk() {
    CPPUNIT_ASSERT_NO_THROW(
        CVarMgrSubscriptions("localhost", m_serverSubPort)
    );
}

void SubTests::constructPortBadHost()
{
    CPPUNIT_ASSERT_THROW(
        CVarMgrSubscriptions("no.such.host", m_serverSubPort),
        CVarMgrSubscriptions::CException
    );
}
void SubTests::constructSvcOk()
{
    CPPUNIT_ASSERT_NO_THROW(
        CVarMgrSubscriptions("localhost", subServiceName.c_str())
    );
}

void SubTests::constructSvcBadSvc()
{
    CPPUNIT_ASSERT_THROW(
        CVarMgrSubscriptions("localhost", "no-such-service"),
        CVarMgrSubscriptions::CException
    );
}

void SubTests::constructSvcBadHost()
{
    CPPUNIT_ASSERT_THROW(
        CVarMgrSubscriptions("no.such.host", subServiceName.c_str()),
        CVarMgrSubscriptions::CException
    );
}

void SubTests::getSocket()
{
    CVarMgrSubscriptions sub("localhost", m_serverSubPort);
    zmq::socket_t* pSocket = sub.socket();
    
    int type;
    size_t size(sizeof(type));
    
    pSocket->getsockopt(ZMQ_TYPE, &type, &size);
    
    EQ(ZMQ_SUB, type);
    
}

void SubTests::getFd()
{
    CVarMgrSubscriptions sub("localhost", m_serverSubPort);
    zmq::socket_t* pSocket = sub.socket();
    int fd    = sub.fd();
    
    int fdopt;
    size_t size(sizeof(fdopt));
    
    pSocket->getsockopt(ZMQ_FD, &fdopt, &size);
    
    EQ(fdopt, fd);
}

// Subsribing to a tree should give a message when that tree
// is created and should not give a message for stuff on
// other trees:

void SubTests::subOk()
{
    CVarMgrSubscriptions sub("localhost", m_serverSubPort);
    sub.subscribe("/test1");
    zmq::socket_t* pSocket = sub.socket();
    
    // Make a pollitem for us:
    
    zmq_pollitem_t item = { (void*)(*pSocket), -1, ZMQ_POLLIN, 0   };
    
    // This operation, creating /junk should not cause a message:
    
    m_pApi->mkdir("/nomessage");
    zmq_poll(&item, 1, 10000l);     // 10ms.
    EQ((short int)0, item.revents);
    
    // This operation, creating /test1, should cause a message:
    
    m_pApi->mkdir("/test1/test");
    zmq_poll(&item, 1, 1000*1000);    // At most one second.
    EQ((short int)ZMQ_POLLIN, item.revents);
    
    // Should be able to receive the message:  /test1:MKDIR:test
    
    zmq::message_t msg(1000);
    pSocket->recv(&msg, 0);
    
    void* data = msg.data();
    size_t n   = msg.size();
    
    char szData[n+1];
    memset(szData, 0, n+1);
    memcpy(szData, data, n);
    
    std::string message(szData);
    EQ(std::string("/test1:MKDIR:test"), message);
}
// Duplicate subscription fails:

void SubTests::subDupFail()
{
    CVarMgrSubscriptions sub("localhost", m_serverSubPort);
    sub.subscribe("/test1");
    
    CPPUNIT_ASSERT_THROW(
        sub.subscribe("/test1"),
        CVarMgrSubscriptions::CException
    );
}

// removal of existing subscription is ok and I stop getting stuff.

void SubTests::unsubOk()
{
    CVarMgrSubscriptions sub("localhost", m_serverSubPort);
    zmq::socket_t* pSocket = sub.socket();
    m_pApi->mkdir("/test1/subdir");
    sub.subscribe("/test1");           // Notify on /test1.
    
    sub.unsubscribe("/test1");         // no longer notify.
    m_pApi->mkdir("/test1/another");
    
    zmq_pollitem_t item = { (void*)(*pSocket), -1, ZMQ_POLLIN, 0   };
    zmq_poll(&item, 1, 10000l);
    EQ((short int)0, item.revents);
    zmq::message_t msg(100);
    
    pSocket->recv(&msg, ZMQ_NOBLOCK);  // Flushes msg if there was one.
}

void SubTests::unsubNoSuch()
{
    CVarMgrSubscriptions sub("localhost", m_serverSubPort);
    CPPUNIT_ASSERT_THROW(
        sub.unsubscribe("/test1"),
        CVarMgrSubscriptions::CException
    );
}

void SubTests::unsubRemoves()
{
    CVarMgrSubscriptions sub("localhost", m_serverSubPort);
    sub.subscribe("/test");
    CPPUNIT_ASSERT_NO_THROW(
        sub.unsubscribe("/test")
    );
    CPPUNIT_ASSERT_THROW(
        sub.unsubscribe("/test"),
        CVarMgrSubscriptions::CException
    );
}


// When no message can come wait times out.

void SubTests::waitNoMsg()
{
    CVarMgrSubscriptions sub("localhost", m_serverSubPort);
    ASSERT(!sub.waitmsg(100));
}

// When there is a message we can be informed of it:

void SubTests::waitMsg()
{
    CVarMgrSubscriptions sub("localhost", m_serverSubPort);
    sub.subscribe("/test");
    
    m_pApi->mkdir("/test/testing");
    
    ASSERT(sub.waitmsg(100));
    zmq::socket_t* pSocket = sub.socket();
    zmq::message_t msg(100);
    ASSERT(pSocket->recv(&msg, ZMQ_NOBLOCK));   // Should not need to wait.
}

void SubTests::readable()
{

    CVarMgrSubscriptions sub("localhost", m_serverSubPort);
    sub.subscribe("/test");
    
    m_pApi->mkdir("/test/testing");
    
    ASSERT(sub.waitmsg(100));
    ASSERT(sub.readable());
    zmq::socket_t* pSocket = sub.socket();
    zmq::message_t msg(100);
    ASSERT(pSocket->recv(&msg, ZMQ_NOBLOCK));   // Should not need to wait.    
}

void SubTests::notreadable()
{
    CVarMgrSubscriptions sub("localhost", m_serverSubPort);
    ASSERT(!sub.readable());
}

void SubTests::read()
{
    CVarMgrSubscriptions sub("localhost", m_serverSubPort);
    sub.subscribe("/test");
    m_pApi->mkdir("/test/testing");
    
    CVarMgrSubscriptions::Message msg = sub.read();
    
    EQ(std::string("/test"), msg.s_path);
    EQ(std::string("MKDIR"), msg.s_operation);
    EQ(std::string("testing"), msg.s_data);
}


class NotTest : public CVarMgrSubscriptions {
public:
    CVarMgrSubscriptions::Message m_message;
    
    NotTest(const char* phost, const char* pService) : CVarMgrSubscriptions(phost, pService) {}
    NotTest(const char* phost, int port) : CVarMgrSubscriptions(phost, port) {}
    virtual ~NotTest() {}
    void notify(CVarMgrSubscriptions::pMessage pm) {
        m_message = *pm;
    }
};

void SubTests::notifier()
{
    NotTest sub("localhost", m_serverSubPort);
    sub.subscribe("/test");
    m_pApi->mkdir("/test/testing");
    
    sub();
    EQ(std::string("/test"), sub.m_message.s_path);
    EQ(std::string("MKDIR"), sub.m_message.s_operation);
    EQ(std::string("testing"), sub.m_message.s_data);
    
}