// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"


#include "CTypeFactory.h"
#include "CDataType.h"
#include "CIntegerType.h"
#include "CRealType.h"
#include "CStringType.h"


#include "CDataTypeCreatorBase.h"
#include "CIntegerTypeCreator.h"
#include "CRealTypeCreator.h"
#include "CStringTypeCreator.h"

#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <string>
#include <stdexcept>

#include <CSqlite.h>
#include <CSqliteStatement.h>



static const char* defaultTypes[] = {
    "integer", "real", "string",
    0
};


class TypeFactoryTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(TypeFactoryTests);
  CPPUNIT_TEST(createSchema);
  CPPUNIT_TEST(checkDefaultTypes);
  CPPUNIT_TEST(checkNoDupTypes);
  
  // Tests of integer types:
  
  CPPUNIT_TEST(baseClassId);
  CPPUNIT_TEST(baseClassType);
  CPPUNIT_TEST(intLegalOk);
  CPPUNIT_TEST(intLegalBad);
  CPPUNIT_TEST(intDefault);
  
  
  // Tests of Real types:
  
  CPPUNIT_TEST(realLegalOk);
  CPPUNIT_TEST(realLegalBad);
  CPPUNIT_TEST(readlDefault);
  
  // Tests of string types:
  
  CPPUNIT_TEST(stringLegal);
  CPPUNIT_TEST(stringDefault);
  
  // Tests of creators.
  
  // Base type creator:
  
  CPPUNIT_TEST(baseCreatorRegisters);
  CPPUNIT_TEST(baseCreatorSingleRegister);
  
  // Integer type creator:
  
  CPPUNIT_TEST(integerCreatorType);
  CPPUNIT_TEST(integerCreatorDesc);
  CPPUNIT_TEST(integerCreatorTypeName);
  
  // Real type creators
  
  CPPUNIT_TEST(realCreatorType);
  CPPUNIT_TEST(realCreatorDesc);
  CPPUNIT_TEST(realCreatorTypeName);
  
  // String type creators
  
  CPPUNIT_TEST(stringCreatorType);
  CPPUNIT_TEST(stringCreatorDesc);
  CPPUNIT_TEST(stringCreatorTypeName);
  
  // NOTE Factory creation methods are already tested by deriving
  // from the extensible factory and therefore need no testing here.
  
  CPPUNIT_TEST_SUITE_END();


private:
    char m_tempFile[100];
    int  m_fd;
    CSqlite* m_db;
public:
  void setUp() {
    strcpy(m_tempFile, "vardbXXXXXX");
    m_fd = mkstemp(m_tempFile);
    if(m_fd == -1) {
        throw std::runtime_error(strerror(errno));
    }
    m_db = new CSqlite(m_tempFile);
  }
  void tearDown() {
    
    close(m_fd);
    unlink(m_tempFile);
    
  }
protected:
  void createSchema();
  void checkDefaultTypes();
  void checkNoDupTypes();
  
  void baseClassId();
  void baseClassType();
  
  void intLegalOk();
  void intLegalBad();
  void intDefault();
  
  void realLegalOk();
  void realLegalBad();
  void readlDefault();
  
  void stringLegal();
  void stringDefault();
  
  void baseCreatorRegisters();
  void baseCreatorSingleRegister();
  
  void integerCreatorType();
  void integerCreatorDesc();
  void integerCreatorTypeName();
  
  void realCreatorType();
  void realCreatorDesc();
  void realCreatorTypeName();
  
  void stringCreatorType();
  void stringCreatorDesc();
  void stringCreatorTypeName();
  
  // Utilities
private:
    void makeSchema();
};

CPPUNIT_TEST_SUITE_REGISTRATION(TypeFactoryTests);

/*--------------------------------------------------------------------------
 * Utilties:
 */

/**
 * makeSchema (white box)
 *    Create the variable_types table as expected by the objects that
 *    register data types in it:
 */
void TypeFactoryTests::makeSchema()
{
    CSqliteStatement::execute(
        *m_db,
        "CREATE TABLE variable_types (                  \
               id    INTEGER PRIMARY KEY NOT NULL,      \
               type_name VARCHAR(256) NOT NULL          \
        )"
    );
}

/*---------------------------------------------------------------------------
 * Tests:
 */

// Schema gets created:

void TypeFactoryTests::createSchema() {
    CTypeFactory::createSchema(*m_db);
    CSqliteStatement s(
        *m_db,
        "SELECT COUNT(*) AS c FROM sqlite_master      \
            WHERE type = 'table' AND name='variable_types'"
    );
    ++s;
    EQ(1, s.getInt(0));
}
// Ensure that creating a factory gets the default types in
// The database.

void TypeFactoryTests::checkDefaultTypes()
{
    CTypeFactory::createSchema(*m_db);
    CTypeFactory fact(*m_db);                      /// Should ensure default types are there.
    
    CSqliteStatement s(
        *m_db,
        "SELECT COUNT(*) AS C FROM variable_types WHERE type_name=?"
    );
    const char** pType = defaultTypes;
    while (*pType) {
        s.bind(1, *pType, -1, SQLITE_TRANSIENT);
        ++s;
        EQMSG(*pType, 1, s.getInt(0));
        s.reset();
        
        pType++;
    }
}

// Creating two factories on the same database should only register types once.

void TypeFactoryTests::checkNoDupTypes()
{
    CTypeFactory::createSchema(*m_db);
    CTypeFactory fact1(*m_db);
    CTypeFactory fact2(*m_db);
    
    CSqliteStatement s(
        *m_db,
        "SELECT COUNT(*) AS C FROM variable_types WHERE type_name=?"
    );
    const char** pType = defaultTypes;
    while (*pType) {
        s.bind(1, *pType, -1, SQLITE_TRANSIENT);
        ++s;
        EQMSG(*pType, 1, s.getInt(0));
        s.reset();
        
        pType++;
    }    
}
// CDataType::id function is faithful:

void TypeFactoryTests::baseClassId()
{
    CIntegerType i(1234);
    EQ(1234, i.id());
}
// CDataType::type method is faithful:

void TypeFactoryTests::baseClassType()
{
    CIntegerType i(1234);
    EQ(std::string("integer"), i.type());
}
// integer verifies an integer is legal:

void TypeFactoryTests::intLegalOk()
{
    CIntegerType i(1234);
    
    ASSERT(i.legal("1"));
}
// Integer verifies an integer is illegal.

void TypeFactoryTests::intLegalBad()
{
    CIntegerType i(1234);
    
    ASSERT(!i.legal("help"));
}
// Integer provides the right default value:

void TypeFactoryTests::intDefault()
{
    CIntegerType i(1234);
    
    EQ(std::string("0"), i.defaultValue());
}

// Real says ok to legal real values:

void TypeFactoryTests::realLegalOk()
{
    CRealType r(4567);
    ASSERT(r.legal("3.14156"));
}

// Real says no to bad real values:

void TypeFactoryTests::realLegalBad()
{
    CRealType r(4567);
    
    ASSERT(!r.legal("I am not a real"));
}

// Real default value is 0.0:

void TypeFactoryTests::readlDefault()
{
    CRealType r(4567);
    
    EQ(std::string("0.0"), r.defaultValue());
}

// String legal returns true:

void TypeFactoryTests::stringLegal()
{
    CStringType s(9876);
    
    ASSERT(s.legal("whatever goes here is good"));
}
// String default value is empty string:

void TypeFactoryTests::stringDefault()
{
    CStringType s(9876);
    
    EQ(std::string(""), s.defaultValue());
}

// The base class of creators can register a data type and return its id:

void TypeFactoryTests::baseCreatorRegisters()
{
    makeSchema();
 
    CDataTypeCreatorBase c(*m_db);
    int id = c.lookupId("integer");
    
    // this should create an entry in the variable_types table with the
    // id it just returned:
    
    CSqliteStatement s(
        *m_db,
        "SELECT id FROM variable_types WHERE type_name = 'integer'"
    );
    ++s;
    ASSERT(! s.atEnd());
    
    EQ(id, s.getInt(0));
    
}

//  There should not be a double registration:

void TypeFactoryTests::baseCreatorSingleRegister()
{
    makeSchema();
    CDataTypeCreatorBase c(*m_db);
    int id1 = c.lookupId("integer");
    int id2 = c.lookupId("integer");
    EQ(id1, id2);
}

// Ensure that an integer type creator makes objects of the right id.

void TypeFactoryTests::integerCreatorType()
{
    makeSchema();
    CIntegerTypeCreator c(*m_db);
    CDataType* type = c();
    
    ASSERT(type);
    EQ(c.lookupId("integer"), type->id());
    
    delete type;
}

// The description should be "integer"

void TypeFactoryTests::integerCreatorDesc()
{
    makeSchema();
    CIntegerTypeCreator c(*m_db);
    
    EQ(std::string("integer"), c.describe());

}

void TypeFactoryTests::integerCreatorTypeName()
{
    makeSchema();
    CIntegerTypeCreator c(*m_db);
    CDataType* type = c();
    
    EQ(std::string("integer"), type->type());
    
    delete type;
}
// ensure that a real type creator makes objects of the right id.

void TypeFactoryTests::realCreatorType()
{
    makeSchema();
    CRealTypeCreator c(*m_db);
    CDataType* type = c();
    
    ASSERT(type);
    EQ(c.lookupId("real"), type->id());
    
    delete type;    
}

void TypeFactoryTests::realCreatorDesc()
{
    makeSchema();
    CRealTypeCreator c(*m_db);
    EQ(std::string("real"), c.describe());
}
 
 void TypeFactoryTests::realCreatorTypeName()
 {
    makeSchema();
    CRealTypeCreator c(*m_db);
    CDataType* type = c();
    EQ(std::string("real"), type->type());
    
    delete type;
    
 }
 
 // String creator tests:
 
 void TypeFactoryTests::stringCreatorType()
 {
    makeSchema();
    CStringTypeCreator c(*m_db);
    CDataType* type = c();
    
    ASSERT(type);
    EQ(c.lookupId("string"), type->id());
    
    delete type;    
    
 }
 void TypeFactoryTests::stringCreatorDesc()
 {
    makeSchema();
    CStringTypeCreator c(*m_db);

    EQ(std::string("string"), c.describe());
 }
 void TypeFactoryTests::stringCreatorTypeName()
 {
    makeSchema();
     CStringTypeCreator c(*m_db);
    CDataType* type = c();
    
    ASSERT(type);
    EQ(std::string("string"), type->type());
    
    delete type;    
 }