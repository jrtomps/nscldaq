// Template for a test suite.  -CStateManager tests.

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


static const std::string serviceName("vardb-request");
static const std::string serverName("vardbServer");

class TestStateManager : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(TestStateManager);
  CPPUNIT_TEST(getParentDirDefault);
  CPPUNIT_TEST(getParentDirModified);

  CPPUNIT_TEST(setParentDir);
  CPPUNIT_TEST(setParentDirNox);
  
  CPPUNIT_TEST(addProgramDefault);
  CPPUNIT_TEST(addProgramOtherDir);
  CPPUNIT_TEST(addProgramDuplicate);

  CPPUNIT_TEST(getProgramDefDefault);
  CPPUNIT_TEST(getProgramDefOtherDir);
  CPPUNIT_TEST(getNoxProgramDef);
  
  CPPUNIT_TEST(modifyProgramDefDefault);
  CPPUNIT_TEST(modifyProgramDefOtherDir);
  CPPUNIT_TEST(modifyNoxProgramDef);
  
  CPPUNIT_TEST(enableProgramDefault);
  CPPUNIT_TEST(disableProgramDefault);
  CPPUNIT_TEST(enableProgramOtherDir);
  CPPUNIT_TEST(disableProgramOtherDir);
  CPPUNIT_TEST(enableNoxProgram);
  CPPUNIT_TEST(disableNoxProgram);

  CPPUNIT_TEST(setStandaloneDefault);
  CPPUNIT_TEST(setStandaloneOtherDir);
  CPPUNIT_TEST(setStandaloneNox);
  CPPUNIT_TEST(setNostandaloneDefault);
  CPPUNIT_TEST(setNostandaloneOtherDir);
  CPPUNIT_TEST(setNostandaloneNox);
  
  // Note these tests tests both senses for their flags.
  
  CPPUNIT_TEST(isEnabledDefault);
  CPPUNIT_TEST(isEnabledOtherDir);
  CPPUNIT_TEST(isEnabledNox);
  CPPUNIT_TEST(isStandaloneDefault);
  CPPUNIT_TEST(isStandaloneOtherDir);
  CPPUNIT_TEST(isStandaloneNox);
  
  CPPUNIT_TEST(listNoProgramsDefault);
  CPPUNIT_TEST(listNoProgramsOtherDir);
  CPPUNIT_TEST(list1ProgramDefault);
  CPPUNIT_TEST(list1ProgramOtherDir);
  CPPUNIT_TEST(listnProgramsDefault);
  CPPUNIT_TEST(listnProgramsOtherDir);

  CPPUNIT_TEST(listNoActiveProgramsDefault);
  CPPUNIT_TEST(listNoActiveProgramsOtherDir);
  // CPPUNIT_TEST(listActiveProgramsOnly);
  // CPPUNIT_TEST(listActiveProgramsFromMix);
  
  CPPUNIT_TEST_SUITE_END();


protected:
  void getParentDirDefault();
  void getParentDirModified();
  
  void setParentDir();
  void setParentDirNox();
  
  void addProgramDefault();
  void addProgramOtherDir();
  void addProgramDuplicate();
  
  void getProgramDefDefault();
  void getProgramDefOtherDir();
  void getNoxProgramDef();
  
  void modifyProgramDefDefault();
  void modifyProgramDefOtherDir();
  void modifyNoxProgramDef();
  
  void enableProgramDefault();
  void enableProgramOtherDir();
  void enableNoxProgram();
  
  void disableProgramDefault();
  void disableProgramOtherDir();
  void disableNoxProgram();
  
  void setStandaloneDefault();
  void setStandaloneOtherDir();
  void setStandaloneNox();
  void setNostandaloneDefault();
  void setNostandaloneOtherDir();
  void setNostandaloneNox();
  
  void isEnabledDefault();
  void isEnabledOtherDir();
  void isEnabledNox();
  void isStandaloneDefault();
  void isStandaloneOtherDir();
  void isStandaloneNox();
  
  void listNoProgramsDefault();
  void listNoProgramsOtherDir();
  void list1ProgramDefault();
  void list1ProgramOtherDir();
  void listnProgramsDefault();
  void listnProgramsOtherDir();
  
  void listNoActiveProgramsDefault();
  void listNoActiveProgramsOtherDir();
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
TestStateManager::populateDb()
{
    // Define the data types:
    
    CVarMgrApi::EnumValues tf;
    tf.push_back("true");
    tf.push_back("false");
    
    m_pApi->defineEnum("boolean", tf);
    
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
    m_pApi->declare("/RunState/Recording", "boolean", "false");
    m_pApi->declare("/RunState/Timeout", "integer", "60");
    m_pApi->declare("/RunState/ReadoutParentDir", "string");
    
    // /RunState/test  - program named test.
    
    m_pApi->mkdir("/RunState/test");
    m_pApi->declare("/RunState/test/State", "RunStateMachine", "0Initial");
    m_pApi->declare("/RunState/test/enable", "boolean", "true");
    m_pApi->declare("/RunState/test/standalone", "boolean", "false");
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
TestStateManager::startServer()
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
TestStateManager::getRequestPort()
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
void TestStateManager::stopServer()
{
    int exitStatus;
    kill(m_serverPid, SIGKILL);
    waitpid(m_serverPid, &exitStatus, 0);
    
}


CPPUNIT_TEST_SUITE_REGISTRATION(TestStateManager);


// Default value should be "/RunState"

void TestStateManager::getParentDirDefault() {
    CStateManager sm("tcp://localhost", "tcp://localhost");
    
    EQ(std::string("/RunState"), sm.getProgramParentDir());
}

// Default value changed: Note construction of CStateManager freezes
// this.

void TestStateManager::getParentDirModified()
{
    m_pApi->mkdir("/Programs");
    m_pApi->set("/RunState/ReadoutParentDir", "/Programs");
    
    
    CStateManager sm("tcp://localhost", "tcp://localhost");
    EQ(std::string("/Programs"), sm.getProgramParentDir());
}
// set parent dir...also manages to update the sm information:

void TestStateManager::setParentDir()
{
    m_pApi->mkdir("/Programs");
    CStateManager sm("tcp://localhost", "tcp://localhost");
    sm.setProgramParentDir("/Programs");
    
    EQ(std::string("/Programs"), sm.getProgramParentDir());
}
// set parent dir but there's no such dir -- exception.

void TestStateManager::setParentDirNox()
{
    CStateManager sm("tcp://localhost", "tcp://localhost");
    CPPUNIT_ASSERT_THROW(
        sm.setProgramParentDir("/Programs"),
        std::runtime_error
    );
}
// Add a program into the default program directory.

void TestStateManager::addProgramDefault()
{
    // Fill in the program definition struct:
    
    CStateManager::ProgramDefinition pd;
    pd.s_enabled = true;
    pd.s_standalone = false;
    pd.s_path= "/some/fake/path";
    pd.s_host= "charlie.nscl.msu.edu";
    pd.s_outRing = "Output";
    pd.s_inRing  = "Input";
    
    CStateManager sm("tcp://localhost", "tcp://localhost");
    sm.addProgram("mytest", &pd);
    
    CPPUNIT_ASSERT_NO_THROW(
        m_pApi->cd("/RunState/mytest")
    );                                    // Program dir created.
    // Check directory contents:
    
    
    EQ(std::string("0Initial"), m_pApi->get("State"));
    EQ(std::string("true"),     m_pApi->get("enable"));
    EQ(std::string("false"),    m_pApi->get("standalone"));
    EQ(std::string("/some/fake/path"), m_pApi->get("path"));
    EQ(std::string("charlie.nscl.msu.edu"), m_pApi->get("host"));
    EQ(std::string("Output"),   m_pApi->get("outring"));
    EQ(std::string("Input"),    m_pApi->get("inring"));
}

// Add a progrem into a non default dir:

void TestStateManager::addProgramOtherDir()
{
    m_pApi->mkdir("/Programs");
    CStateManager sm("tcp://localhost", "tcp://localhost");
    sm.setProgramParentDir("/Programs");
    
    // Fill in the program definition struct:
    
    CStateManager::ProgramDefinition pd;
    pd.s_enabled = true;
    pd.s_standalone = false;
    pd.s_path= "/some/fake/path";
    pd.s_host= "charlie.nscl.msu.edu";
    pd.s_outRing = "Output";
    pd.s_inRing  = "Input";
    

    sm.addProgram("mytest", &pd);
    
    CPPUNIT_ASSERT_NO_THROW(
        m_pApi->cd("/Programs/mytest")
    );                                    // Program dir created.
    // Check directory contents:
    
    
    EQ(std::string("0Initial"), m_pApi->get("State"));
    EQ(std::string("true"),     m_pApi->get("enable"));
    EQ(std::string("false"),    m_pApi->get("standalone"));
    EQ(std::string("/some/fake/path"), m_pApi->get("path"));
    EQ(std::string("charlie.nscl.msu.edu"), m_pApi->get("host"));
    EQ(std::string("Output"),   m_pApi->get("outring"));
    EQ(std::string("Input"),    m_pApi->get("inring"));
}

// Adding a duplicate program is illegal:

void TestStateManager::addProgramDuplicate()
{
    // Fill in the program definition struct:
    
    CStateManager::ProgramDefinition pd;
    pd.s_enabled = true;
    pd.s_standalone = false;
    pd.s_path= "/some/fake/path";
    pd.s_host= "charlie.nscl.msu.edu";
    pd.s_outRing = "Output";
    pd.s_inRing  = "Input";
    
    CStateManager sm("tcp://localhost", "tcp://localhost");
    sm.addProgram("mytest", &pd);
    
    CPPUNIT_ASSERT_THROW(
        sm.addProgram("mytest", &pd),
        std::runtime_error
    );
}

// Get a good program def from the default dir:

void TestStateManager::getProgramDefDefault()
{
    // Add the mytest program:
    
        // Fill in the program definition struct:
    
    CStateManager::ProgramDefinition pd;
    pd.s_enabled = true;
    pd.s_standalone = false;
    pd.s_path= "/some/fake/path";
    pd.s_host= "charlie.nscl.msu.edu";
    pd.s_outRing = "Output";
    pd.s_inRing  = "Input";
    
    CStateManager sm("tcp://localhost", "tcp://localhost");
    sm.addProgram("mytest", &pd);
    
    // Get the def from it:
    
    CStateManager::ProgramDefinition def = sm.getProgramDefinition("mytest");
    EQ(def.s_enabled,    pd.s_enabled);
    EQ(def.s_standalone, pd.s_standalone);
    EQ(def.s_path,       pd.s_path);
    EQ(def.s_host,       pd.s_host);
    EQ(def.s_outRing,    pd.s_outRing);
    EQ(def.s_inRing,     pd.s_inRing);
}

// Get a good program def from non default dir:

void TestStateManager::getProgramDefOtherDir()
{
    m_pApi->mkdir("/Programs");
    CStateManager sm("tcp://localhost", "tcp://localhost");
    sm.setProgramParentDir("/Programs");
    
    // Fill in the program definition struct:
    
    CStateManager::ProgramDefinition pd;
    pd.s_enabled = true;
    pd.s_standalone = false;
    pd.s_path= "/some/fake/path";
    pd.s_host= "charlie.nscl.msu.edu";
    pd.s_outRing = "Output";
    pd.s_inRing  = "Input";
    

    sm.addProgram("mytest", &pd);
    
    // Get the def from it:
    
    CStateManager::ProgramDefinition def = sm.getProgramDefinition("mytest");
    EQ(def.s_enabled,    pd.s_enabled);
    EQ(def.s_standalone, pd.s_standalone);
    EQ(def.s_path,       pd.s_path);
    EQ(def.s_host,       pd.s_host);
    EQ(def.s_outRing,    pd.s_outRing);
    EQ(def.s_inRing,     pd.s_inRing);
}

// Get nonexistent program throws:

void TestStateManager::getNoxProgramDef()
{
    CStateManager sm("tcp://localhost", "tcp://localhost");
    CPPUNIT_ASSERT_THROW(
        sm.getProgramDefinition("mytest"),
        std::runtime_error
    );
}
// Modify a program:

void TestStateManager::modifyProgramDefDefault()
{
    // Add the mytest program:
    
        // Fill in the program definition struct:
    
    CStateManager::ProgramDefinition pd;
    pd.s_enabled = true;
    pd.s_standalone = false;
    pd.s_path= "/some/fake/path";
    pd.s_host= "charlie.nscl.msu.edu";
    pd.s_outRing = "Output";
    pd.s_inRing  = "Input";
    
    CStateManager sm("tcp://localhost", "tcp://localhost");
    sm.addProgram("mytest", &pd);
    
    // Modify it:
    
    pd.s_path   = "/some/other/path";
    pd.s_inRing = "tcp://remote.host/input";
    
    sm.modifyProgram("mytest", &pd);
    
    CStateManager::ProgramDefinition def = sm.getProgramDefinition("mytest");
    EQ(def.s_enabled,    pd.s_enabled);
    EQ(def.s_standalone, pd.s_standalone);
    EQ(def.s_path,       pd.s_path);
    EQ(def.s_host,       pd.s_host);
    EQ(def.s_outRing,    pd.s_outRing);
    EQ(def.s_inRing,     pd.s_inRing);
    
}
// Modify when the program's are not in the default dir:

void TestStateManager::modifyProgramDefOtherDir()
{
    m_pApi->mkdir("/Programs");
    CStateManager sm("tcp://localhost", "tcp://localhost");
    sm.setProgramParentDir("/Programs");
    
    // Fill in the program definition struct:
    
    CStateManager::ProgramDefinition pd;
    pd.s_enabled = true;
    pd.s_standalone = false;
    pd.s_path= "/some/fake/path";
    pd.s_host= "charlie.nscl.msu.edu";
    pd.s_outRing = "Output";
    pd.s_inRing  = "Input";
    

    sm.addProgram("mytest", &pd);

    // Modify it:
    
    pd.s_path   = "/some/other/path";
    pd.s_inRing = "tcp://remote.host/input";
    
    sm.modifyProgram("mytest", &pd);
    
    CStateManager::ProgramDefinition def = sm.getProgramDefinition("mytest");
    EQ(def.s_enabled,    pd.s_enabled);
    EQ(def.s_standalone, pd.s_standalone);
    EQ(def.s_path,       pd.s_path);
    EQ(def.s_host,       pd.s_host);
    EQ(def.s_outRing,    pd.s_outRing);
    EQ(def.s_inRing,     pd.s_inRing);
    
}
// Can only modify existing program:

void TestStateManager::modifyNoxProgramDef()
{
    CStateManager sm("tcp://localhost", "tcp://localhost");
    CStateManager::ProgramDefinition pd;
    pd.s_enabled = true;
    pd.s_standalone = false;
    pd.s_path= "/some/fake/path";
    pd.s_host= "charlie.nscl.msu.edu";
    pd.s_outRing = "Output";
    pd.s_inRing  = "Input";
    
    CPPUNIT_ASSERT_THROW(
        sm.modifyProgram("mytest", &pd),
        std::runtime_error
    );
}
// Should be able to set the enable state to true:

void TestStateManager::enableProgramDefault()
{
    // Make the program disabled:
    
    CStateManager sm("tcp://localhost", "tcp://localhost");
    CStateManager::ProgramDefinition pd;
    pd.s_enabled = false;
    pd.s_standalone = false;
    pd.s_path= "/some/fake/path";
    pd.s_host= "charlie.nscl.msu.edu";
    pd.s_outRing = "Output";
    pd.s_inRing  = "Input";
    

    sm.addProgram("mytest", &pd);
    
    // Enable it and ensure it is enabled:
    
    sm.enableProgram("mytest");
    
    CStateManager::ProgramDefinition def = sm.getProgramDefinition("mytest");
    ASSERT(def.s_enabled);
    
}
void TestStateManager::enableProgramOtherDir()
{
    m_pApi->mkdir("/Programs");
    CStateManager sm("tcp://localhost", "tcp://localhost");
    sm.setProgramParentDir("/Programs");
   
    CStateManager::ProgramDefinition pd;
    pd.s_enabled = false;
    pd.s_standalone = false;
    pd.s_path= "/some/fake/path";
    pd.s_host= "charlie.nscl.msu.edu";
    pd.s_outRing = "Output";
    pd.s_inRing  = "Input";
    

    sm.addProgram("mytest", &pd);
    
    sm.enableProgram("mytest");
    CStateManager::ProgramDefinition def = sm.getProgramDefinition("mytest");
    ASSERT(def.s_enabled);
}

void TestStateManager::enableNoxProgram()
{
    CStateManager sm("tcp://localhost", "tcp://localhost");
    CPPUNIT_ASSERT_THROW(
        sm.enableProgram("mytest"),
        std::runtime_error
    );
}

// Tests for disable program:

void TestStateManager::disableProgramDefault()
{
// Make the program disabled:
    
    CStateManager sm("tcp://localhost", "tcp://localhost");
    CStateManager::ProgramDefinition pd;
    pd.s_enabled = true;
    pd.s_standalone = false;
    pd.s_path= "/some/fake/path";
    pd.s_host= "charlie.nscl.msu.edu";
    pd.s_outRing = "Output";
    pd.s_inRing  = "Input";
    

    sm.addProgram("mytest", &pd);
    
    // Enable it and ensure it is enabled:
    
    sm.disableProgram("mytest");
    
    CStateManager::ProgramDefinition def = sm.getProgramDefinition("mytest");
    ASSERT(!def.s_enabled);
        
}

void TestStateManager::disableProgramOtherDir()
{
    m_pApi->mkdir("/Programs");
    CStateManager sm("tcp://localhost", "tcp://localhost");
    sm.setProgramParentDir("/Programs");
   
    CStateManager::ProgramDefinition pd;
    pd.s_enabled = true;
    pd.s_standalone = false;
    pd.s_path= "/some/fake/path";
    pd.s_host= "charlie.nscl.msu.edu";
    pd.s_outRing = "Output";
    pd.s_inRing  = "Input";
    

    sm.addProgram("mytest", &pd);
    
    sm.disableProgram("mytest");
    
    CStateManager::ProgramDefinition def = sm.getProgramDefinition("mytest");
    ASSERT(!def.s_enabled);
}

void TestStateManager::disableNoxProgram()
{
    CStateManager sm("tcp://localhost", "tcp://localhost");
    
    CPPUNIT_ASSERT_THROW(
        sm.disableProgram("mytest"),
        std::runtime_error
    );
}
//  Tests for setProgramStandalone -- the usual three

void TestStateManager::setStandaloneDefault()
{
    // Make the program - initially not standalone:
    
    CStateManager sm("tcp://localhost", "tcp://localhost");
    CStateManager::ProgramDefinition pd;
    pd.s_enabled = true;
    pd.s_standalone = false;
    pd.s_path= "/some/fake/path";
    pd.s_host= "charlie.nscl.msu.edu";
    pd.s_outRing = "Output";
    pd.s_inRing  = "Input";

    sm.addProgram("mytest", &pd);
    
    
    // Set it standalone:
    
    sm.setProgramStandalone("mytest");
    
    // Get the program definition:  Must have s_standalone true:
    
    CStateManager::ProgramDefinition def = sm.getProgramDefinition("mytest");
    ASSERT(def.s_standalone);
        
}


void TestStateManager::setStandaloneOtherDir()
{
    m_pApi->mkdir("/Programs");
    CStateManager sm("tcp://localhost", "tcp://localhost");
    sm.setProgramParentDir("/Programs");
   
    CStateManager::ProgramDefinition pd;
    pd.s_enabled = true;
    pd.s_standalone = false;
    pd.s_path= "/some/fake/path";
    pd.s_host= "charlie.nscl.msu.edu";
    pd.s_outRing = "Output";
    pd.s_inRing  = "Input";
    

    sm.addProgram("mytest", &pd);

    sm.setProgramStandalone("mytest");
    // Get the program definition:  Must have s_standalone true:
    
    CStateManager::ProgramDefinition def = sm.getProgramDefinition("mytest");
    ASSERT(def.s_standalone);
    
}
void TestStateManager::setStandaloneNox()
{
    CStateManager sm("tcp://localhost", "tcp://localhost");
    CPPUNIT_ASSERT_THROW(
        sm.setProgramStandalone("mytest"),
        std::runtime_error
    );
}
// Tests for setProgramNoStandalone:

void TestStateManager::setNostandaloneDefault()
{
    // Make the program - initially  standalone:
    
    CStateManager sm("tcp://localhost", "tcp://localhost");
    CStateManager::ProgramDefinition pd;
    pd.s_enabled = true;
    pd.s_standalone = true;
    pd.s_path= "/some/fake/path";
    pd.s_host= "charlie.nscl.msu.edu";
    pd.s_outRing = "Output";
    pd.s_inRing  = "Input";

    sm.addProgram("mytest", &pd);
    
    
    // Set it standalone:
    
    sm.setProgramNoStandalone("mytest");
    
    // Get the program definition:  Must have s_standalone true:
    
    CStateManager::ProgramDefinition def = sm.getProgramDefinition("mytest");
    ASSERT(!def.s_standalone);
           
}

void TestStateManager::setNostandaloneOtherDir()
{
    m_pApi->mkdir("/Programs");
    CStateManager sm("tcp://localhost", "tcp://localhost");
    sm.setProgramParentDir("/Programs");
   
    CStateManager::ProgramDefinition pd;
    pd.s_enabled = true;
    pd.s_standalone = true;
    pd.s_path= "/some/fake/path";
    pd.s_host= "charlie.nscl.msu.edu";
    pd.s_outRing = "Output";
    pd.s_inRing  = "Input";
    

    sm.addProgram("mytest", &pd);

    sm.setProgramNoStandalone("mytest");
    // Get the program definition:  Must have s_standalone true:
    
    CStateManager::ProgramDefinition def = sm.getProgramDefinition("mytest");
    ASSERT(!def.s_standalone);

}

void TestStateManager::setNostandaloneNox()
{
    CStateManager sm("tcp://localhost", "tcp://localhost");
    CPPUNIT_ASSERT_THROW(
        sm.setProgramNoStandalone("mytest"),
        std::runtime_error
    );    
}
// Tests for isEnabled - each test checks both states:

void TestStateManager::isEnabledDefault()
{
    CStateManager sm("tcp://localhost", "tcp://localhost");
    CStateManager::ProgramDefinition pd;
    pd.s_enabled = true;
    pd.s_standalone = true;
    pd.s_path= "/some/fake/path";
    pd.s_host= "charlie.nscl.msu.edu";
    pd.s_outRing = "Output";
    pd.s_inRing  = "Input";

    sm.addProgram("mytest", &pd);
    
    // Enabled now:
    
    ASSERT(sm.isProgramEnabled("mytest"));
    
    // Disable:
    
    sm.disableProgram("mytest");
    ASSERT(!sm.isProgramEnabled("mytest"));
}

void TestStateManager::isEnabledOtherDir()
{
    m_pApi->mkdir("/Programs");
    CStateManager sm("tcp://localhost", "tcp://localhost");
    sm.setProgramParentDir("/Programs");
   
    CStateManager::ProgramDefinition pd;
    pd.s_enabled = true;
    pd.s_standalone = true;
    pd.s_path= "/some/fake/path";
    pd.s_host= "charlie.nscl.msu.edu";
    pd.s_outRing = "Output";
    pd.s_inRing  = "Input";
    

    sm.addProgram("mytest", &pd);


    ASSERT(sm.isProgramEnabled("mytest"));
    
    sm.disableProgram("mytest");
    ASSERT(!sm.isProgramEnabled("mytest"));
}

void TestStateManager::isEnabledNox()
{
    CStateManager sm("tcp://localhost", "tcp://localhost");
    CPPUNIT_ASSERT_THROW(
        sm.isProgramEnabled("mytest"),
        std::runtime_error
    );
}

// Tests for isProgramStandalone:

void TestStateManager::isStandaloneDefault()
{
    CStateManager sm("tcp://localhost", "tcp://localhost");
    CStateManager::ProgramDefinition pd;
    pd.s_enabled = true;
    pd.s_standalone = false;
    pd.s_path= "/some/fake/path";
    pd.s_host= "charlie.nscl.msu.edu";
    pd.s_outRing = "Output";
    pd.s_inRing  = "Input";

    sm.addProgram("mytest", &pd);
    
    ASSERT(!sm.isProgramStandalone("mytest"));
    
    sm.setProgramStandalone("mytest");
    
    ASSERT(sm.isProgramStandalone("mytest"));
}

void TestStateManager::isStandaloneOtherDir()
{
    m_pApi->mkdir("/Programs");
    CStateManager sm("tcp://localhost", "tcp://localhost");
    sm.setProgramParentDir("/Programs");
   
    CStateManager::ProgramDefinition pd;
    pd.s_enabled = true;
    pd.s_standalone = false;
    pd.s_path= "/some/fake/path";
    pd.s_host= "charlie.nscl.msu.edu";
    pd.s_outRing = "Output";
    pd.s_inRing  = "Input";
    

    sm.addProgram("mytest", &pd);
    
    ASSERT(!sm.isProgramStandalone("mytest"));
    
    sm.setProgramStandalone("mytest");
    ASSERT(sm.isProgramStandalone("mytest"));
    
}
void TestStateManager::isStandaloneNox()
{
    CStateManager sm("tcp://localhost", "tcp://localhost");
    CPPUNIT_ASSERT_THROW(
        sm.isProgramStandalone("mytest"),
        std::runtime_error
    );
}
// listPrograms tests:

// If there are no programs I get an empty list:
// (remember setup makes program "test")

void TestStateManager::listNoProgramsDefault()
{
    CStateManager sm("tcp://localhost", "tcp://localhost");
     
     // Pending creation of deletProgram api element.
     
     m_pApi->rmvar("/RunState/test/State");
     m_pApi->rmvar("/RunState/test/enable");
     m_pApi->rmvar("/RunState/test/standalone");
     m_pApi->rmvar("/RunState/test/path");
     m_pApi->rmvar("/RunState/test/host");
     m_pApi->rmvar("/RunState/test/outring");
     m_pApi->rmvar("/RunState/test/inring");
     
     m_pApi->rmdir("/RunState/test");
     
     std::vector<std::string> progs = sm.listPrograms();
     
     EQ(size_t(0), progs.size());

}
// Easier in another dir:

void TestStateManager::listNoProgramsOtherDir()
{
    m_pApi->mkdir("/Programs");
    CStateManager sm("tcp://localhost", "tcp://localhost");
    sm.setProgramParentDir("/Programs");
    
    std::vector<std::string> progs = sm.listPrograms();
    EQ(size_t(0), progs.size());
}

// There's one program, "test" in the default dir:

void TestStateManager::list1ProgramDefault()
{
    CStateManager sm("tcp://localhost", "tcp://localhost");
    std::vector<std::string> progs = sm.listPrograms();
    
    EQ(size_t(1), progs.size());
    EQ(std::string("test"), progs[0]);
}
// There's one program (we need to make it) in the non default dir:

void TestStateManager::list1ProgramOtherDir()
{
    m_pApi->mkdir("/Programs");
    CStateManager sm("tcp://localhost", "tcp://localhost");
    sm.setProgramParentDir("/Programs");
   
    CStateManager::ProgramDefinition pd;
    pd.s_enabled = true;
    pd.s_standalone = false;
    pd.s_path= "/some/fake/path";
    pd.s_host= "charlie.nscl.msu.edu";
    pd.s_outRing = "Output";
    pd.s_inRing  = "Input";
    

    sm.addProgram("mytest", &pd);
    
    std::vector<std::string> progs = sm.listPrograms();
    EQ(size_t(1), progs.size());
    EQ(std::string("mytest"), progs[0]);
}
//  Several programs in default dir:

void TestStateManager::listnProgramsDefault()
{
    CStateManager sm("tcp://localhost", "tcp://localhost");
    CStateManager::ProgramDefinition pd;
    pd.s_enabled = true;
    pd.s_standalone = false;
    pd.s_path= "/some/fake/path";
    pd.s_host= "charlie.nscl.msu.edu";
    pd.s_outRing = "Output";
    pd.s_inRing  = "Input";

    sm.addProgram("atest", &pd);
    sm.addProgram("btest", &pd);
    sm.addProgram("ztest", &pd);
    
    std::vector<std::string> progs = sm.listPrograms();
    
    EQ(size_t(4), progs.size());   // Remember test is already there.
    
    // They should pop out alphabetically:
    
    EQ(std::string("atest"), progs[0]);
    EQ(std::string("btest"), progs[1]);
    EQ(std::string("test"), progs[2]);
    EQ(std::string("ztest"), progs[3]);
}
// Several programs in the other dir:

void TestStateManager::listnProgramsOtherDir()
{
    m_pApi->mkdir("/Programs");
    CStateManager sm("tcp://localhost", "tcp://localhost");
    sm.setProgramParentDir("/Programs");
   
    CStateManager::ProgramDefinition pd;
    pd.s_enabled = true;
    pd.s_standalone = false;
    pd.s_path= "/some/fake/path";
    pd.s_host= "charlie.nscl.msu.edu";
    pd.s_outRing = "Output";
    pd.s_inRing  = "Input";
    
    sm.addProgram("atest", &pd);
    sm.addProgram("btest", &pd);
    sm.addProgram("ztest", &pd);
    
    std::vector<std::string> progs = sm.listPrograms();
    
    EQ(size_t(3), progs.size());   // there's no 'test' here.
    EQ(std::string("atest"), progs[0]);
    EQ(std::string("btest"), progs[1]);
    EQ(std::string("ztest"), progs[2]);
    
}
// Test for listActivePrograms.

// No active programs though there's a program:

void TestStateManager::listNoActiveProgramsDefault()
{
    CStateManager sm("tcp://localhost", "tcp://localhost");
    sm.disableProgram("test");
    
    std::vector<std::string> progs = sm.listEnabledPrograms();
    
    EQ(size_t(0), progs.size());
}
// in other dir:

void TestStateManager::listNoActiveProgramsOtherDir()
{
    m_pApi->mkdir("/Programs");
    CStateManager sm("tcp://localhost", "tcp://localhost");
    sm.setProgramParentDir("/Programs");
   
    CStateManager::ProgramDefinition pd;
    pd.s_enabled = false;              // Disabled.
    pd.s_standalone = false;
    pd.s_path= "/some/fake/path";
    pd.s_host= "charlie.nscl.msu.edu";
    pd.s_outRing = "Output";
    pd.s_inRing  = "Input";
    
    sm.addProgram("atest", &pd);
    
    std::vector<std::string> progs = sm.listEnabledPrograms();
    EQ(size_t(0), progs.size());
}