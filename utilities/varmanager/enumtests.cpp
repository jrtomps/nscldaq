// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include <string.h>
#include <stdexcept>
#include <errno.h>
#include <unistd.h>
#include <vector>
#include <set>
#include <utility>




#include "CVariableDb.h"
#include <CSqliteStatement.h>
#include "CEnumType.h"
#include "CEnumeration.h"


class EnumTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(EnumTests);
  CPPUNIT_TEST(schemaExists);
  
  // Tests for specific Enum types.
  
  CPPUNIT_TEST(legalValue);
  CPPUNIT_TEST(illegalValue);
  CPPUNIT_TEST(defaultValue);

  // Tests for CEnumeration  - Creation
  
  CPPUNIT_TEST(newOk);
  CPPUNIT_TEST(newDuplicateType);
  CPPUNIT_TEST(newDuplicateValue);
  CPPUNIT_TEST(newEmptyValueSet);

  // Tests for CEnumeration - find enum
  
  CPPUNIT_TEST(findOk);
  CPPUNIT_TEST(findNoSuch);
  CPPUNIT_TEST(findNotEnum);
  
  // Tests for CEnumeration  - added values
  
  CPPUNIT_TEST(addValueOk);
  CPPUNIT_TEST(addNoSuchEnum);
  CPPUNIT_TEST(addValueDup);
  
  // Tests for CEnumeration  - list values
  
  //CPPUNIT_TEST(listValues);
  //CPPUNIT_TEST(listNoSuchEnum);
  
  // Tests for CEnumeration  - list enums.

  //CPPUNIT_TEST(listEnums);
  
  
  CPPUNIT_TEST_SUITE_END();
  
  

private:
    char m_tempFile[100];
    int  m_fd;
    CVariableDb* m_db;
public:
  void setUp() {
    strcpy(m_tempFile, "testvardbXXXXXX");
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
  void schemaExists();
  void legalValue();
  void illegalValue();
  void defaultValue();
  
  void newOk();
  void newDuplicateType();
  void newDuplicateValue();
  void newEmptyValueSet();
  
  void findOk();
  void findNoSuch();
  void findNotEnum();
  
  void addValueOk();
  void addNoSuchEnum();
  void addValueDup();
private:
    int createEnum(const char* name, const char** pValues);
    std::pair<int, std::set<std::string> > getEnumInfo(const char* typeName);
};

CPPUNIT_TEST_SUITE_REGISTRATION(EnumTests);

/*-----------------------------------------------------------------------------
 * utility methods
 */

// Create an enum:

int
EnumTests::createEnum(const char* name, const char** pValues)
{
    //  Create the type:
    
    
    CSqliteStatement typeCreate(
        *m_db,
        "INSERT INTO variable_types (type_name) VALUES (?)"
    );
    typeCreate.bind(1, name, -1, SQLITE_TRANSIENT);
    ++typeCreate;
    int typeId = typeCreate.lastInsertId();
    
    // Add the values:
    
    CSqliteStatement valueAdd(
        *m_db,
        "INSERT INTO enumerated_values (type_id, value) \
            VALUES (?,?)"
    );
    while (*pValues) {
        valueAdd.bind(1, typeId);
        valueAdd.bind(2, *pValues, -1, SQLITE_TRANSIENT);
        ++valueAdd;
        valueAdd.reset();
        pValues++;
    }
    
    return typeId;
}

// Get information about an enumerated type.
// first is the id of the type or -1 if there isn't one.
// second is the set of legal values.
//
std::pair<int, std::set<std::string> >
EnumTests::getEnumInfo(const char* typeName)
{
    std::pair<int, std::set<std::string> > result;
    result.first = -1;                     // Assume there's no match.
    
    CSqliteStatement enumLister(
        *m_db,
        "SELECT t.id, e.value                                 \
           FROM enumerated_values e                           \
           INNER JOIN variable_types t ON t.id = e.type_id    \
           WHERE t.type_name = ?                              "
    );
    enumLister.bind(1, typeName, -1, SQLITE_TRANSIENT);
    while (! (++enumLister).atEnd()) {
        result.first = enumLister.getInt(0);
        result.second.insert(
            std::string(reinterpret_cast<const char*>(enumLister.getText(1))));
    }
    
    return result;
}

/*-------------------------------------------------------------------------*/
/* Tests
 */

// Creation of the database should make the table
// 'enumerated_values'  Note CVariableDb is subclassed from CSqlite:

void EnumTests::schemaExists() {
    CSqliteStatement exists(
        *m_db,
        "SELECT COUNT(*) FROM sqlite_master    \
           WHERE name = 'enumerated_values'    \
           AND   type = 'table'                \
        "
    );
    ++exists;
    EQ(1, exists.getInt(0));
}

// Check that legal values are legal:

void EnumTests::legalValue()
{
    const char* values[] = {"one", "two", "three", 0};
    int enumId = createEnum("testenum", values);
    CEnumType type(enumId, "testenum", *m_db);
    
    // Check that all legal values pass:
    
    const char** pTestValue = values;
    while (*pTestValue) {
        ASSERT(type.legal(*pTestValue));
        pTestValue++;
    }
    
}

// Check that an illegal value is bad:

void EnumTests::illegalValue()
{
    const char* values[] = {"one", "two", "three", 0};
    int enumId = createEnum("testenum", values);
    CEnumType type(enumId, "testenum", *m_db);
    
    ASSERT(!type.legal("four"));
}
    
// The default value should be the first enum in the set:

void EnumTests::defaultValue()
{
    const char* values[] = {"one", "two", "three", 0};
    int enumId = createEnum("testenum", values);
    CEnumType type(enumId, "testenum", *m_db);
    
    EQ(std::string("one"), type.defaultValue());
}
//  CEnumeration::create should
//  - Create an entry the variable_types table with the new type name.
//  - return the id of that entry.
//  - create entries in the enumeration_values table for each and only for each
//    value in the types.

void EnumTests::newOk()
{
    const char* values[] = {"first", "second", "third", "last", 0};
    std::vector<std::string> vValues;
    const char** pValues = values;
    while (*pValues) {
        vValues.push_back(std::string(*pValues));
        pValues++;
    }
    
    int typeId = CEnumeration::create(*m_db, "myenum", vValues);
    
    std::pair<int, std::set<std::string> > enumInfo = getEnumInfo("myenum");
    ASSERT(enumInfo.first > 0);             // -1 if no such enum.
    EQ(enumInfo.first, typeId);
    EQ(vValues.size(), enumInfo.second.size());
    
    for (int i = 0; i < vValues.size(); i++) {
        ASSERT(enumInfo.second.find(vValues[i]) != enumInfo.second.end());
    }
}


// Creation of a duplicate type should throw an exception:

void EnumTests::newDuplicateType()
{
    const char* values[] = {"first", "second", "third", "last", 0};
    std::vector<std::string> vValues;
    const char** pValues = values;
    while (*pValues) {
        vValues.push_back(std::string(*pValues));
        pValues++;
    }
    
    int typeId = CEnumeration::create(*m_db, "myenum", vValues);   // ok.
    
    CPPUNIT_ASSERT_THROW(
        CEnumeration::create(*m_db, "myenum", vValues),
        CEnumeration::CException
    );
}
// Create with a duplicate possible value is an error.

void EnumTests::newDuplicateValue()
{
    const char* values[] = {"first", "second", "third", "last", "first", 0};
    std::vector<std::string> vValues;
    const char** pValues = values;
    while (*pValues) {
        vValues.push_back(std::string(*pValues));
        pValues++;
    }
    CPPUNIT_ASSERT_THROW(
        CEnumeration::create(*m_db, "myenum", vValues),
        CEnumeration::CException
    );
}

//  Enums must have a non empty value set:

void EnumTests::newEmptyValueSet()
{
    std::vector<std::string> vValues;
    CPPUNIT_ASSERT_THROW(
        CEnumeration::create(*m_db, "myenum", vValues),
        CEnumeration::CException
    );
}

// id returns the correct id for a type:

void EnumTests::findOk()
{
    const char* values[] = {"first", "second", "third", "last", 0};
    std::vector<std::string> vValues;
    const char** pValues = values;
    while (*pValues) {
        vValues.push_back(std::string(*pValues));
        pValues++;
    }
    
    int typeId = CEnumeration::create(*m_db, "myenum", vValues);   // ok.
    
    EQ(typeId, CEnumeration::id(*m_db, "myenum"));
}

// Find when there's no such type is a well defined exception:

void EnumTests::findNoSuch()
{
    CPPUNIT_ASSERT_THROW(
        CEnumeration::id(*m_db, "myenum"),
        CEnumeration::CException
    );
}
// If there's a type that's not an enum that's an exception too:

void EnumTests::findNotEnum()
{
    CPPUNIT_ASSERT_THROW(
        CEnumeration::id(*m_db, "integer"),
        CEnumeration::CException
    );
}

// Add a nonexistent value to an enum adds it:

void EnumTests::addValueOk()
{
    const char* values[] = {"first", "second", "third", "last", 0};
    std::vector<std::string> vValues;
    const char** pValues = values;
    while (*pValues) {
        vValues.push_back(std::string(*pValues));
        pValues++;
    }
    
    int typeId = CEnumeration::create(*m_db, "myenum", vValues);   // ok.
    
    CEnumeration::addValue(*m_db, "myenum", "onemore");
    
    std::pair<int, std::set<std::string> > info = getEnumInfo("myenum");
    
    ASSERT(info.second.find("onemore") != info.second.end());
}

// Add value to a nonexisting enum is an error:

void EnumTests::addNoSuchEnum()
{
    CPPUNIT_ASSERT_THROW(
        CEnumeration::addValue(*m_db, "myenum", "onemore"),
        CEnumeration::CException
    );
}
// Adding a duplicate value is an error:

void EnumTests::addValueDup()
{
    const char* values[] = {"first", "second", "third", "last", 0};
    std::vector<std::string> vValues;
    const char** pValues = values;
    while (*pValues) {
        vValues.push_back(std::string(*pValues));
        pValues++;
    }
    
    int typeId = CEnumeration::create(*m_db, "myenum", vValues);   // ok.
    
    CPPUNIT_ASSERT_THROW(
        CEnumeration::addValue(*m_db, "myenum", "first"),
        CEnumeration::CException
    );
}