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
  CPPUNIT_TEST(declareDuplicatePath);
  CPPUNIT_TEST(declareBadInitialValue);
  
  CPPUNIT_TEST(setOk);
  CPPUNIT_TEST(setNoVar);
  CPPUNIT_TEST(setBadValue);
  
  CPPUNIT_TEST(getOk);
  CPPUNIT_TEST(getNoVar);
  
  CPPUNIT_TEST(defEnumOk);
  CPPUNIT_TEST(defEnumDupType);
  CPPUNIT_TEST(defEnumDupValue);
  
  CPPUNIT_TEST(defSmOk);
  CPPUNIT_TEST(defSmUnreachable);
  CPPUNIT_TEST(defSmBadTransition);
  
  // Tests for cd; Note that wherever a path is used
  // we also need to test that relative paths work ok.
  
  CPPUNIT_TEST(cdOk);
  CPPUNIT_TEST(cdNoSuchPath);    // Needs server mods.
  CPPUNIT_TEST(cdRelative);
  CPPUNIT_TEST(relativeMkdir);
  CPPUNIT_TEST(relativeRmdir);
  CPPUNIT_TEST(relativeDeclare);
  CPPUNIT_TEST(relativeSet);
  CPPUNIT_TEST(relativeGet);
  
  CPPUNIT_TEST(lsEmpty);
  CPPUNIT_TEST(lsList);
  CPPUNIT_TEST(lsAbsPath);
  CPPUNIT_TEST(lsRelPath);

  CPPUNIT_TEST(lsvarEmpty);
  CPPUNIT_TEST(lsvarOneVar);
  CPPUNIT_TEST(lsvarVars);
  CPPUNIT_TEST(lsvarRelpath);
  CPPUNIT_TEST(lsvarAbsPath);
  CPPUNIT_TEST(lsvarEmptyString);

  CPPUNIT_TEST(rmvarOk);
  CPPUNIT_TEST(rmvarNoSuch);
  
  
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
  void declareDuplicatePath();
  void declareBadInitialValue();
  void setOk();
  void setNoVar();
  void setBadValue();
  void getOk();
  void getNoVar();
  void defEnumOk();
  void defEnumDupType();
  void defEnumDupValue();
  void defSmOk();
  void defSmUnreachable();
  void defSmBadTransition();
  void cdOk();
  void cdRelative();
  void cdNoSuchPath();
  void relativeMkdir();
  void relativeRmdir();
  void relativeDeclare();
  void relativeSet();
  void relativeGet();
  void lsEmpty();
  void lsList();
  void lsAbsPath();
  void lsRelPath();
  void lsvarEmpty();
  void lsvarOneVar();
  void lsvarVars();
  void lsvarRelpath();
  void lsvarAbsPath();
  void lsvarEmptyString();
  void rmvarOk();
  void rmvarNoSuch();
  
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
// Duplicate variable decl:

void ServerApiTests::declareDuplicatePath()
{
    m_pApi->declare("/myvar", "integer");
    CPPUNIT_ASSERT_THROW(
        m_pApi->declare("/myvar", "real"),
        CVarMgrApi::CException
    );
}
// Bad initial value also throws:

void ServerApiTests::declareBadInitialValue()
{
    CPPUNIT_ASSERT_THROW(
        m_pApi->declare("/myvar", "integer", "hello-world"),
        CVarMgrApi::CException
    );
}

// Can set an existing variable:

void ServerApiTests::setOk()
{
    m_pApi->declare("/myvar", "integer");
    m_pApi->set("/myvar", "1234");
    
    CVariableDb db(m_tempFile);
    CVariable   var(db, "/myvar");
    
    EQ(std::string("1234"), var.get());
}
// Cannot set a nonexisting variable:

void ServerApiTests::setNoVar()
{
    CPPUNIT_ASSERT_THROW(
        m_pApi->set("/myvar", "1234"),
        CVarMgrApi::CException
    );
}
// Cannot give a variable an illegal value:

void ServerApiTests::setBadValue()
{
    m_pApi->declare("/myvar", "integer");
    CPPUNIT_ASSERT_THROW(
        m_pApi->set("/myvar", "3.14159265359"),
        CVarMgrApi::CException
    );
}
// Get value of a variable:

void ServerApiTests::getOk()
{
    m_pApi->declare("/myvar", "integer", "12345");
    EQ(std::string("12345"), m_pApi->get("/myvar"));
}
// Fails when getting a value of an undefined variable:

void ServerApiTests::getNoVar()
{
    CPPUNIT_ASSERT_THROW(
        m_pApi->get("/myvar"),
        CVarMgrApi::CException
    );
}
// Can define an enumerated type after which we can make a variable
// of that type:

void ServerApiTests::defEnumOk()
{
    CVarMgrApi::EnumValues v;
    v.push_back("red");
    v.push_back("green");
    v.push_back("blue");
    
    m_pApi->defineEnum("colors", v);
    
    CPPUNIT_ASSERT_NO_THROW(
        m_pApi->declare("/acolor", "colors")
    );
    EQ(std::string("red"), m_pApi->get("/acolor"));
}
// cannot define an enum with the same name as an existing enum/type:

void ServerApiTests::defEnumDupType()
{
    CVarMgrApi::EnumValues v;
    v.push_back("red");
    v.push_back("green");
    v.push_back("blue");

    CPPUNIT_ASSERT_THROW(
        
        m_pApi->defineEnum("integer", v),
        CVarMgrApi::CException
    );
}
// Cannot define an enum type with a duplicate value:

void ServerApiTests::defEnumDupValue()
{
CVarMgrApi::EnumValues v;
    v.push_back("red");
    v.push_back("green");
    v.push_back("red");            // Duplicate !!
    v.push_back("blue");
    
    CPPUNIT_ASSERT_THROW(
        
        m_pApi->defineEnum("integer", v),
        CVarMgrApi::CException
    );
}
// Successfully define a simple state machine.

void ServerApiTests::defSmOk()
{
    // Define the state transition map for the machine:
    
    CVarMgrApi::StateMap sm;
    m_pApi->addTransition(sm, "0ready", "running");
    m_pApi->addTransition(sm, "running", "0ready");   // flip flop state machine.
    
    m_pApi->defineStateMachine("simple", sm);
    
    CPPUNIT_ASSERT_NO_THROW(
        m_pApi->declare("/aStateMachine", "simple")
    );
    EQ(std::string("0ready"), m_pApi->get("/aStateMachine"));
}
// Define with an unreachable state should fail:

void ServerApiTests::defSmUnreachable()
{
    CVarMgrApi::StateMap sm;
    m_pApi->addTransition(sm, "0ready", "running");
    m_pApi->addTransition(sm, "running", "0ready");   
    m_pApi->addTransition(sm, "unreachable", "runing");
    
    CPPUNIT_ASSERT_THROW(
        m_pApi->defineStateMachine("bad", sm),
        CVarMgrApi::CException
    );
}
// Define with a transition to a nonexistent state is bad:

void ServerApiTests::defSmBadTransition()
{
    CVarMgrApi::StateMap sm;
    m_pApi->addTransition(sm, "0ready", "running");
    m_pApi->addTransition(sm, "running", "0ready");
    m_pApi->addTransition(sm, "0ready", "does-not-exist");
    
    CPPUNIT_ASSERT_THROW(
        m_pApi->defineStateMachine("bad", sm),
        CVarMgrApi::CException
    );
}
// cd to an existing path is ok:

void ServerApiTests::cdOk()
{
    m_pApi->mkdir("/a/test/path");
    CPPUNIT_ASSERT_NO_THROW(
        m_pApi->cd("/a/test")
    );
    EQ(std::string("/a/test"), m_pApi->getwd());
}
// Given a cd other than / creating a directory with a relative path
// takes into account the cd.

void ServerApiTests::relativeMkdir()
{
    m_pApi->mkdir("/a/test/path");
    m_pApi->cd("/a/test");
    CPPUNIT_ASSERT_NO_THROW(
        m_pApi->mkdir("directory")   // makes /a/test/directory
    );
    
    CVariableDb db(m_tempFile);
    CVarDirTree tree(db);
    tree.cd("/a/test");
    
    std::vector<CVarDirTree::DirInfo> dirs = tree.ls();
    EQ(size_t(2), dirs.size());
    bool found = false;
    for (int i =0; i < dirs.size(); i++) {
        if (dirs[i].s_name == "directory") {
            found = true;
            break;
        }
    }
    ASSERT(found);
}
// Remove a directory relative to a wd:

void ServerApiTests::relativeRmdir()
{
    relativeMkdir();
    
    CPPUNIT_ASSERT_NO_THROW(
        m_pApi->rmdir("directory")
    );
    CVariableDb db(m_tempFile);
    CVarDirTree tree(db);
    tree.cd("/a/test");
    
    std::vector<CVarDirTree::DirInfo> dirs = tree.ls();
    EQ(size_t(1), dirs.size());
    EQ(std::string("path"), dirs[0].s_name);
}
// Declare a variable relative to a wd

void ServerApiTests::relativeDeclare()
{
    m_pApi->mkdir("/a/test/path");
    m_pApi->cd("/a/test");
    
    CPPUNIT_ASSERT_NO_THROW(
        m_pApi->declare("path/myvar", "integer")
    );
    
    CVariableDb db(m_tempFile);
    CVarDirTree tree(db);
    tree.cd("/a/test/path");
    
    CPPUNIT_ASSERT_NO_THROW(
        CVariable(db, tree, "myvar")
    );
}
// Can set a variable relative to the cwd

void ServerApiTests::relativeSet()
{
    relativeDeclare();        // /a/test/path/myvar is created.
    m_pApi->cd("/a/test");
    CPPUNIT_ASSERT_NO_THROW(
        m_pApi->set("path/myvar", "1234")
    );
    EQ(std::string("1234"), m_pApi->get("/a/test/path/myvar"));
}
// Can get the value of a variable relative to the cwd:

void ServerApiTests::relativeGet()
{
    relativeSet();            // /a/test/path/myvar = '1234'
    m_pApi->cd("/a");
    CPPUNIT_ASSERT_NO_THROW(
        EQ(std::string("1234"), m_pApi->get("test/path/myvar"));
    );
}
// cd with relative path canonicalizes:

void ServerApiTests::cdRelative()
{
    m_pApi->mkdir("/a/first/test/dir");
    m_pApi->mkdir("/a/second/test/dir");
    
    m_pApi->cd("/a/first/test/dir");
    m_pApi->cd("../../../second/test/dir");
    
    EQ(std::string("/a/second/test/dir"), m_pApi->getwd());
}
// cd to nonexistent path:

void ServerApiTests::cdNoSuchPath()
{
    CPPUNIT_ASSERT_THROW(
        m_pApi->cd("/nosuch"),
        CVarMgrApi::CException
    );
}

// List initially gives an empty vector>

void ServerApiTests::lsEmpty()
{
    CVarMgrApi& api(*m_pApi);
    EQ(size_t(0), api.ls().size());
}

// List nonempty directory:

void ServerApiTests::lsList()
{
    CVarMgrApi& api(*m_pApi);
    api.mkdir("/adir");
    api.mkdir("/anotherdir");
    api.mkdir("/lastdir");
    
    std::vector<std::string> dirs = api.ls();
    
    EQ(size_t(3), dirs.size());
    
    ASSERT(find(dirs.begin(), dirs.end(), "adir") != dirs.end());
    ASSERT(find(dirs.begin(), dirs.end(), "anotherdir") != dirs.end());
    ASSERT(find(dirs.begin(), dirs.end(), "lastdir") != dirs.end());
    
}
// Ls with path absolute:
void ServerApiTests::lsAbsPath()
{
    CVarMgrApi& api(*m_pApi);
    api.mkdir("/adir");
    api.mkdir("/anotherdir");
    api.mkdir("/lastdir");
    api.cd("/adir");    
    std::vector<std::string> dirs = api.ls("/");
    
    EQ(size_t(3), dirs.size());
    
    ASSERT(find(dirs.begin(), dirs.end(), "adir") != dirs.end());
    ASSERT(find(dirs.begin(), dirs.end(), "anotherdir") != dirs.end());
    ASSERT(find(dirs.begin(), dirs.end(), "lastdir") != dirs.end());

    EQ(size_t(0), api.ls("/adir").size());    
    
}
// Ls with relative path

void ServerApiTests::lsRelPath()
{
    CVarMgrApi& api(*m_pApi);
    api.mkdir("/adir");
    api.mkdir("/anotherdir");
    api.mkdir("/lastdir");
    api.cd("/adir");
    api.mkdir("/anotherdir/test");
    
    std::vector<std::string> dirs = api.ls("../anotherdir");
    EQ(size_t(1), dirs.size());
    EQ(std::string("test"), dirs[0]);
}

// ::lsvar with empty dir gives empty list:

void ServerApiTests::lsvarEmpty()
{
    CVarMgrApi& api(*m_pApi);
    EQ(size_t(0), api.lsvar().size());
}
// ::lsvar with a single variable in the wd gives the stuff for that var:

class FindVar {
    std::string m_name;
public:
    FindVar(std::string name) : m_name(name) {}
    bool operator()(CVarMgrApi::VarInfo item) {
        return item.s_name == m_name;
    }
};

static void varcheck(
    std::vector<CVarMgrApi::VarInfo> info, std::string name, std::string type,
    std::string value
)
{
    std::vector<CVarMgrApi::VarInfo>::iterator p;
    FindVar matcher(name);
    p = find_if(info.begin(), info.end(), matcher);
    ASSERT(p != info.end());
    
    EQ(name, p->s_name);
    EQ(type, p->s_typeName);
    EQ(value, p->s_value);
}

void ServerApiTests::lsvarOneVar()
{
    CVarMgrApi& api(*m_pApi);
    api.declare("/aninteger", "integer", "1234");
    std::vector<CVarMgrApi::VarInfo> info = api.lsvar();
    
    EQ(size_t(1), info.size());
    EQ(std::string("aninteger"), info[0].s_name);
    EQ(std::string("integer"),   info[0].s_typeName);
    EQ(std::string("1234"),      info[0].s_value);
}
// ::lsvarVars - list when there's more than one.

void ServerApiTests::lsvarVars()
{
    CVarMgrApi& api(*m_pApi);
    api.declare("/aninteger", "integer", "1234");
    api.declare("/areal", "real", "3.1416");
    api.declare("/astring", "string", "hello world");
    
    std::vector<CVarMgrApi::VarInfo> info = api.lsvar();
    EQ(size_t(3), info.size());
    
    varcheck(info, "aninteger", "integer", "1234");
    varcheck(info, "areal", "real", "3.1416");
    varcheck(info, "astring", "string", "hello world");
}

// ::lsvarRelpath - List variables in a path relative to the wd.

void ServerApiTests::lsvarRelpath()
{
    CVarMgrApi& api(*m_pApi);
    api.mkdir("/subdir");
    api.declare("/subdir/aninteger", "integer", "1234");
    api.declare("/subdir/areal", "real", "3.1416");
    api.declare("/subdir/astring", "string", "hello world");
    
    
    std::vector<CVarMgrApi::VarInfo> info = api.lsvar("subdir");
    EQ(size_t(3), info.size());
    
    varcheck(info, "aninteger", "integer", "1234");
    varcheck(info, "areal", "real", "3.1416");
    varcheck(info, "astring", "string", "hello world");
}
// ::lsvarAbsPath - List variables in an absolute path

void ServerApiTests::lsvarAbsPath()
{
    CVarMgrApi& api(*m_pApi);
    api.mkdir("/subdir");
    api.declare("/subdir/aninteger", "integer", "1234");
    api.declare("/subdir/areal", "real", "3.1416");
    api.declare("/subdir/astring", "string", "hello world");
    api.cd("subdir");
    
    std::vector<CVarMgrApi::VarInfo> info = api.lsvar("/subdir");
    EQ(size_t(3), info.size());
    
    varcheck(info, "aninteger", "integer", "1234");
    varcheck(info, "areal", "real", "3.1416");
    varcheck(info, "astring", "string", "hello world");
    
}
// Edge case -  a variable that is an empty string -- exposed a defect in
// split:

void ServerApiTests::lsvarEmptyString()
{
  CVarMgrApi& api(*m_pApi);
  api.declare("/astring", "string");
  std::vector<CVarMgrApi::VarInfo> info = api.lsvar("/");
  EQ(size_t(1), info.size());
  varcheck(info, "astring", "string", "");

}

// rmvar works:

void ServerApiTests::rmvarOk()
{
    CVarMgrApi& api(*m_pApi);
    
    api.declare("/anint", "integer", "1234");
    api.rmvar("/anint");
    std::vector<CVarMgrApi::VarInfo> info = api.lsvar("/");
    EQ(size_t(0), info.size());
}
// rmvar with no such variable throws.

void ServerApiTests::rmvarNoSuch()
{
    CVarMgrApi& api(*m_pApi);
    
    CPPUNIT_ASSERT_THROW(
        api.rmvar("/anint"),
        std::runtime_error
    );
}


