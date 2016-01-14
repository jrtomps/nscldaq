// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include <string>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <stdexcept>
#include <stdio.h>

#include <Asserts.h>

#include "CVardbRingBuffer.h"
#include <CVarMgrFileApi.h>
#include <CVariableDb.h>


class VardbRingApiTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(VardbRingApiTests);
  CPPUNIT_TEST(noInitialSchema);
  CPPUNIT_TEST(makeSchema);
  CPPUNIT_TEST(makeSchema2x);
  CPPUNIT_TEST(create);
  CPPUNIT_TEST(createdup);
  CPPUNIT_TEST(createnodefaults);
  
  CPPUNIT_TEST(rm);
  CPPUNIT_TEST(rmnox);
  CPPUNIT_TEST(rmextra);

  CPPUNIT_TEST(setdatasize);
  CPPUNIT_TEST(setmaxconsumers);
  
  CPPUNIT_TEST(info);

  CPPUNIT_TEST(ls1);
  CPPUNIT_TEST(ls2);
  CPPUNIT_TEST_SUITE_END();


private:

protected:
  void noInitialSchema();
  void makeSchema();
  void makeSchema2x();
  
  void create();
  void createdup();
  void createnodefaults();
  
  void rm();
  void rmnox();
  void rmextra();
  
  void setdatasize();
  void setmaxconsumers();
  
  void info();

  void ls1();
  void ls2();
  
private:
  CVarMgrApi*         m_pApi;
  CVardbRingBuffer*   m_pRingApi;
  std::string         m_dbFile;
public:
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
    
    m_pRingApi = new CVardbRingBuffer(uri.c_str());
  }
  void tearDown() {
    delete m_pApi;
    delete m_pRingApi;
    unlink(m_dbFile.c_str());
  }  
};

CPPUNIT_TEST_SUITE_REGISTRATION(VardbRingApiTests);

void VardbRingApiTests::noInitialSchema() {
  ASSERT(!m_pRingApi->haveSchema());
}


void VardbRingApiTests::makeSchema()
{
  m_pRingApi->createSchema();
  ASSERT(m_pRingApi->haveSchema());
}

void VardbRingApiTests::makeSchema2x()
{
  m_pRingApi->createSchema();
  CPPUNIT_ASSERT_NO_THROW(
    m_pRingApi->createSchema()
  );
}

// Basic create:
void VardbRingApiTests::create()
{
  m_pRingApi->createSchema();
  m_pRingApi->create("fox", "charlie");
  CPPUNIT_ASSERT_NO_THROW(
    m_pApi->cd("RingBuffers/fox@charlie")
  );                                          // create makes this directory.
  
  EQ(static_cast<unsigned long>(8*1024*1024), strtoul(m_pApi->get("datasize").c_str(), NULL, 0));
  EQ(std::string("100"), m_pApi->get("maxconsumers"));
}

void VardbRingApiTests::createdup()
{
  m_pRingApi->createSchema();
  m_pRingApi->create("fox", "charlie");
  CPPUNIT_ASSERT_THROW(
    m_pRingApi->create("fox", "charlie"),
    std::runtime_error
  );
}

void VardbRingApiTests::createnodefaults()
{
  m_pRingApi->createSchema();
  m_pRingApi->create("fox", "charlie", 1000000, 25);
  m_pApi->cd("/RingBuffers/fox@charlie");
  EQ(std::string("1000000"), m_pApi->get("datasize"));
  EQ(std::string("25"), m_pApi->get("maxconsumers"));
}

void VardbRingApiTests::rm()
{
  m_pRingApi->createSchema();
  m_pRingApi->create("fox",  "charlie");
  m_pRingApi->destroy("fox", "charlie");
  
  CPPUNIT_ASSERT_THROW(
    m_pApi->cd("/RingBufers/fox@charlie"),
    std::runtime_error
  );
}


void VardbRingApiTests::rmnox()
{
  m_pRingApi->createSchema();
  CPPUNIT_ASSERT_THROW(
    m_pRingApi->destroy("fox", "charlie"),
    std::runtime_error
  );
}

// Remove ring with some extra stuff added to it:

void VardbRingApiTests::rmextra()
{
  m_pRingApi->createSchema();
  m_pRingApi->create("fox", "charlie");
  m_pApi->cd("/RingBuffers/fox@charlie");
  m_pApi->declare("extra-var", "string", "some extra stuff");
  m_pApi->mkdir("extra-dir");
  m_pApi->declare("extra-dir/stuff", "integer", "1245");
  
  m_pRingApi->destroy("fox", "charlie");
  
  // Everything should be gone:
  
  CPPUNIT_ASSERT_THROW(
    m_pApi->cd("/RingBuffers/fox@charlie"),
    std::runtime_error
  );
}

void VardbRingApiTests::setdatasize()
{
  m_pRingApi->createSchema();
  m_pRingApi->create("fox", "charlie");
  m_pRingApi->setMaxData("fox", "charlie", 1000000);
  
  EQ(std::string("1000000"), m_pApi->get("/RingBuffers/fox@charlie/datasize"));
}

void VardbRingApiTests::setmaxconsumers()
{
  m_pRingApi->createSchema();
  m_pRingApi->create("fox", "charlie");
  m_pRingApi->setMaxConsumers("fox", "charlie", 25);
  
  EQ(std::string("25"), m_pApi->get("/RingBuffers/fox@charlie/maxconsumers"));
}

void VardbRingApiTests::info()
{
  m_pRingApi->createSchema();
  m_pRingApi->create("fox", "charlie", 1000000, 20);
  
  
  CVardbRingBuffer::RingInfo info = m_pRingApi->ringInfo("fox", "charlie");
  
  EQ(std::string("fox"), info.s_name);
  EQ(std::string("charlie"), info.s_host);
  EQ((unsigned)(1000000), info.s_dataSize);
  EQ((unsigned)(20), info.s_maxConsumers);
}

void VardbRingApiTests::ls1()
{
  m_pRingApi->createSchema();
  m_pRingApi->create("fox", "charlie", 1000000, 25);
  
  std::vector<CVardbRingBuffer::RingInfo> rings = m_pRingApi->list();
  
  EQ(size_t(1), rings.size());
  EQ(std::string("fox"), rings[0].s_name);
  EQ(std::string("charlie"), rings[0].s_host);
  EQ(unsigned(1000000), rings[0].s_dataSize);
  EQ(unsigned(25), rings[0].s_maxConsumers);
}

void VardbRingApiTests::ls2()
{
  m_pRingApi->createSchema();
  m_pRingApi->create("aRing", "charlie", 1000000, 25);
  m_pRingApi->create("bRing", "spdaq20", 2000000, 35);
  
  std::vector<CVardbRingBuffer::RingInfo> rings = m_pRingApi->list();
  
  EQ(size_t(2), rings.size());
  EQ(std::string("aRing"), rings[0].s_name);
  EQ(std::string("charlie"), rings[0].s_host);
  EQ(unsigned(1000000), rings[0].s_dataSize);
  EQ(unsigned(25), rings[0].s_maxConsumers);
  
  EQ(std::string("bRing"), rings[1].s_name);
  EQ(std::string("spdaq20"), rings[1].s_host);
  EQ(unsigned(2000000), rings[1].s_dataSize);
  EQ(unsigned(35), rings[1].s_maxConsumers);
}