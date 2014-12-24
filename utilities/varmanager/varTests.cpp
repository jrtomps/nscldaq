// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include <CSqliteStatement.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <stdexcept>

#include "CVariableDb.h"
#include "CVarDirTree.h"
#include "CVariable.h"

class VarTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(VarTests);
  CPPUNIT_TEST(typeSchema);
  CPPUNIT_TEST(typeContents);
  
  CPPUNIT_TEST(constraintTypesSchema);
  CPPUNIT_TEST(constraintTypesContents);
  
  CPPUNIT_TEST(constraintAppSchema);
  CPPUNIT_TEST(rangeAllowed);
  CPPUNIT_TEST(rangeDataSchema);
  
  CPPUNIT_TEST(variableSchema);
  
  // Creation tests method with no cwd provided.
  // note all creational tests in this clump ignore the returned  object.
  
  CPPUNIT_TEST(create1InRootInteger);
  CPPUNIT_TEST(create1InRootReal);
  CPPUNIT_TEST(create1InRootString);
  CPPUNIT_TEST(create1InRootBadType);
  CPPUNIT_TEST(create1InRootIntegerDefaultValue);
  CPPUNIT_TEST(create1InRootRealDefaultValue);
  CPPUNIT_TEST(create1InRootStringDefaultValue);
  CPPUNIT_TEST(create1InRootIntegerGoodValue);
  CPPUNIT_TEST(create1InRootIntegerBadValue);
  CPPUNIT_TEST(create1InRootRealGoodValue);
  CPPUNIT_TEST(create1InRootRealBadValue);
  CPPUNIT_TEST(create1InRootStringOk);
  CPPUNIT_TEST(create1InSubdir);
  CPPUNIT_TEST(create1InBadSubdir);
  CPPUNIT_TEST(create1DupPath);
  
  CPPUNIT_TEST(createInWd);
  
  // Construct by id:
  
  CPPUNIT_TEST(constructByIdOk);
  CPPUNIT_TEST(createConstructs);
  CPPUNIT_TEST(constructByIdFail);

  // Create relative to a directory:
  
  CPPUNIT_TEST(createRelAbsPath);
  CPPUNIT_TEST(createRelRelPath);
  CPPUNIT_TEST(createRelConstructs);

  // Construct with abs path
  
  CPPUNIT_TEST(constructAbsPathOk);
  CPPUNIT_TEST(constructAbsPathFail);
  CPPUNIT_TEST(constructRelPathOk);
  CPPUNIT_TEST(constructRelPathFail);

  // Tests of set:
  
  CPPUNIT_TEST(setIntOk);
  CPPUNIT_TEST(setIntBad);
  CPPUNIT_TEST(setRealOk);
  CPPUNIT_TEST(setRealBad);
  CPPUNIT_TEST(setStringOk);   // All strings are legal.
  
  // Tests of getters:
  
  CPPUNIT_TEST(id);
  CPPUNIT_TEST(name);
  CPPUNIT_TEST(dirpath);
  
  // Test of the listing method
  
  CPPUNIT_TEST(listEmpty);
  CPPUNIT_TEST(listOne);
  CPPUNIT_TEST(listAFew);
  CPPUNIT_TEST(listWithPath);

  //  Destroy variables:
  
  CPPUNIT_TEST(destroyByIdOk);
  CPPUNIT_TEST(destroyByBadId);
  CPPUNIT_TEST(destroyByObj);
  CPPUNIT_TEST(destroyByRelPath);
  CPPUNIT_TEST(destroyByAbsPath);
  
  CPPUNIT_TEST_SUITE_END();
  
  


private:
    int          m_fd;
    char         m_tempFile[100];
    CVariableDb* m_db;

public:
  void setUp() {
    strcpy(m_tempFile, "vardbXXXXXX");
    m_fd = mkstemp(m_tempFile);
    if(m_fd == -1) {
        throw std::runtime_error(strerror(errno));
    }
    CVariableDb::create(m_tempFile);
    m_db = new CVariableDb(m_tempFile);
  }
  void tearDown() {
    delete m_db;
    close(m_fd);
    unlink(m_tempFile);
  }
protected:
    void typeSchema();
    void typeContents();

    
    void constraintTypesSchema();
    void constraintTypesContents();
    
    void constraintAppSchema();
    void rangeAllowed();
    void rangeDataSchema();
    
    void variableSchema();
    
    void create1InRootInteger();
    void create1InRootReal();
    void create1InRootString();
    void create1InRootBadType();
    void create1InRootIntegerDefaultValue();
    void create1InRootRealDefaultValue();
    void create1InRootStringDefaultValue();
    void create1InRootIntegerGoodValue();
    void create1InRootIntegerBadValue();
    void create1InRootRealGoodValue();
    void create1InRootRealBadValue();
    void create1InRootStringOk();

    
    void create1InSubdir();
    void create1InBadSubdir();
    void create1DupPath();

    void createInWd();
    
    void constructByIdOk();
    void createConstructs();
    void constructByIdFail();
    
    void createRelAbsPath();
    void createRelRelPath();
    void createRelConstructs();
    
    void constructAbsPathOk();
    void constructAbsPathFail();
    void constructRelPathOk();
    void constructRelPathFail();
    
    void setIntOk();
    void setIntBad();
    void setRealOk();
    void setRealBad();
    void setStringOk();
    
    void id();
    void name();
    void dirpath();
    
    void listEmpty();
    void listOne();
    void listAFew();
    void listWithPath();
    
    void destroyByIdOk();
    void destroyByBadId();
    void destroyByObj();
    void destroyByRelPath();
    void destroyByAbsPath();
    
private:
    bool haveTable(const char* name);
    bool constraintOk(const char* constraint, const char* type);
    int  varid(const char* dirpath, const char* name);
    bool varExists(const char* dirpath, const char* name);
    std::string getValue(const char* dirpath, const char* name);
    int getTypeId(const char* type);
};

CPPUNIT_TEST_SUITE_REGISTRATION(VarTests);


// True if the table 'name' exists.

bool
VarTests::haveTable(const char* name)
{
    CSqliteStatement stmt(
        *m_db,
        "SELECT COUNT(*) AS c FROM sqlite_master \
            WHERE type ='table' AND name=? \
        "
    );
    stmt.bind(1, name, -1, SQLITE_TRANSIENT);
    ++stmt;
    
    return 1 == stmt.getInt(0);
}
// True if the 'constraint' is allowed for 'type'.

bool
VarTests::constraintOk(const char* constraint, const char* type)
{
    CSqliteStatement stmt(
        *m_db,
        "SELECT COUNT(*) FROM constraint_types ct                        \
            INNER JOIN constraint_allowed ca ON ca.constraint_id = ct.id \
            INNER JOIN variable_types vt ON vt.id = ca.type_id           \
            WHERE ct.constraint_name = ?                                 \
            AND   vt.type_name = ?                                       \
        "
    );
    stmt.bind(1, constraint, -1, SQLITE_TRANSIENT);
    stmt.bind(2, type,       -1, SQLITE_TRANSIENT);
    ++stmt;
    
    return stmt.getInt(0) > 0;
    
}
// Returns the variable's id -1 if does not exist:

int
VarTests::varid(const char* dirpath, const char* name)
{
    CVarDirTree dir (*m_db);
    dir.cd(dirpath);
    
    CVarDirTree::DirInfo info = dir.getwd();
    int dirid = info.s_id;
    
    CSqliteStatement find(
         *m_db,
         "SELECT id FROM variables WHERE directory_id = ? and name = ?"
    );
    find.bind(1, dirid);
    find.bind(2, name, -1, SQLITE_TRANSIENT);
    
    ++find;
    if (find.atEnd()) {
        return -1;         
    }
    return find.getInt(0);
}
// True if the variable 'name' exists in the 'dirpath' directory

bool
VarTests::varExists(const char* dirpath, const char* name)
{
    int id = varid(dirpath, name);
    return id != -1;
}

std::string
VarTests::getValue(const char* dirpath, const char* name)
{
    CVarDirTree dir(*m_db);
    dir.cd(dirpath);
    CVarDirTree::DirInfo info = dir.getwd();
    int dirid = info.s_id;
    
    CSqliteStatement find(
        *m_db,
        "SELECT v.value AS value FROM variables v \
           WHERE name = ? AND directory_id = ?"
    );
    find.bind(1, name, -1, SQLITE_TRANSIENT);
    find.bind(2, dirid);
    ++find;
    
    std::string result(reinterpret_cast<const char*>(find.getText(0)));
    return result;
}


// Return the type id associated with a typename.
// -1 if no such type.

int
VarTests::getTypeId(const char* type)
{
    CSqliteStatement s(
        *m_db,
        "SELECT id FROM variable_types WHERE type_name = ?"
    );
    s.bind(1, type, -1, SQLITE_TRANSIENT);
    ++s;
    if (s.atEnd()) {
        return -1;
    } else {
        return s.getInt(0);
    }
}
/*----------------------------------------------------------------------------
 * Tests
 */

//Check for the variable_types table:

void VarTests::typeSchema() {
    
    ASSERT(haveTable("variable_types"));
        
    
}
// Make sure all type defs are present and accounted for:

void VarTests::typeContents() {
    CSqliteStatement stmt(
        *m_db,
        "SELECT type_name FROM variable_types ORDER BY type_name ASC"
    );
    ++stmt;
    EQ(0, strcmp("integer", reinterpret_cast<const char*>(stmt.getText(0))));
    ++stmt;
    EQ(0, strcmp("real", reinterpret_cast<const char*>(stmt.getText(0))));
    ++stmt;
    EQ(0, strcmp("string", reinterpret_cast<const char*>(stmt.getText(0))));
    
    
    
    ++stmt;
    ASSERT(stmt.atEnd());
    
}

// Ensure the existence of constraint_types

void VarTests::constraintTypesSchema()
{
    ASSERT(haveTable("constraint_types"));
}

void VarTests::constraintTypesContents()
{
    CSqliteStatement stmt(
        *m_db,
        "SELECT constraint_name \
            FROM constraint_types ORDER BY constraint_name ASC"
    );
    
    ++stmt;
    ASSERT(!stmt.atEnd());
    EQ(0, strcmp("range", reinterpret_cast<const char*>(stmt.getText(0))));
    
    ++stmt;
    ASSERT(stmt.atEnd());
}
// Ensure the constraint application join table is present:

void VarTests::constraintAppSchema()
{
    ASSERT(haveTable("constraint_allowed"));
}

// Ensure the range constraint is allowed on the right types:

void VarTests::rangeAllowed()
{
    ASSERT(constraintOk("range", "integer"));
    ASSERT(constraintOk("range", "real"));
}

// Ensure the range constraint data table is present:

void VarTests::rangeDataSchema()
{
    ASSERT(haveTable("range_constraint_data"));
}

// ensure the variables table exists:

void VarTests::variableSchema()
{
    ASSERT(haveTable("variables"));
    ASSERT(haveTable("constraints"));
}

// Create a xxx in the root directory without providing a cwd object.

void VarTests::create1InRootInteger()
{
    CVariable* pVariable = CVariable::create(*m_db, "/myvar", "integer");
    delete pVariable;
    
    ASSERT(varExists("/", "myvar"));
}

void VarTests::create1InRootReal()
{
    CVariable* pVariable = CVariable::create(*m_db, "/myvar", "real");
    delete pVariable;
    
    ASSERT(varExists("/", "myvar"));
        
}
void VarTests::create1InRootString()
{
    CVariable* pVariable = CVariable::create(*m_db, "/myvar", "string");
    delete pVariable;
    
    ASSERT(varExists("/", "myvar"));
    
}
void VarTests::create1InRootBadType()
{
    CPPUNIT_ASSERT_THROW(
        CVariable::create(*m_db, "/myvar", "invalid type"),
        CVariable::CException
    );
}
// Default value checking: integer: 0, real 0.0 string ""

void VarTests::create1InRootIntegerDefaultValue()
{
    CVariable* pVariable = CVariable::create(*m_db, "/myvar", "integer");
    delete pVariable;
    
    EQ(std::string("0"), getValue("/", "myvar"));
}
void VarTests::create1InRootRealDefaultValue()
{
    CVariable* pVariable = CVariable::create(*m_db, "/myvar", "real");
    delete pVariable;
    
    EQ(std::string("0.0"), getValue("/", "myvar"));
}
void VarTests::create1InRootStringDefaultValue()
{
    CVariable* pVariable = CVariable::create(*m_db, "/myvar", "string");
    delete pVariable;
    
    EQ(std::string(""), getValue("/", "myvar"));
    
}

// Creating with non-default values good and bad

void VarTests::create1InRootIntegerGoodValue()
{
    CVariable* pVariable = CVariable::create(*m_db, "/myvar", "integer", "1234");
    delete pVariable;
    
    EQ(std::string("1234"), getValue("/", "myvar"));
}
void VarTests::create1InRootIntegerBadValue()
{
    CPPUNIT_ASSERT_THROW(
        CVariable::create(*m_db, "/myvar", "integer", "not an integer string"),
        CVariable::CException
    );
}

void VarTests::create1InRootRealGoodValue()
{
    CVariable* pVariable = CVariable::create(*m_db, "/myvar", "integer", "3.1416");
    delete pVariable;
    
    EQ(std::string("3.1416"), getValue("/", "myvar"));
}

void VarTests::create1InRootRealBadValue()
{
    CPPUNIT_ASSERT_THROW(
        CVariable::create(*m_db, "/myvar", "real", "not a real value"),
        CVariable::CException
    );
}

void VarTests::create1InRootStringOk()
{
    const char* s = "There are only legal string values";
    CVariable* pVariable = CVariable::create(*m_db, "/myvar", "string", s);
    delete pVariable;
    
    EQ(std::string(s), getValue("/", "myvar"));
}

// Subdirectories:

void VarTests::create1InSubdir()
{
    const char* dir="/a/sub/dir";
    std::string name = dir;
    name += "/myvar";
    CVarDirTree d(*m_db);
    d.mkdir(dir);
    
    CVariable* pVariable = CVariable::create(
        *m_db, name.c_str(), "integer", "1234"
    );
    delete pVariable;
    
    EQ(std::string("1234"), getValue(dir, "myvar"));
}

void VarTests::create1InBadSubdir()
{
    CPPUNIT_ASSERT_THROW(
        CVariable::create(*m_db, "/this/sub/directory/myvar", "integer"),
        std::runtime_error
    );
}
// Duplicate variable name creation is a sin:

void VarTests::create1DupPath()
{
    CVariable* pVariable = CVariable::create(*m_db, "/myvar", "string", "test");
    delete pVariable;
    CPPUNIT_ASSERT_THROW(
        CVariable::create(*m_db, "/myvar", "string", "test"),
        CVariable::CException
    );
}

// Create in the current working directory:

void VarTests::createInWd()
{
    const char* dir = "/test";
    CVarDirTree d(*m_db);
    d.mkdir(dir);
    d.cd(dir);
    
    CVariable::create(*m_db, d, "myvar", "integer", 0);
    EQ(std::string("0"), getValue(dir, "myvar"));
}

// Construction by id:

void VarTests::constructByIdOk()
{
    CVariable *pVar = CVariable::create(*m_db, "/myvar", "integer");
    delete pVar;
    
    int id = varid("/", "myvar");
    CVariable variable(*m_db, id);
    
    EQ(std::string("0"), variable.get());
}
// A creation produces a variable that is faithful to what was created:

void VarTests::createConstructs()
{
    CVariable *pVar = CVariable::create(*m_db, "/myvar", "integer", "124");
    ASSERT(pVar);
    EQ(std::string("124"), pVar->get());
    
    delete pVar;
}

// Throw an exception construct on a bad id:

void VarTests::constructByIdFail()
{
    CPPUNIT_ASSERT_THROW(
        CVariable(*m_db, 1234),
        CVariable::CException
    );
}
// Create with dirtree object but absolute path is absolute:

void VarTests::createRelAbsPath()
{
    CVarDirTree d(*m_db);
    d.mkdir("/test");
    
    CVariable* pVar = CVariable::create(*m_db, d, "/test/myvar", "integer");
    delete pVar;
    
    ASSERT(varExists("/test", "myvar"));
    EQ(std::string("0"), getValue("/test", "myvar"));
}

// Create with a dirtree object with a relative path:

void VarTests::createRelRelPath()
{
    CVarDirTree d(*m_db);
    d.mkdir("/test");
    d.mkdir("/testing");
    d.cd("/test");
    
    CVariable* pVar = CVariable::create(*m_db, d, "../testing/myvar", "integer");
    delete pVar;
    ASSERT(varExists("/testing", "myvar"));
    EQ(std::string("0"), getValue("/testing", "myvar"));
}

// Create with dirtree returns an object for the var

void VarTests::createRelConstructs()
{
    CVarDirTree d(*m_db);
    d.mkdir("/test");
    d.mkdir("/testing");
    d.cd("/test");
    
    CVariable* pVar = CVariable::create(*m_db, d, "../testing/myvar", "integer");
    ASSERT(pVar);
    
    EQ(std::string("0"), pVar->get());
    delete pVar;
}

// Construct with absolute path (no dirtree) ok

void VarTests::constructAbsPathOk()
{
    CVarDirTree d(*m_db);
    d.mkdir("/test");
    CVariable* pVar = CVariable::create(*m_db, "/test/myvar", "integer");
    delete pVar;
    
    CVariable var(*m_db, "/test/myvar");
    
    EQ(std::string("0"), var.get());
}
// If abspath does not exist, creation must faile:

void VarTests::constructAbsPathFail()
{
    CVarDirTree d(*m_db);
    CPPUNIT_ASSERT_THROW(
        CVariable(*m_db, "/test/myvar"), CVariable::CException
    );
    
}

// Relative path creates should work too:

void VarTests::constructRelPathOk()
{
    const char* dir = "/test";
    const char* varname = "myvar";
    std::string varabspath = dir;
    varabspath += CVarDirTree::m_pathSeparator;
    varabspath += varname;
    
    CVarDirTree d(*m_db);
    d.mkdir(dir);
    CVariable* pVar = CVariable::create(*m_db, varabspath.c_str(), "integer");
    delete pVar;
    
    d.cd(dir);
    
    CVariable var(*m_db, d, varname);
    
    EQ(std::string("0"), var.get());   
}

// Construct in relative path that does not exist will fail

void VarTests::constructRelPathFail()
{
    const char* dir="/test";
    CVarDirTree d(*m_db);
    d.mkdir(dir);
    d.cd(dir);
    
    CPPUNIT_ASSERT_THROW(
        CVariable(*m_db, d, "myvar"), CVariable::CException
    );
}

// Set should modify the variable value:

void VarTests::setIntOk()
{
    CVariable* pVar = CVariable::create(*m_db, "/myvar", "integer");
    
    pVar->set("1234");
    EQ(std::string("1234"), pVar->get());
    delete pVar;
}
// Set for an integer should fail with non integer value:

void VarTests::setIntBad()
{
    CVariable* pVar = CVariable::create(*m_db, "/myvar", "integer");
    CPPUNIT_ASSERT_THROW(
        pVar->set("abcde"),
        CVariable::CException
    );
    delete pVar;
}

void VarTests::setRealOk()
{
    CVariable* pVar = CVariable::create(*m_db, "/myvar", "real");
    
    pVar->set("1234.5678");
    EQ(std::string("1234.5678"), pVar->get());
    delete pVar;
}
void VarTests::setRealBad()
{
    CVariable* pVar = CVariable::create(*m_db, "/myvar", "real");
    CPPUNIT_ASSERT_THROW(
        pVar->set("abcde"),
        CVariable::CException
    );
    delete pVar;
}

void VarTests::setStringOk()
{
    CVariable* pVar = CVariable::create(*m_db, "/myvar", "string");
    
    pVar->set("hello world.");
    EQ(std::string("hello world."), pVar->get());
    delete pVar;
}

// id returns the id of the variable:

void VarTests::id()
{
    CVariable* pVar = CVariable::create(*m_db, "/myvar", "string");
    EQ(varid("/","myvar"), pVar->getId());
    
    delete pVar;
}

// name -returns the name of the variable (minus the path).

void VarTests::name()
{
    CVarDirTree d(*m_db);
    d.mkdir("/a/b/c/d");
    CVariable* pVar = CVariable::create(*m_db, "/a/b/c/d/myvar", "string");
    EQ(std::string("myvar"), pVar->getName());
    
    delete pVar;
}

// dirpath - returns the directory part of the variable:

void VarTests::dirpath()
{
    const char* dirpath="/a/b/c/d";
    std::string varpath = dirpath;
    varpath += CVarDirTree::m_pathSeparator;
    varpath += "myvar";
    
    CVarDirTree d(*m_db);
    d.mkdir(dirpath);
    CVariable* pVar = CVariable::create(*m_db, varpath.c_str(), "string");
    
    EQ(std::string(dirpath), pVar->getDirectory());
    
    delete pVar;
}
// listEmpty
//    Listing the directories in an empty directory give and empty vector.

void VarTests::listEmpty()
{
    CVarDirTree root(*m_db);
    std::vector<CVariable::VarInfo> result = CVariable::list(m_db, root);
    
    EQ(static_cast<size_t>(0), result.size());
}
// list directory with a single variable:
void VarTests::listOne()
{
    CVarDirTree root(*m_db);
    CVariable* pVar = CVariable::create(*m_db, "/variable", "integer");
    
    std::vector<CVariable::VarInfo> info = CVariable::list(m_db, root);
    
    EQ(static_cast<size_t>(1), info.size());
    
    CVariable::VarInfo item = info[0];
    
    EQ(pVar->getId(), item.s_id);
    EQ(pVar->getName(), item.s_name);
    EQ(std::string("integer"), item.s_type);
    EQ(getTypeId("integer"), item.s_typeId);
    EQ(root.getwd().s_id, item.s_dirId);
}

// List a directory with several vars.

void VarTests::listAFew()
{
    const char* dirPath("/a/b/c");
    const char* varnames[] = {
      "a", "b", "c", "d",    // Must be alpha sorted.
      0
    };
    std::vector<CVariable*> vars;
    
    CVarDirTree adir(*m_db);
    adir.mkdir(dirPath);
    adir.cd(dirPath);
    
    const char** pName = varnames;
    while(*pName) {
        vars.push_back(CVariable::create(*m_db, adir, *pName, "real"));
        pName++;
    }
    
    std::vector<CVariable::VarInfo> listing = CVariable::list(m_db, adir);
    
    EQ(vars.size(), listing.size());
    for(int i = 0; i < listing.size(); i++) {
        CVariable* pVar         = vars[i];
        CVariable::VarInfo item = listing[i];
 
        EQ(pVar->getId(), item.s_id);
        EQ(pVar->getName(), item.s_name);
        EQ(std::string("real"), item.s_type);
        EQ(getTypeId("real"), item.s_typeId);
        EQ(adir.getwd().s_id, item.s_dirId);       
    
        delete pVar;        
    }
    
    
}

// Can use a supplementary path in the list method:

void VarTests::listWithPath()
{
    const char* dirPath("/a/b/c");
    const char* varnames[] = {
      "a", "b", "c", "d",    // Must be alpha sorted.
      0
    };
    std::vector<CVariable*> vars;
    
    CVarDirTree adir(*m_db);
    adir.mkdir(dirPath);
    adir.cd(dirPath);
   
    const char** pName = varnames;
    while(*pName) {
        vars.push_back(CVariable::create(*m_db, adir, *pName, "real"));
        pName++;
    }
    int dirId = adir.getwd().s_id;             // Save the directory of the vars:
    
    // Set the dir back to root:
    
    adir.cd("/");
    
    std::vector<CVariable::VarInfo> listing =
        CVariable::list(m_db, adir, dirPath);
    
    EQ(vars.size(), listing.size());
    for(int i = 0; i < listing.size(); i++) {
        CVariable* pVar         = vars[i];
        CVariable::VarInfo item = listing[i];
 
        EQ(pVar->getId(), item.s_id);
        EQ(pVar->getName(), item.s_name);
        EQ(std::string("real"), item.s_type);
        EQ(getTypeId("real"), item.s_typeId);
        EQ(dirId, item.s_dirId);       
    
        delete pVar;        
    }
}
// Destroy by id with valid id:

void VarTests::destroyByIdOk()
{
    CVariable* pVar = CVariable::create(*m_db, "/myvariable", "integer");
    CVariable::destroy(*m_db, pVar->getId());
    delete pVar;
    
    CVarDirTree root(*m_db);             // Rootdir:
    
    std::vector<CVariable::VarInfo> listing = CVariable::list(m_db, root);
    
    EQ(static_cast<size_t>(0), listing.size());
}
// Bad id results in an exception.

void VarTests::destroyByBadId()
{
    CPPUNIT_ASSERT_THROW(
        CVariable::destroy(*m_db, 1234),
        CVariable::CException
    );
}
// Destroy given an object.  Note that we can't really test the
// code paths for dodelete true/false.
//
void VarTests::destroyByObj()
{
    CVariable* pVar = CVariable::create(*m_db, "/mvariable", "integer");
    CVariable::destroy(pVar);
    
    ASSERT(!varExists("/", "myvariable"));
}

// Destroy given a path relative to a dir objet:

void VarTests::destroyByRelPath()
{
    const char* dirPath="/a/b/c";
    
    CVarDirTree adir(*m_db);
    adir.mkdir(dirPath);
    adir.cd(dirPath);
    
    CVariable* pVar = CVariable::create(*m_db, adir, "myvar", "integer" );
    delete pVar;
    
    // Now go up a directory or two and  delete relative to there:
    
    adir.cd("../..");
    CVariable::destroy(*m_db, adir, "b/c/myvar");
    
    ASSERT(!varExists(dirPath, "myvar"));
}
// Destroy given an abspath

void VarTests::destroyByAbsPath()
{
    const char* dirPath = "/a/b/c";
    CVarDirTree adir(*m_db);
    adir.mkdir(dirPath);
    adir.cd(dirPath);
    
    CVariable* pVar = CVariable::create(*m_db, adir, "myvar", "integer");
    delete pVar;
    
    std::string fullPath = dirPath;
    fullPath += CVarDirTree::m_pathSeparator;
    fullPath += "myvar";
    CVariable::destroy(*m_db, fullPath.c_str());
    
    ASSERT(!varExists(dirPath, "myvar"));
    
}