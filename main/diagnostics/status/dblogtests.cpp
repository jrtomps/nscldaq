// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"

#define private public
#include "CStatusDb.h"
#undef private

#include <CSqlite.h>
#include <CSqliteStatement.h>

#include "CStatusMessage.h"


class DbLogTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(DbLogTests);
  CPPUNIT_TEST(insert);
  CPPUNIT_TEST(insertReuse);
  CPPUNIT_TEST_SUITE_END();


private:
  CStatusDb* m_pDb;
public:
  void setUp() {
    m_pDb = new CStatusDb(":memory:", CSqlite::readwrite);
  }
  void tearDown() {
    delete m_pDb;
  }
protected:
  void insert();
  void insertReuse();
};

CPPUNIT_TEST_SUITE_REGISTRATION(DbLogTests);


// Insert log messages.

void DbLogTests::insert() {
  m_pDb->addLogMessage(
    CStatusDefinitions::SeverityLevels::INFO, "TestApp", "charlie.nscl.msu.edu", 1234567,
    "This is a message"
  );
  
  CSqliteStatement l(m_pDb->m_handle, "SELECT * FROM log_messages");
  ++l;
  
  // id,severity, application, source, timestamp, message
  
  EQ(1, l.getInt(0));                    // id
  EQ(std::string("INFO"), std::string(reinterpret_cast<const char*>(l.getText(1))));
  EQ(std::string("TestApp"), std::string(reinterpret_cast<const char*>(l.getText(2))));
  EQ(std::string("charlie.nscl.msu.edu"), std::string(reinterpret_cast<const char*>(l.getText(3))));
  EQ(int64_t(1234567), l.getInt64(4));
  EQ(std::string("This is a message"), std::string(reinterpret_cast<const char*>(l.getText(5))));
  
}

// Insert two log messages:

void DbLogTests::insertReuse()
{
  m_pDb->addLogMessage(
    CStatusDefinitions::SeverityLevels::INFO, "TestApp", "charlie.nscl.msu.edu", 1234567,
    "This is a message"
  );
  CPPUNIT_ASSERT_NO_THROW(
    m_pDb->addLogMessage(
      CStatusDefinitions::SeverityLevels::WARNING, "AnotherApp", "fox.nscl.msu.edu",
      666666, "Another test message"
    )
  );
  // ensure we have a second message entry.
  
  CSqliteStatement probe(m_pDb->m_handle, "SELECT COUNT(*) FROM log_messages");
  ++probe;
  EQ(2, probe.getInt(0));
  

}