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
#include <stdexcept>
#include <stdio.h>


class VarMgrEvbtests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(VarMgrEvbtests);
  CPPUNIT_TEST(existsNo);
  CPPUNIT_TEST(existsYes);
  CPPUNIT_TEST(createSchema);
  CPPUNIT_TEST(createWhenExists);
  
  CPPUNIT_TEST(createEvb);
  CPPUNIT_TEST(createEvbExists);

  // We'll use set host tests to ensure that
  // nonexistent hosts throw.
  
  CPPUNIT_TEST(evbsethost);
  CPPUNIT_TEST(evbsethostnox);
  
  CPPUNIT_TEST(evbsetinterval);
  CPPUNIT_TEST(evbsetid);
  CPPUNIT_TEST(setprefix);
  CPPUNIT_TEST(setsuffix);
  CPPUNIT_TEST(disablebuild);
  CPPUNIT_TEST(enablebuild);
  CPPUNIT_TEST(settspolicy);

  // Event builder removal:
  
  CPPUNIT_TEST(rmEventBuilder);
  CPPUNIT_TEST(rmNoxEventBuilder);
  
  
  // Get event builder info:
  
  CPPUNIT_TEST(evbinfo);
  CPPUNIT_TEST(evbNoxInfo);
  CPPUNIT_TEST(evbls);
  
  // Data source creation:
  
  CPPUNIT_TEST(mkDs);
  CPPUNIT_TEST(mkDsDup);
  CPPUNIT_TEST(mkDsNoxEvb);
  
  // data source modification:
  
  CPPUNIT_TEST(dsSetHost);
  CPPUNIT_TEST(dsSetPath);
  CPPUNIT_TEST(dsSetRingUri);
  CPPUNIT_TEST(dsSetIds);
  CPPUNIT_TEST(dsSetInfo);
  CPPUNIT_TEST(dsSetDefaultId);
  CPPUNIT_TEST(dsExpectBodyHeaders);
  CPPUNIT_TEST(dsDontExpectBodyHeaders);
  CPPUNIT_TEST(dsSetTimestampExt);

  CPPUNIT_TEST(dsInfo);
  CPPUNIT_TEST(dsInfoNox);
  CPPUNIT_TEST(lsds);
  CPPUNIT_TEST(rmds);
  //CPPUNIT_TEST(rmnoxds);
  CPPUNIT_TEST_SUITE_END();

protected:
  void existsNo();
  void existsYes();
  void createSchema();
  void createWhenExists();
  void createEvb();
  void createEvbExists();
  
  void evbsethost();
  void evbsethostnox();
  void evbsetinterval();
  void evbsetid();
  void setprefix();
  void setsuffix();
  void disablebuild();
  void enablebuild();
  void settspolicy();
  
  void rmEventBuilder();
  void rmNoxEventBuilder();
  
  void evbinfo();
  void evbNoxInfo();
  void evbls();
  
  void mkDs();
  void mkDsDup();
  void mkDsNoxEvb();
  
  void dsSetHost();
  void dsSetPath();
  void dsSetRingUri();
  void dsSetIds();
  void dsSetInfo();
  void dsSetDefaultId();
  void dsExpectBodyHeaders();
  void dsDontExpectBodyHeaders();
  void dsSetTimestampExt();
  
  void dsInfo();
  void dsInfoNox();
  void lsds();
  void rmds();
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
private:
  void setup2();
  void setup3();
};

CPPUNIT_TEST_SUITE_REGISTRATION(VarMgrEvbtests);

void VarMgrEvbtests::setup2()
{
  m_pEvbApi->createSchema();
  m_pEvbApi->createEventBuilder(
    "test", "charlie", 10);
  
}

void VarMgrEvbtests::setup3()
{
  setup2();
  std::vector<unsigned> ids;
  ids.push_back(1);
  ids.push_back(2);
  m_pEvbApi->addDataSource(
    "test", "ds1", "charlie", "/usr/opt/daq/current/bin/ringFragmentSource",
    "tcp://charlie/fox", ids, "Test data source"
  );
}
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
  setup2();
  
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
void VarMgrEvbtests::createEvbExists()
{
  setup2();
  
  // Creating another test event builder will throw.
  
  CPPUNIT_ASSERT_THROW(
      m_pEvbApi->createEventBuilder(
        "test", "charlie", 10),
      std::runtime_error
  );
  
}

void VarMgrEvbtests::evbsethost()
{
  setup2();
  m_pEvbApi->evbSetHost("test", "spdaq22");
  
  EQ(std::string("spdaq22"), m_pApi->get("EventBuilder/test/host"));
}
void VarMgrEvbtests::evbsethostnox()
{
  CPPUNIT_ASSERT_THROW(
    m_pEvbApi->evbSetHost("test", "spaq22"),
    std::runtime_error
  );
  
}
void VarMgrEvbtests::evbsetinterval()
{
  setup2();
  m_pEvbApi->evbSetCoincidenceInterval("test", 15);
  EQ(std::string("15"), m_pApi->get("EventBuilder/test/coincidenceInterval"));
  
}

void VarMgrEvbtests::evbsetid()
{
  setup2();
  m_pEvbApi->evbSetSourceId("test", 1);
  EQ(std::string("1"), m_pApi->get("EventBuilder/test/sourceId"));
}

void VarMgrEvbtests::setprefix()
{
  setup2();
  m_pEvbApi->evbSetServicePrefix("test", "junk");
  EQ(std::string("junk"), m_pApi->get("EventBuilder/test/servicePrefix"));
}
void VarMgrEvbtests::setsuffix()
{
  setup2();
  m_pEvbApi->evbSetServiceSuffix("test", "fox");
  EQ(std::string("fox"), m_pApi->get("EventBuilder/test/serviceSuffix"));
}
void VarMgrEvbtests::disablebuild()
{
  setup2();
  m_pEvbApi->evbDisableBuild("test");
  EQ(std::string("false"), m_pApi->get("EventBuilder/test/build"));
}

void VarMgrEvbtests::enablebuild()
{
 setup2();
  m_pEvbApi->evbDisableBuild("test");   // false
  m_pEvbApi->evbEnableBuild("test");    // now true
  EQ(std::string("true"), m_pApi->get("EventBuilder/test/build")); 
}
void VarMgrEvbtests::settspolicy()
{
  setup2();
  m_pEvbApi->evbSetTimestampPolicy("test", CVardbEventBuilder::latest);
  EQ(std::string("latest"), m_pApi->get("EventBuilder/test/timestampPolicy"));
  m_pEvbApi->evbSetTimestampPolicy("test", CVardbEventBuilder::average);
  EQ(std::string("average"), m_pApi->get("EventBuilder/test/timestampPolicy"));
  m_pEvbApi->evbSetTimestampPolicy("test", CVardbEventBuilder::earliest);
  EQ(std::string("earliest"), m_pApi->get("EventBuilder/test/timestampPolicy"));
}

void VarMgrEvbtests::rmEventBuilder()
{
  setup2();
  m_pEvbApi->rmEventBuilder("test");
  CPPUNIT_ASSERT_THROW(
    m_pApi->cd("EventBuilder/test"),
    std::runtime_error
  );
}
void VarMgrEvbtests::rmNoxEventBuilder()
{
  setup2();
  CPPUNIT_ASSERT_THROW(
    m_pEvbApi->rmEventBuilder("junk"),
    std::runtime_error
  );
}

void VarMgrEvbtests::evbinfo()
{
  setup2();
  CVardbEventBuilder::EvbDescription info = m_pEvbApi->evbInfo("test");
  
  EQ(std::string("test"),    info.s_name);
  EQ(std::string("charlie"), info.s_host);
  EQ(unsigned(10),           info.s_coincidenceInterval);
  EQ(std::string("ORDERER"), info.s_servicePrefix);
  EQ(std::string(""),        info.s_serviceSuffix);
  EQ(true,                   info.s_build);
  EQ(CVardbEventBuilder::earliest, info.s_timestampPolicy);
  EQ(unsigned(0),            info.s_sourceId);
}

void VarMgrEvbtests::evbNoxInfo()
{
  setup2();
  CPPUNIT_ASSERT_THROW(
    m_pEvbApi->evbInfo("junk"),
    std::runtime_error
  );
}

void VarMgrEvbtests::evbls()
{
  setup2();   // that gives us test, event source 0.
  m_pEvbApi->rmEventBuilder("test");   // So get rid of it.
  
  // Create the event builders:
  
  for (int i =0; i < 10; i++) {
    char name[100];
    sprintf(name, "test%d", i );
    m_pEvbApi->createEventBuilder(name, "charlie", 10, i);
  }
  std::vector<CVardbEventBuilder::EvbDescription> evbs = m_pEvbApi->listEventBuilders();
  
  EQ(size_t(10), evbs.size());
  for (int i = 0; i < 10; i++) {
    char name[100];
    sprintf(name, "test%d", i);
    EQ(std::string(name), evbs[i].s_name);
    EQ(unsigned(i), evbs[i].s_sourceId);
  }
}

void VarMgrEvbtests::mkDs()
{
  setup3();
  
  CPPUNIT_ASSERT_NO_THROW(
    m_pApi->cd("/EventBuilder/test/ds1")
  );
  CPPUNIT_ASSERT_NO_THROW(EQ(std::string("charlie"), m_pApi->get("host")));
  CPPUNIT_ASSERT_NO_THROW(
    EQ(
      std::string("/usr/opt/daq/current/bin/ringFragmentSource"),
      m_pApi->get("path")
    )
  );
  CPPUNIT_ASSERT_NO_THROW(EQ(std::string("Test data source"), m_pApi->get("info")));
  CPPUNIT_ASSERT_NO_THROW(EQ(std::string("1"), m_pApi->get("id0")));
  CPPUNIT_ASSERT_NO_THROW(EQ(std::string("2"), m_pApi->get("id1")));
  CPPUNIT_ASSERT_NO_THROW(EQ(std::string("tcp://charlie/fox"), m_pApi->get("ring")));
  CPPUNIT_ASSERT_NO_THROW(EQ(std::string("0"), m_pApi->get("default-id")));
  CPPUNIT_ASSERT_NO_THROW(EQ(std::string(""), m_pApi->get("timestamp-extractor")));
  CPPUNIT_ASSERT_NO_THROW(EQ(std::string("true"), m_pApi->get("expect-bodyheaders")));
    
}

void VarMgrEvbtests::mkDsDup()
{
  setup3();
  std::vector<unsigned> ids;
  
  // Throw for duplicate event builder source.
  
  CPPUNIT_ASSERT_THROW(
    m_pEvbApi->addDataSource("test", "ds1", "anything", "something", "ring", ids),
    std::runtime_error
  );
}


void VarMgrEvbtests::mkDsNoxEvb()
{
  setup3();
  std::vector<unsigned> ids;
  
  
    // Throw for no such event builder.
  CPPUNIT_ASSERT_THROW(
    m_pEvbApi->addDataSource("testing", "ds2", "anything", "something", "ring", ids),
    std::runtime_error
  );
}

void VarMgrEvbtests::dsSetHost()
{
  setup3();
  m_pEvbApi->dsSetHost("test", "ds1", "spdaq22.nscl.msu.edu");
  EQ(std::string("spdaq22.nscl.msu.edu"), m_pApi->get("/EventBuilder/test/ds1/host"));
}

void VarMgrEvbtests::dsSetPath()
{
  setup3();
  m_pEvbApi->dsSetPath("test", "ds1", "/usr/bin/ls");
  EQ(std::string("/usr/bin/ls"), m_pApi->get("/EventBuilder/test/ds1/path"));
}

void VarMgrEvbtests::dsSetRingUri()
{
  setup3();
  m_pEvbApi->dsSetRingUri("test", "ds1", "tcp://spdaq20/0400x");
  EQ(std::string("tcp://spdaq20/0400x"), m_pApi->get("/EventBuilder/test/ds1/ring"));
}

void VarMgrEvbtests::dsSetIds()
{
  setup3();
  std::vector<unsigned> ids;
  ids.push_back(5);
  ids.push_back(4);
  ids.push_back(3);
  m_pEvbApi->dsSetIds("test", "ds1", ids);
  
  m_pApi->cd("/EventBuilder/test/ds1");
  
  EQ(std::string("5"), m_pApi->get("id0"));
  EQ(std::string("4"), m_pApi->get("id1"));
  EQ(std::string("3"), m_pApi->get("id2"));
}

void VarMgrEvbtests::dsSetInfo()
{
  setup3();
  
  m_pEvbApi->dsSetInfo("test", "ds1", "My new information");
  EQ(std::string("My new information"), m_pApi->get("/EventBuilder/test/ds1/info"));
}
void VarMgrEvbtests::dsSetDefaultId()
{
  setup3();
  
  m_pEvbApi->dsSetDefaultId("test", "ds1", 666);
  EQ(std::string("666"), m_pApi->get("/EventBuilder/test/ds1/default-id"));
}

void VarMgrEvbtests::dsExpectBodyHeaders()
{
  setup3();
  m_pEvbApi->dsExpectBodyHeaders("test", "ds1");
  EQ(
    std::string("true"),
    m_pApi->get("/EventBuilder/test/ds1/expect-bodyheaders"));
}

void VarMgrEvbtests::dsDontExpectBodyHeaders()
{
  setup3();
  m_pEvbApi->dsDontExpectBodyHeaders("test", "ds1");
  EQ(
    std::string("false"),
    m_pApi->get("/EventBuilder/test/ds1/expect-bodyheaders"));
}

void VarMgrEvbtests::dsSetTimestampExt()
{
  setup3();
  m_pEvbApi->dsSetTimestampExtractor("test", "ds1", "/user/fox/lib/libtsextract.so");
  EQ(
    std::string("/user/fox/lib/libtsextract.so"),
    m_pApi->get("/EventBuilder/test/ds1/timestamp-extractor")
  );
}

void VarMgrEvbtests::dsInfo()
{
  setup3();
  CVardbEventBuilder::DsDescription Info = m_pEvbApi->dsInfo("test", "ds1");
  
  EQ(std::string("ds1"),     Info.s_name);
  EQ(std::string("charlie"), Info.s_host);
  EQ(std::string("/usr/opt/daq/current/bin/ringFragmentSource"), Info.s_path);
  EQ(std::string("Test data source"), Info.s_info);
  EQ(size_t(2), Info.s_ids.size());
  EQ(unsigned(1), Info.s_ids[0]);
  EQ(unsigned(2), Info.s_ids[1]);
  EQ(std::string("tcp://charlie/fox"), Info.s_ringUri);
  EQ(true, Info.s_expectBodyheaders);
  EQ(unsigned(0), Info.s_defaultId);
  EQ(std::string(""), Info.s_timestampExtractor);
}

void VarMgrEvbtests::dsInfoNox()
{
  setup2();
  
  CPPUNIT_ASSERT_THROW(
    CVardbEventBuilder::DsDescription Info = m_pEvbApi->dsInfo("test", "ds1"),
    std::runtime_error
  );
}

void VarMgrEvbtests::lsds()
{
  setup3();                       // ds1 is built.
  std::vector<unsigned> ids;
  
  ids.push_back(2);
  ids.push_back(3);    
  ids.push_back(5);    // Prime example of a vector of source ids.
  ids.push_back(7);
  ids.push_back(11);
  
  m_pEvbApi->addDataSource(
    "test", "ds2", "spdaq20", "/usr/opt/daq/current/bin/ringFragmentSource",
    "tcp://spdaq20/0400x", ids, "2nd Test data source"
  );
  m_pEvbApi->addDataSource(
    "test", "ds3", "charlie", "/usr/opt/daq/current/bin/ringFragmentSource",
    "tcp://spdaq30/0400x", ids, "remote Test data source"
    );
  
    std::vector<CVardbEventBuilder::DsDescription> desc =
      m_pEvbApi->listDataSources("test");
      
    EQ(size_t(3), desc.size());
    
    EQ(std::string("ds1"), desc[0].s_name);
    EQ(std::string("charlie"), desc[0].s_host);
    
    EQ(std::string("ds2"), desc[1].s_name);
    EQ(std::string("spdaq20"), desc[1].s_host);
    EQ(std::string("tcp://spdaq20/0400x"), desc[1].s_ringUri);
    EQ(std::string("2nd Test data source"), desc[1].s_info);
    
    EQ(std::string("ds3"), desc[2].s_name);   // If the prior tests are ok this
                                              // is probably sufficient.
    
}

void VarMgrEvbtests::rmds()
{
setup3();                       // ds1 is built.
  std::vector<unsigned> ids;
  
  ids.push_back(2);
  ids.push_back(3);    
  ids.push_back(5);    // Prime example of a vector of source ids.
  ids.push_back(7);
  ids.push_back(11);
  
  m_pEvbApi->addDataSource(
    "test", "ds2", "spdaq20", "/usr/opt/daq/current/bin/ringFragmentSource",
    "tcp://spdaq20/0400x", ids, "2nd Test data source"
  );
  m_pEvbApi->addDataSource(
    "test", "ds3", "charlie", "/usr/opt/daq/current/bin/ringFragmentSource",
    "tcp://spdaq30/0400x", ids, "remote Test data source"
    );
  
  m_pEvbApi->rmDataSource("test", "ds2");
  std::vector<CVardbEventBuilder::DsDescription> desc =
      m_pEvbApi->listDataSources("test");
      
  EQ(size_t(2), desc.size());
  
  EQ(std::string("ds1"), desc[0].s_name);
  EQ(std::string("ds3"), desc[1].s_name);
  
}