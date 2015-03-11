// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"

/* Test the direct api (uses the database file). */

#include "CVarMgrFileApi.h"
#include <CVariableDb.h>
#include <CVarDirTree.h>
#include <CVariable.h>
#include <CSqliteException.h>
#include <CSqlite.h>
#include <unistd.h>
#include <string.h>
#include <stdexcept>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


// White box:  We are assuming the API is sitting on top of the primitive API
//             and therefore we don't need to test the edge cases as those
//             were tested when that API was tested.

class TestDirect : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(TestDirect);
  CPPUNIT_TEST(constructOk);
  CPPUNIT_TEST(constructNoSuchFile);
  CPPUNIT_TEST(constructBadSchema);
  
  CPPUNIT_TEST(mkdir);
  CPPUNIT_TEST(cdOkgetwd);
  
  CPPUNIT_TEST(cdNoSuch);
  
  CPPUNIT_TEST(rmdirOk);
  
  CPPUNIT_TEST(declareOk);
  
  CPPUNIT_TEST(setOk);
  
  CPPUNIT_TEST(get);
  
  CPPUNIT_TEST(defEnum);
  
  CPPUNIT_TEST(defStateMachine);
  CPPUNIT_TEST_SUITE_END();


private:
    char m_tempFile[100];
    int  m_fd;
public:
  void setUp() {
    strcpy(m_tempFile, "testvardbXXXXXX");
    m_fd = mkstemp(m_tempFile);
    if(m_fd == -1) {
        throw std::runtime_error(strerror(errno));
    }
    CVariableDb::create(m_tempFile);
  }
  void tearDown() {
    close(m_fd);
    unlink(m_tempFile);
  }
protected:
  void constructOk();
  void constructNoSuchFile();
  void constructBadSchema();
  
  void mkdir();

  
  void cdNoSuch();
  void cdOkgetwd();
  
  void rmdirOk();
  
  void declareOk();
  
  void setOk();
  
  void get();
  
  void defEnum();

  void defStateMachine();
};

CPPUNIT_TEST_SUITE_REGISTRATION(TestDirect);

// Constructing on a valid database file does not kill us
void TestDirect::constructOk() {
    
    CPPUNIT_ASSERT_NO_THROW(CVarMgrFileApi api(m_tempFile));
}
// Constructing with no such file is a killer:

void TestDirect::constructNoSuchFile()
{
    CPPUNIT_ASSERT_THROW(CVarMgrFileApi api("/no/such/file"), CSqliteException);
}
// Construct with existing file that does not have database schema:

void TestDirect::constructBadSchema()
{
    close(m_fd);
    unlink(m_tempFile);
    m_fd = creat(m_tempFile, S_IRWXU);
    CSqlite db(m_tempFile);               // Init it as an empty database.
    CPPUNIT_ASSERT_THROW(CVarMgrFileApi api(m_tempFile), CVariableDb::CException);
    
}
// Test ability of the api to make a directory:

void TestDirect::mkdir()
{
    CVarMgrFileApi api(m_tempFile);
    api.mkdir("/atest");
    CVariableDb db(m_tempFile);
    CVarDirTree dir(db);
    std::vector<CVarDirTree::DirInfo> info = dir.ls();
    EQ(size_t(1), info.size());
    EQ(std::string("atest"), info[0].s_name);
}

// An exception is thrown if we cd to a nonexistent dir:

void TestDirect::cdNoSuch()
{
    CVarMgrFileApi api(m_tempFile);
    CPPUNIT_ASSERT_THROW(api.cd("/no/such/directory"), std::runtime_error);
}
// cd to an existing directory ..the path can be fetched.

void TestDirect::cdOkgetwd()
{
    CVarMgrFileApi api(m_tempFile);
    api.mkdir("/this/is/a/big/set/of/directories");
    api.cd("/this/is/a");
    EQ(std::string("/this/is/a"), api.getwd() );
}

// rmdir removes an existing directory

void TestDirect::rmdirOk()
{
    CVarMgrFileApi api(m_tempFile);
    api.mkdir("/this/is/a/big/set/of/directories");
    api.rmdir("/this/is/a/big/set/of/directories");
    
    CVariableDb db(m_tempFile);
    CVarDirTree dir(db);

    dir.cd("/this/is/a/big/set/of");
    std::vector<CVarDirTree::DirInfo> info = dir.ls();
    EQ(size_t(0), info.size());
    
}
// Can declare a variable:

void TestDirect::declareOk()
{
    CVarMgrFileApi api(m_tempFile);
    api.mkdir("/this/is/a/big/set/of/directories");
    api.cd("/this/is/a");
    api.declare("variable", "integer");
    
    CVariableDb db(m_tempFile);
    CVarDirTree dir(db);
    dir.cd("/this/is/a");
    CVariable v(db, dir, "variable");
    
}
// Can modify a variable's value:

void TestDirect::setOk()
{
    CVarMgrFileApi api(m_tempFile);
    api.mkdir("/this/is/a/big/set/of/directories");
    api.cd("/this/is/a");
    api.declare("variable", "integer");
    api.set("variable", "1234");
    
    CVariableDb db(m_tempFile);
    CVarDirTree dir(db);
    dir.cd("/this/is/a");
    CVariable v(db, dir, "variable");
    EQ(std::string("1234"), v.get());
}
// Can get a variable's value:

void TestDirect::get()
{
    CVarMgrFileApi api(m_tempFile);
    api.mkdir("/this/is/a/big/set/of/directories");
    api.cd("/this/is/a");
    api.declare("variable", "integer");
    api.set("variable", "1234");
    EQ(std::string("1234"), api.get("variable"));
}

// Define an enum -- can make a variable of that type:

void TestDirect::defEnum()
{
    CVarMgrFileApi api(m_tempFile);
    CVarMgrApi::EnumValues colors;

    colors.push_back("red");
    colors.push_back("green");
    colors.push_back("blue");
    api.defineEnum("colors", colors);
    api.declare("/var", "colors");

    CVariableDb db(m_tempFile);
    CVarDirTree d(db);
    std::vector<CVariable::VarInfo> vinfo = CVariable::list(&db, d);
    EQ(size_t(1), vinfo.size());
    EQ(std::string("colors"), vinfo[0].s_type);
    
}
// Define a state machine -- make a variable of that type:

void TestDirect::defStateMachine()
{
    CVarMgrFileApi api(m_tempFile);
    CVarMgrApi::StateMap transitions;
    
    // Minimal 2 state machine
    
    api.addTransition(transitions, "0state1", "state2");
    api.addTransition(transitions, "state2", "0state1");
    api.addTransition(transitions, "state2", "state2");   //What the hell.
    
    api.defineStateMachine("silly", transitions);
    api.declare("/var", "silly");
    
    CVariableDb db(m_tempFile);
    CVarDirTree d(db);
    std::vector<CVariable::VarInfo> vinfo = CVariable::list(&db, d);
    EQ(size_t(1), vinfo.size());
    EQ(std::string("silly"), vinfo[0].s_type);
    
    
}