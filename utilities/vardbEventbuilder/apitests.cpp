// Template for a test suite.


#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>

#include "Asserts.h"
#include "CVardbEventBuilder.h"
#include <CVarMgrFileApi.h>
#include <CVariableDb.h>
#include <string>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>


class VarMgrEvbtests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(VarMgrEvbtests);
  CPPUNIT_TEST(existsNo);
  CPPUNIT_TEST(existsYes);
  CPPUNIT_TEST(createSchema);
  CPPUNIT_TEST(createWhenExists);
  
  CPPUNIT_TEST(createEvb);
  //CPPUNIT_TEST(createEvbExists);
  CPPUNIT_TEST_SUITE_END();

protected:
  void existsNo();
  void existsYes();
  void createSchema();
  void createWhenExists();
  void createEvb();
  
private:
  CVarMgrApi*         m_pApi;
  CVardbEventBuilder* m_pEvbApi;
  std::string         m_dbFile;
public:
  // Run at the start of each test
  // - Create database file.
  // - Create file database api.
  // - Create the event builder database api:
  //
  void setUp() {
    // Create the database file:
    
    char dbTemplate[PATH_MAX];
    strcpy(dbTemplate, "testdbXXXXXX");
    int fd = mkstemp(dbTemplate);
    if (fd == -1) {
      perror("Unable to make temporary database file");
      exit(EXIT_FAILURE);
    }
    // Create the database file:
  
    CVariableDb::create(dbTemplate);
    m_dbFile = dbTemplate;
    close(fd);
    
    // Now make the APis:
    
    m_pApi = new CVarMgrFileApi(dbTemplate);
    std::string uri = "file://./";
    uri += dbTemplate;
    
    m_pEvbApi = new CVardbEventBuilder(uri.c_str());
    
  }
  // Run at the end of each test:
  // - Delete the database object.
  // - delete the temp file.
  void tearDown() {
    delete m_pApi;
    delete m_pEvbApi;
    unlink(m_dbFile.c_str());
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(VarMgrEvbtests);

// Unless created the event builder db won't exist:

void VarMgrEvbtests::existsNo() {
  ASSERT(!m_pEvbApi->schemaExists());
}

void VarMgrEvbtests::existsYes() {
  m_pApi->mkdir("/EventBuilder");
  ASSERT(m_pEvbApi->schemaExists());
}
// Create the schema:

void VarMgrEvbtests::createSchema()
{
  m_pEvbApi->createSchema();
  ASSERT(m_pEvbApi->schemaExists());
}

void VarMgrEvbtests::createWhenExists()
{
  m_pEvbApi->createSchema();
  
  // Should not throw to create twice:
  
  CPPUNIT_ASSERT_NO_THROW(m_pEvbApi->createSchema());
}

void VarMgrEvbtests::createEvb()
{
  m_pEvbApi->createSchema();
  m_pEvbApi->createEventBuilder(
    "test", "charlie", 10);
  
  // If the directory exists we can cd to it:
  
  CPPUNIT_ASSERT_NO_THROW(m_pApi->cd("/EventBuilder/test"));
  
  // Variables should all be set accordingly.  Note that there will be
  // throws if some are not created which will make the outer assert fail.
  
  CPPUNIT_ASSERT_NO_THROW(
    {
      EQ(std::string("charlie"), m_pApi->get("host"));
      EQ(std::string("10"),      m_pApi->get("coincidenceInterval"));
      EQ(std::string("0"),       m_pApi->get("sourceId"));
      EQ(std::string("ORDERER"), m_pApi->get("servicePrefix"));
      EQ(std::string("true"),    m_pApi->get("build"));
      EQ(std::string("earliest"), m_pApi->get("timestampPolicy"));
      EQ(std::string(""),        m_pApi->get("serviceSuffix"));
    }
  );
  
}