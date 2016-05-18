// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include <memory>

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
#include <algorithm>


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
  
  CPPUNIT_TEST(lsEmpty);
  CPPUNIT_TEST(lsList);
  CPPUNIT_TEST(lsAbsPath);
  CPPUNIT_TEST(lsRelPath);
  
  CPPUNIT_TEST(defStateMachine);
  
  CPPUNIT_TEST(lsvarEmpty);
  CPPUNIT_TEST(lsvarOneVar);
  CPPUNIT_TEST(lsvarVars);
  CPPUNIT_TEST(lsvarRelpath);
  CPPUNIT_TEST(lsvarAbsPath);
  CPPUNIT_TEST(lsvarAbsPath2);
  
  CPPUNIT_TEST(rmvarOk);
  CPPUNIT_TEST(rmvarNoSuch);
  
  // Transactions
  
  CPPUNIT_TEST(committed);
  CPPUNIT_TEST(explicitcommit);
  CPPUNIT_TEST(rolledback);
  CPPUNIT_TEST(scheduledRollback);
  
  
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
  
  void lsEmpty();
  void lsList();
  void lsAbsPath();
  void lsRelPath();
  
  void lsvarEmpty();
  void lsvarOneVar();
  void lsvarVars();
  void lsvarRelpath();
  void lsvarAbsPath();
  void lsvarAbsPath2();
  
  void rmvarOk();
  void rmvarNoSuch();
  
  void committed();
  void explicitcommit();
  void rolledback();
  void scheduledRollback();
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
// List initially gives an empty vector>

void TestDirect::lsEmpty()
{
    CVarMgrFileApi api(m_tempFile);
    EQ(size_t(0), api.ls().size());
}

// List nonempty directory:

void TestDirect::lsList()
{
    CVarMgrFileApi api(m_tempFile);
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
void TestDirect::lsAbsPath()
{
    CVarMgrFileApi api(m_tempFile);
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

void TestDirect::lsRelPath()
{
    CVarMgrFileApi api(m_tempFile);
    api.mkdir("/adir");
    api.mkdir("/anotherdir");
    api.mkdir("/lastdir");
    api.cd("/adir");
    api.mkdir("/anotherdir/test");
    
    std::vector<std::string> dirs = api.ls("../anotherdir");
    EQ(size_t(1), dirs.size());
    EQ(std::string("test"), dirs[0]);
}

// lsvar with empty dir gives empty list:

void TestDirect::lsvarEmpty()
{
    CVarMgrFileApi api(m_tempFile);
    EQ(size_t(0), api.lsvar().size());
}
// lsvar with a single variable in the wd gives the stuff for that var:

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

void TestDirect::lsvarOneVar()
{
    CVarMgrFileApi api(m_tempFile);
    api.declare("/aninteger", "integer", "1234");
    std::vector<CVarMgrApi::VarInfo> info = api.lsvar();
    
    EQ(size_t(1), info.size());
    EQ(std::string("aninteger"), info[0].s_name);
    EQ(std::string("integer"),   info[0].s_typeName);
    EQ(std::string("1234"),      info[0].s_value);
}
// lsvarVars - list when there's more than one.

void TestDirect::lsvarVars()
{
    CVarMgrFileApi api(m_tempFile);
    api.declare("/aninteger", "integer", "1234");
    api.declare("/areal", "real", "3.1416");
    api.declare("/astring", "string", "hello world");
    
    std::vector<CVarMgrApi::VarInfo> info = api.lsvar();
    EQ(size_t(3), info.size());
    
    varcheck(info, "aninteger", "integer", "1234");
    varcheck(info, "areal", "real", "3.1416");
    varcheck(info, "astring", "string", "hello world");
}

// lsvarRelpath - List variables in a path relative to the wd.

void TestDirect::lsvarRelpath()
{
    CVarMgrFileApi api(m_tempFile);
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
// lsvarAbsPath - List variables in an absolute path

void TestDirect::lsvarAbsPath()
{
    CVarMgrFileApi api(m_tempFile);
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

// lsvarAbsPath2 - same as above but listing / -- which initiall fails:

void TestDirect::lsvarAbsPath2()
{
    CVarMgrFileApi api(m_tempFile);
    api.mkdir("/subdir");
    api.declare("/aninteger", "integer", "1234");
    api.declare("/areal", "real", "3.1416");
    api.declare("/astring", "string", "hello world");
    api.cd("subdir");
    
    std::vector<CVarMgrApi::VarInfo> info = api.lsvar("/");
    EQ(size_t(3), info.size());
    
    varcheck(info, "aninteger", "integer", "1234");
    varcheck(info, "areal", "real", "3.1416");
    varcheck(info, "astring", "string", "hello world");
    
}

// Deleting works:

void TestDirect::rmvarOk()
{
    CVarMgrFileApi api(m_tempFile);
    
    api.declare("/avar", "integer", "1234");
    
    api.rmvar("/avar");
    
    std::vector<CVarMgrApi::VarInfo> info = api.lsvar();
    EQ(size_t(0), info.size());
}
// Throw if removing a nonexistent variable:

void TestDirect::rmvarNoSuch()
{
    CVarMgrFileApi api(m_tempFile);
    
    CPPUNIT_ASSERT_THROW(
        api.rmvar("/avar"),
        std::runtime_error
    );
}

// Test transaction commits if nothing goes wrong.

void TestDirect::committed()
{
  CVarMgrFileApi api(m_tempFile);
  
  try {
    std::unique_ptr<CVarMgrApi::Transaction> t(api.transaction());
    
    // do some stuff in the db.
    
    api.declare("/avar", "integer", "12234");
  }                              // Commited here.
  catch(...) {}
  // be sure it all showed up.
  
  std::string value;
  CPPUNIT_ASSERT_NO_THROW(
    value = api.get("/avar")
  );
  
  EQ(std::string("12234"), value);
  
}

// Explicit commits also work:

void TestDirect::explicitcommit()
{
  CVarMgrFileApi api(m_tempFile);
  
  try {
    std::unique_ptr<CVarMgrApi::Transaction> t(api.transaction());
    
    // do some stuff in the db.
    
    api.declare("/avar", "integer", "12234");
    t->commit();                             // Explicit commit here.
    
  }
  catch(...) {}                    // Hard to actualy tell the diff.
  
  std::string value;
  CPPUNIT_ASSERT_NO_THROW(
    value = api.get("/avar")
  );
  
  EQ(std::string("12234"), value);
 
}

// rollback using typical throw method:

void TestDirect::rolledback()
{
  CVarMgrFileApi api(m_tempFile);
  
  {
    std::unique_ptr<CVarMgrApi::Transaction> t(api.transaction());
    try {
      
      
      // do some stuff in the db.
      
      api.declare("/avar", "integer", "12234");
      throw(1);
    }
    catch (...) {
      t->rollback();
    }
  }  
  CPPUNIT_ASSERT_THROW(
    api.get("/avar"),
    std::exception
  );
}

// Rollback scheduled for transaction object destruction:

void TestDirect::scheduledRollback()
{
CVarMgrFileApi api(m_tempFile);
  
  try {
    std::unique_ptr<CVarMgrApi::Transaction> t(api.transaction());
    
    // do some stuff in the db.
    
    api.declare("/avar", "integer", "12234");
    t->scheduleRollback();          // Rollback on destruction.
    
    // Since the rollback has not yet occured we should be able to see the var:

    std::string value;
    CPPUNIT_ASSERT_NO_THROW(
      value = api.get("/avar")
    );
    
    EQ(std::string("12234"), value);
      
  }
  catch (...) {                  // Actual rollback.
    
  }
  
  CPPUNIT_ASSERT_THROW(
    api.get("/avar"),
    std::exception
  );  
}