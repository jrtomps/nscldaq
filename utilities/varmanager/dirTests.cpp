// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <string>
#include <stdexcept>

#include "CVariableDb.h"

// The define enables white box testing:


#include "CVarDirTree.h"


#include <CSqliteStatement.h>


class DirTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(DirTests);
  CPPUNIT_TEST(isAbsolute);
  CPPUNIT_TEST(isRelative);

  
  CPPUNIT_TEST(parseRelative);
  CPPUNIT_TEST(parseAbsolute);
  CPPUNIT_TEST(removeEmpty);
  
  CPPUNIT_TEST(mkdirTopLevel);
  CPPUNIT_TEST(makeAbsPath);
  CPPUNIT_TEST(mkdirNoPath);
  
  // Sensible relative path creations are going to need cd to be working.
  
  CPPUNIT_TEST(cdSimple);
  CPPUNIT_TEST(cdBadPath);
  CPPUNIT_TEST(cdRelative);
  CPPUNIT_TEST(cdDotDot);
  CPPUNIT_TEST(cdDotDotTooMuch);
  CPPUNIT_TEST(cdRoot);

  // Now we have sufficient mechanism to do relative mkdirs and mkdirs
  // with ..'s in the path.
  
  CPPUNIT_TEST(mkdirRelative);
  CPPUNIT_TEST(mkdirDotDot);
  
  CPPUNIT_TEST(lsEmpty);
  CPPUNIT_TEST(lsNotEmpty);
  
  CPPUNIT_TEST(rmEmptyDir);
  CPPUNIT_TEST(rmNonEmptyDir);
  CPPUNIT_TEST(rmNoSuchDir);
  CPPUNIT_TEST(rmRoot);
  
  CPPUNIT_TEST(constructById);
  CPPUNIT_TEST(constructByBadId);
  CPPUNIT_TEST(constructByRootId);
  
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
  void isAbsolute();
  void isRelative();
  
  void parseRelative();
  void parseAbsolute();
  void removeEmpty();
  
  void mkdirTopLevel();
  void makeAbsPath();
  void mkdirNoPath();
  
  void cdSimple();
  void cdBadPath();
  void cdRelative();
  void cdDotDot();
  void cdDotDotTooMuch();
  void cdRoot();
  
  void mkdirRelative();
  void mkdirDotDot();
  
  void lsEmpty();
  void lsNotEmpty();
  
  void rmEmptyDir();
  void rmNonEmptyDir();
  void rmNoSuchDir();
  
  void rmRoot();
  
  void constructById();
  void constructByBadId();
  void constructByRootId();
private:
    int findRoot(CVariableDb& db);
    CVarDirTree::DirInfo findDir(CVariableDb& db, const char* path);
};

CPPUNIT_TEST_SUITE_REGISTRATION(DirTests);

// If the first char of a path is '/' the path is absolute.

void DirTests::isAbsolute()
{
    CPPUNIT_ASSERT(!CVarDirTree::isRelative("/this/that/the/other"));

}
void DirTests::isRelative()
{
    CPPUNIT_ASSERT(CVarDirTree::isRelative("this/that/the/other"));
}
// Divide the path into a vector of components when it's relative:

void DirTests::parseRelative()
{
    std::vector<std::string> path = CVarDirTree::parsePath("this/that/the/other");
    EQ(std::string("this"), path[0]);
    EQ(std::string("that"), path[1]);
    EQ(std::string("the"), path[2]);
    EQ(std::string("other"), path[3]);
}
// When absolute:
void DirTests::parseAbsolute()
{
    std::vector<std::string> path = CVarDirTree::parsePath("/this/that/the/other");
    EQ(std::string("this"), path[0]);
    EQ(std::string("that"), path[1]);
    EQ(std::string("the"), path[2]);
    EQ(std::string("other"), path[3]);
}

void DirTests::removeEmpty()
{
    std::vector<std::string> path = CVarDirTree::parsePath("/this/that//the////other");
    EQ(std::string("this"), path[0]);
    EQ(std::string("that"), path[1]);
    EQ(std::string("the"), path[2]);
    EQ(std::string("other"), path[3]);    
}

// Make a directory in the top level:

void DirTests::mkdirTopLevel()
{
    CVariableDb db(m_tempFile);
    CVarDirTree t(db);
    
    t.mkdir("/stuff");
    
    // There should now be a directory named stuff whose parent is the root.
    
    int rootId = findRoot(db);

    CSqliteStatement sFindDir(
        db,
        "SELECT COUNT(*) c FROM directory WHERE name ='stuff' AND parent_id=:rootid"
    );
    sFindDir.bind(1, rootId);
    ++sFindDir;
    
    EQ(1, sFindDir.getInt(0));
}
// make an entire absolute directroy path:

void DirTests::makeAbsPath()
{
    CVariableDb db(m_tempFile);
    CVarDirTree t(db);
    
    const char* path = "/this/that/and/the/other";
    
    t.mkdir(path);
    std::vector<std::string> pathVec = CVarDirTree::parsePath(path);
    
    // Ensure all path elements exist:
    
    int parentId = findRoot(db);
    CSqliteStatement getNextEle(
        db,
        "SELECT id FROM directory WHERE name = :name AND parent_id = :parent"
    );
    for (int i = 0; i < pathVec.size(); i++) {
        getNextEle.bind(1, pathVec[i].c_str(), -1, SQLITE_TRANSIENT);
        getNextEle.bind(2, parentId);
        ++getNextEle;
        ASSERT(! (getNextEle.atEnd()));      // The element must exist!
        
        parentId = getNextEle.getInt(0);     // Prep for next iteration
        getNextEle.reset();              
    }
}
// If we don't ask it to make intermediate paths then mkdir should fail if needs to

void DirTests::mkdirNoPath()
{
    CVariableDb db(m_tempFile);
    CVarDirTree  t(db);
    
    const char* path = "/this/that";           // That does not exist should fail.
    
    CPPUNIT_ASSERT_THROW(t.mkdir(path, false), CVarDirTree::CException);
    
}
// Cd to an existing path element using an abspath.

void DirTests::cdSimple()
{
    CVariableDb db(m_tempFile);
    CVarDirTree t(db);
    
    const char* path1="/this/that/the/other";
    const char* path2="/this/that/the";
    
    t.mkdir(path1);          // Our cd will be on this path.
    t.cd(path2);
    
    EQ(std::string("the"), t.getwd().s_name);
    
    // FIgure out the id/parent of /this/that.
    
    CVarDirTree::DirInfo info = findDir(db, path2);
    EQ(info.s_id, t.getwd().s_id);
    EQ(info.s_parentId, t.getwd().s_parentId);
}
void DirTests::cdBadPath()
{
    CVariableDb db(m_tempFile);
    CVarDirTree t(db);
    
    const char* path1="/this/that/the/other";
    const char* path2="/this/mistake/the/other";
    
    t.mkdir(path1);
    
    CPPUNIT_ASSERT_THROW(t.cd(path2), CVarDirTree::CException);
}

void DirTests::cdRelative()
{
    CVariableDb db(m_tempFile);
    CVarDirTree t(db);
    
    const char* path1="/this/that/the/other";
    const char* path2="/this";
    const char* path3="that";
    const char* finalDir = "/this/that";
    
    t.mkdir(path1);
    t.cd(path2);
    t.cd(path3);
    
    EQ(std::string("that"), t.getwd().s_name);
    CVarDirTree::DirInfo info = findDir(db, finalDir);
    EQ(info.s_id, t.getwd().s_id);
    EQ(info.s_parentId, t.getwd().s_parentId);
    
}

// cd when path has ..'s in it.. we're doing the worst case:
// a relative cd where .. is the first path element.

void DirTests::cdDotDot()
{
    CVariableDb db(m_tempFile);
    CVarDirTree t(db);
    
    const char* path1="/this/that/the/other";
    const char* path2="/this/that/the/else";
    const char* cd1  = "/this/that/the/other";
    const char* cd2  = "../else";
    
    t.mkdir(path1);
    t.mkdir(path2);
    t.cd(cd1);
    t.cd(cd2);
    
    CVarDirTree::DirInfo info = findDir(db, path2);
    EQ(info.s_name, t.getwd().s_name);
    EQ(info.s_id,   t.getwd().s_id);
    EQ(info.s_parentId, t.getwd().s_parentId);
}

// CD / should work:

void DirTests::cdRoot()
{
    CVariableDb db(m_tempFile);
    CVarDirTree t(db);
    
    const char* path1="/this/that/the/other";
    
    t.mkdir(path1);
    t.cd(path1);
    
    t.cd("/");
    
    EQ(std::string("/"), t.wdPath());
}

// if .. goes above the root directory expect an exception:

void DirTests::cdDotDotTooMuch()
{
    CVariableDb db(m_tempFile);
    CVarDirTree t(db);
    
    const char* path1="/this/that/the/other";
    
    t.mkdir(path1);
    t.cd("/this");
    CPPUNIT_ASSERT_THROW(t.cd("../.."), CVarDirTree::CException);
}

// Make a directory on a relative path:

void DirTests::mkdirRelative()
{
    CVariableDb db(m_tempFile);
    CVarDirTree t(db);
    
    const char* path1="/this/that/the/other";
    t.mkdir(path1);
    t.cd("/this/that");
    t.mkdir("another");
    
    CVarDirTree::DirInfo info = findDir(db, "/this/that/another");
    ASSERT(info.s_id != -1);
    ASSERT(info.s_parentId != -1);
}

// Make a directory on a relative path using ..:

void DirTests::mkdirDotDot()
{
    CVariableDb db(m_tempFile);
    CVarDirTree t(db);
    
    const char* path1="/this/that/the/other";
    t.mkdir(path1);
    t.cd("/this/that");
    t.mkdir("../the");

    CVarDirTree::DirInfo info = findDir(db, "/this/the");
    ASSERT(info.s_id != -1);
    ASSERT(info.s_parentId != -1);
    
}
// test ls when there's nothing in the directory.

void DirTests::lsEmpty()
{
    CVariableDb db(m_tempFile);
    CVarDirTree t(db);

    EQ(static_cast<size_t>(0), t.ls().size());    
}

void DirTests::lsNotEmpty()
{
    CVariableDb db(m_tempFile);
    CVarDirTree t(db);

    t.mkdir("aTest");
    t.mkdir("testing");
    t.mkdir("thetest");
    
    std::vector<CVarDirTree::DirInfo> listing = t.ls();
    EQ(static_cast<size_t>(3), listing.size());
    
    // The data are specified to come in alpha order:
    
    EQ(std::string("aTest"), listing[0].s_name);
    EQ(std::string("testing"), listing[1].s_name);
    EQ(std::string("thetest"), listing[2].s_name);
}

void DirTests::rmEmptyDir()
{
    CVariableDb db(m_tempFile);
    CVarDirTree t(db);

    t.mkdir("aTest");
    t.mkdir("testing");
    t.mkdir("thetest");
    
    t.rmdir("testing");
    std::vector<CVarDirTree::DirInfo> listing = t.ls();
    EQ(static_cast<size_t>(2), listing.size());
    EQ(std::string("aTest"), listing[0].s_name);
    EQ(std::string("thetest"), listing[1].s_name);
        
}


void DirTests::rmNonEmptyDir()
{
    CVariableDb db(m_tempFile);
    CVarDirTree t(db);
    
    t.mkdir("/this/that/the/other");
    
    CPPUNIT_ASSERT_THROW(t.rmdir("/this/that"), CVarDirTree::CException);
}


void DirTests::rmNoSuchDir()
{
    CVariableDb db(m_tempFile);
    CVarDirTree t(db);

    CPPUNIT_ASSERT_THROW(t.rmdir("/this"), CVarDirTree::CException);
}

void DirTests::rmRoot()
{
    CVariableDb db(m_tempFile);
    CVarDirTree t(db);
    
    CPPUNIT_ASSERT_THROW(t.rmdir("/"), CVarDirTree::CException);
}

// Construct a dirtree with the default directory set by its id:

void DirTests::constructById()
{
    const char* dirname="/a/b/c/d";
    CVariableDb db(m_tempFile);
    CVarDirTree creator(db);
    creator.mkdir(dirname);
    creator.cd(dirname);
    
    
    CVarDirTree testDir(db, creator.getwd().s_id);
    
    EQ(std::string(dirname), testDir.wdPath());
    
}

//  Construction using an id for a directory that does not exist
// should be a failure.

void DirTests::constructByBadId()
{
    CVariableDb db(m_tempFile);
    
    CPPUNIT_ASSERT_THROW(
        CVarDirTree(db, 1234),
        CVarDirTree::CException
    );
}
// Construct with root id should work just fine too:

void DirTests::constructByRootId()
{
    CVariableDb db(m_tempFile);
    CVarDirTree root(db, findRoot(db));
    
    EQ(findRoot(db), root.getwd().s_id);
}

/*--------------------------------------------------------------------------
 * Utilities:
 */

int DirTests::findRoot(CVariableDb& db)
{
    CSqliteStatement sFindRoot(
        db,
        "SELECT id FROM directory WHERE name IS null and parent_id IS null"
    );
    ++sFindRoot;
    int rootId = sFindRoot.getInt(0);
    
    return rootId;    
}

// Get info about the bottom directory of a path.
// If not found, the struct will have an empty string and both ids will be -1.

CVarDirTree::DirInfo
DirTests::findDir(CVariableDb& db, const char* path)
{
    CVarDirTree::DirInfo result;
    int parentId = findRoot(db);
    CSqliteStatement getNextEle(
        db,
        "SELECT id ,parent_id FROM directory WHERE name = :name AND parent_id = :parent"
    );
    
    std::vector<std::string> pathVec = CVarDirTree::parsePath(path);
    for (int i = 0; i < pathVec.size(); i++) {
        getNextEle.bind(1, pathVec[i].c_str(), -1, SQLITE_TRANSIENT);
        getNextEle.bind(2, parentId);
        ++getNextEle;
        if (getNextEle.atEnd()) {
            return {"", -1, -1};
        } else {
            result.s_name = pathVec[i];
            parentId      = result.s_id   = getNextEle.getInt(0);
            result.s_parentId = getNextEle.getInt(1);     // Prep for next iteration
        }
        
        getNextEle.reset();              
    }
    return result;
}


