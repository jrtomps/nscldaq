// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include <Asserts.h>
#include <CSqlite.h>
#include <CSqliteStatement.h>
#include "CStatusMessage.h"

#define private public
#include "CStatusDb.h"
#undef private


class SCDbtests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(SCDbtests);
  CPPUNIT_TEST(addapp);
  CPPUNIT_TEST(addapp2);
  CPPUNIT_TEST(addtransition2);
  CPPUNIT_TEST_SUITE_END();


private:
  CStatusDb*  m_db;
public:
  void setUp() {
    m_db = new CStatusDb(":memory:", CSqlite::readwrite);
  }
  void tearDown() {
    delete m_db;
  }
protected:
  void addapp();
  void addapp2();
  void addtransition2();
};

CPPUNIT_TEST_SUITE_REGISTRATION(SCDbtests);

// New application and state change.

void SCDbtests::addapp() {
  m_db->addStateChange(
    CStatusDefinitions::SeverityLevels::INFO, "Readout",
    "charlie.nscl.msu.edu", 12345678, "Begining", "Active"
  );
  // There should be an entry in state_application and one linked to it in
  // state_transition - we'll pull them both in one cool sql query:
  
  CSqliteStatement getInfo(
    m_db->m_handle,
    "SELECT a.id, a.name, a.host, s.app_id, timestamp, leaving, entering     \
            FROM state_application AS a                                     \
            INNER JOIN state_transitions AS s                               \
                  ON a.id = s.app_id                                        \
    "
  );
  ++getInfo;
  ASSERT(!getInfo.atEnd());
  
  EQ(1, getInfo.getInt(0));             // a.id.
  EQ(
    std::string("Readout"),
    std::string(reinterpret_cast<const char*>(getInfo.getText(1)))
  );                      // a.name
  EQ(
    std::string("charlie.nscl.msu.edu"),
    std::string(reinterpret_cast<const char*>(getInfo.getText(2)))
  );                      // a.host
  EQ(1, getInfo.getInt(3));  // s.app_id
  EQ(12345678, getInfo.getInt(4));  // timestamp.
  EQ(
    std::string("Begining"),
    std::string(reinterpret_cast<const char*>(getInfo.getText(5)))
  );
  EQ(
    std::string("Active"),
    std::string(reinterpret_cast<const char*>(getInfo.getText(6)))
  );
  
  ++getInfo;
  ASSERT(getInfo.atEnd());
}

// Two transitions on two apps:

void SCDbtests::addapp2()
{
  m_db->addStateChange(
    CStatusDefinitions::SeverityLevels::INFO, "Readout",
    "charlie.nscl.msu.edu", 12345678, "Begining", "Active"
  );
  m_db->addStateChange(
    CStatusDefinitions::SeverityLevels::INFO, "Readout",
    "spdaq20.nscl.msu.edu", 12345680, "Begining", "Active"
  );
  
  // Should be two entries in both tables linked to the appropriate applications:
  
    CSqliteStatement getInfo(
    m_db->m_handle,
    "SELECT a.id, a.name, a.host, s.app_id, timestamp, leaving, entering     \
            FROM state_application AS a                                      \
            INNER JOIN state_transitions AS s                                \
                  ON a.id = s.app_id                                         \
            ORDER BY a.id ASC                                                \
    "
  );
  ++getInfo;
  ASSERT(!getInfo.atEnd());
  
  // App and state change from app 1:
  
  EQ(1, getInfo.getInt(0));             // a.id.
  EQ(
    std::string("Readout"),
    std::string(reinterpret_cast<const char*>(getInfo.getText(1)))
  );                      // a.name
  EQ(
    std::string("charlie.nscl.msu.edu"),
    std::string(reinterpret_cast<const char*>(getInfo.getText(2)))
  );                      // a.host
  EQ(1, getInfo.getInt(3));  // s.app_id
  EQ(12345678, getInfo.getInt(4));  // timestamp.
  EQ(
    std::string("Begining"),
    std::string(reinterpret_cast<const char*>(getInfo.getText(5)))
  );
  EQ(
    std::string("Active"),
    std::string(reinterpret_cast<const char*>(getInfo.getText(6)))
  ); 
  
  ++getInfo;
  ASSERT(!getInfo.atEnd());
  
  // App and state change from app 2:
  
    EQ(2, getInfo.getInt(0));             // a.id.
  EQ(
    std::string("Readout"),
    std::string(reinterpret_cast<const char*>(getInfo.getText(1)))
  );                      // a.name
  EQ(
    std::string("spdaq20.nscl.msu.edu"),
    std::string(reinterpret_cast<const char*>(getInfo.getText(2)))
  );                      // a.host
  EQ(2, getInfo.getInt(3));  // s.app_id
  EQ(12345680, getInfo.getInt(4));  // timestamp.
  EQ(
    std::string("Begining"),
    std::string(reinterpret_cast<const char*>(getInfo.getText(5)))
  );
  EQ(
    std::string("Active"),
    std::string(reinterpret_cast<const char*>(getInfo.getText(6)))
  );
  
  ++getInfo;
  ASSERT(getInfo.atEnd());
}

// Two transitions on the same application:


void SCDbtests::addtransition2()
{
  m_db->addStateChange(
    CStatusDefinitions::SeverityLevels::INFO, "Readout",
    "charlie.nscl.msu.edu", 12345678, "Beginning", "Active"
  );
  m_db->addStateChange(
    CStatusDefinitions::SeverityLevels::INFO, "Readout",
    "charlie.nscl.msu.edu", 12345680, "Active", "Ending"
  );
  
  // SB one app and two state changes linked to it.
  
  CSqliteStatement getInfo(
    m_db->m_handle,
    "SELECT a.id, a.name, a.host, s.app_id, timestamp, leaving, entering     \
            FROM state_application AS a                                      \
            INNER JOIN state_transitions AS s                                \
                  ON a.id = s.app_id                                         \
            ORDER BY a.id ASC                                                \
    "
  );
  
  ++getInfo;
  ASSERT(!getInfo.atEnd());
  
  EQ(1, getInfo.getInt(0));
  EQ(
    std::string("Readout"),
    std::string(reinterpret_cast<const char*>(getInfo.getText(1)))
  );
  EQ(
    std::string("charlie.nscl.msu.edu"),
    std::string(reinterpret_cast<const char*>(getInfo.getText(2)))
  );
  EQ(1, getInfo.getInt(3));
  EQ(12345678, getInfo.getInt(4));
  EQ(
    std::string("Beginning"),
    std::string(reinterpret_cast<const char*>(getInfo.getText(5)))
  );
  EQ(
     std::string("Active"),
     std::string(reinterpret_cast<const char*>(getInfo.getText(6)))
  );
  
  ++getInfo;
  ASSERT(!getInfo.atEnd());
  
  EQ(1, getInfo.getInt(0));
  EQ(
    std::string("Readout"),
    std::string(reinterpret_cast<const char*>(getInfo.getText(1)))
  );
  EQ(
    std::string("charlie.nscl.msu.edu"),
    std::string(reinterpret_cast<const char*>(getInfo.getText(2)))
  );
  EQ(1, getInfo.getInt(3));
  EQ(12345680, getInfo.getInt(4));
  EQ(
    std::string("Active"),
    std::string(reinterpret_cast<const char*>(getInfo.getText(5)))
  );
  EQ(
     std::string("Ending"),
     std::string(reinterpret_cast<const char*>(getInfo.getText(6)))
  );
  
  ++ getInfo;
  ASSERT(getInfo.atEnd());
}