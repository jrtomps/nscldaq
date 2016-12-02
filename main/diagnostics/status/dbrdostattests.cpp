// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include <CStatusMessage.h>

#define private public
#include "CStatusDb.h"
#undef private

#include <CSqlite.h>
#include <CSqliteStatement.h>



class RdoDbStatTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(RdoDbStatTests);
  CPPUNIT_TEST(programAndRun);
  CPPUNIT_TEST(programAndTwoRuns);
  CPPUNIT_TEST(twoprogramswithruns);
  CPPUNIT_TEST(programwithstats);
  CPPUNIT_TEST(programwithtwostats);
  CPPUNIT_TEST(twoprogramswithstats);

  CPPUNIT_TEST_SUITE_END();


private:
  CStatusDb* m_db;
public:
  void setUp() {
    m_db = new CStatusDb(":memory:", CSqlite::readwrite);
  }
  void tearDown() {
    delete m_db;
  }
protected:
  void programAndRun();
  void programAndTwoRuns();
  void twoprogramswithruns();
  void programwithstats();
  void programwithtwostats();
  void twoprogramswithstats();
};

CPPUNIT_TEST_SUITE_REGISTRATION(RdoDbStatTests);


// Only insert a single program and run info record associated with it.

void RdoDbStatTests::programAndRun() {
  m_db->addReadoutStatistics(
    CStatusDefinitions::SeverityLevels::INFO, "Readout", "charlie.nscl.msu.edu",
    123456, 10, "A test title"
  );
  // This should have created a readout_program record and a run_info record
  // linked back to it -- the inner join query below should return both in one
  // fell swoop:
  
  CSqliteStatement getInfo(
    m_db->m_handle,
    "SELECT p.id, p.name, p.host, r.readout_id, r.start, r.run, r.title           \
      FROM readout_program AS p                                             \
      INNER JOIN run_info AS r ON p.id = r.readout_id                       \
    "
  );
  ++getInfo;
  ASSERT(!getInfo.atEnd());
  EQ(1, getInfo.getInt(0));        // p.id
  EQ(
    std::string("Readout"),
    std::string(reinterpret_cast<const char*>(getInfo.getText(1)))
  );                              // p.name
  EQ(
    std::string("charlie.nscl.msu.edu"),
    std::string(reinterpret_cast<const char*>(getInfo.getText(2)))
  );                             // p.host
  EQ(1, getInfo.getInt(3));      // r.readout_id (FK)
  EQ(123456, getInfo.getInt(4)); // r.start
  EQ(10, getInfo.getInt(5));     // r.run
  EQ(
    std::string("A test title"),
    std::string(reinterpret_cast<const char*>(getInfo.getText(6)))
  );                             // r.title
  
  
  ++getInfo;                                   // should only be one record.
  ASSERT(getInfo.atEnd());
}
//   Insert a program and two runs associated with it.

void RdoDbStatTests::programAndTwoRuns()
{
  m_db->addReadoutStatistics(
    CStatusDefinitions::SeverityLevels::INFO, "Readout", "charlie.nscl.msu.edu",
    123456, 10, "A test title"
  );
  m_db->addReadoutStatistics(
    CStatusDefinitions::SeverityLevels::INFO, "Readout", "charlie.nscl.msu.edu",
    124456, 11, "The second run"
  );
  
  // Fetch this out but ordered by the id in the run_info table:
  
  CSqliteStatement getInfo(
    m_db->m_handle,
    "SELECT p.id, p.name, p.host, r.readout_id, r.start, r.run, r.title     \
      FROM readout_program AS p                                             \
      INNER JOIN run_info AS r ON p.id = r.readout_id                       \
      ORDER BY r.id ASC                                                     \
    "
  );
  
  // First record is run 10 .. just spot check the run info.
  
  ++getInfo;
  ASSERT(!getInfo.atEnd());
  
  EQ(1, getInfo.getInt(3));             // r.readout_id
  EQ(123456, getInfo.getInt(4));        // r.start
  EQ(10, getInfo.getInt(5));            // r.run
  EQ(
    std::string("A test title"),
    std::string(reinterpret_cast<const char*>(getInfo.getText(6)))
  );
  
  // Second record is run 11
  
  ++getInfo;
  ASSERT(!getInfo.atEnd());

  EQ(1, getInfo.getInt(3));             // r.readout_id
  EQ(124456, getInfo.getInt(4));        // r.start
  EQ(11, getInfo.getInt(5));            // r.run
  EQ(
    std::string("The second run"),
    std::string(reinterpret_cast<const char*>(getInfo.getText(6)))
  );
  
  
  // there is no third run:
  
  ++getInfo;
  ASSERT(getInfo.atEnd());
  
  
  
}
void RdoDbStatTests::twoprogramswithruns()
{
  m_db->addReadoutStatistics(
    CStatusDefinitions::SeverityLevels::INFO, "Readout", "charlie.nscl.msu.edu",
    123456, 10, "A test title"
  );
  m_db->addReadoutStatistics(                   // Distinct readout program.
    CStatusDefinitions::SeverityLevels::INFO, "Readout", "spdaq20.nscl.msu.edu",
    124456, 11, "The second run"
  );
  
  // Fetch this out but ordered by the id in the run_info table:
  
  CSqliteStatement getInfo(
    m_db->m_handle,
    "SELECT p.id, p.name, p.host, r.readout_id, r.start, r.run, r.title     \
      FROM readout_program AS p                                             \
      INNER JOIN run_info AS r ON p.id = r.readout_id                       \
      ORDER BY r.id ASC                                                     \
    "
  );
  // First we should get the one for Readout@charlie.nscl.msu.edu:
  
  ++getInfo;
  ASSERT(!getInfo.atEnd());
  
  EQ(1, getInfo.getInt(3));      // r.readout_id points back to frist program.
  EQ(123456, getInfo.getInt(4)); // r.start
  EQ(10, getInfo.getInt(5));     // r.run.
  
  // Second we should get one for Readout@spdaq20.nscl.msu.edu
  
  ++getInfo;
  ASSERT(!getInfo.atEnd());
  EQ(2, getInfo.getInt(3));      // r.readout_id points back to second program.
  EQ(124456, getInfo.getInt(4)); // r.start
  EQ(11, getInfo.getInt(5));     // r.run.
  
  
  
  // There should not be a third one:
  
  ++getInfo;
  ASSERT(getInfo.atEnd());
}
// Create a readout program, a run and a statistics entry for it:

void RdoDbStatTests::programwithstats()
{
  CStatusDefinitions::ReadoutStatCounters stats = {
    123466, 10, 150, 100, 1024  
  };
  m_db->addReadoutStatistics(
    CStatusDefinitions::SeverityLevels::INFO, "Readout", "charlie.nscl.msu.edu",
    123456, 10, "A test title", &stats
  );
  // We can get all of the information combined in one grand select:
  
  CSqliteStatement getInfo(
    m_db->m_handle,
    "SELECT r.run, st.run_id, st.readout_id, st.timestamp, st.elapsedtime,    \
            st.triggers, st.events, st.bytes                              \
            FROM readout_statistics AS st                                 \
            INNER JOIN readout_program AS p ON p.id = st.readout_id       \
            INNER JOIN run_info AS r ON r.id = st.run_id                  \
    "
  );
  // The query should give a single row result -- note that if the records
  // were not properly linked, the inner join will result in no records returned.
  
  ++getInfo;
  ASSERT(!getInfo.atEnd());
  
  EQ(10, getInfo.getInt(0));              // r.run
  EQ(1, getInfo.getInt(1));               // st.run_id  -- link to run table
  EQ(1, getInfo.getInt(2));               // st.readout_id - link to readout table
  EQ(123466, getInfo.getInt(3));          // st.timestamp
  EQ(10, getInfo.getInt(4));              // st.elapsedtime.
  EQ(150, getInfo.getInt(5));             // st.triggers.
  EQ(100, getInfo.getInt(6));             // st.events.
  EQ(1024, getInfo.getInt(7));            // st.bytes.
  
  ++getInfo;
  ASSERT(getInfo.atEnd());
}
// Add two statistics messages for one program:

void RdoDbStatTests::programwithtwostats()
{
  CStatusDefinitions::ReadoutStatCounters stats1 = {
    123466, 10, 150, 100, 1024  
  };
  CStatusDefinitions::ReadoutStatCounters stats2 = {
    123476, 20, 200, 175, 2048  
  };
  m_db->addReadoutStatistics(
    CStatusDefinitions::SeverityLevels::INFO, "Readout", "charlie.nscl.msu.edu",
    123456, 10, "A test title", &stats1
  );
  m_db->addReadoutStatistics(
    CStatusDefinitions::SeverityLevels::INFO, "Readout", "charlie.nscl.msu.edu",
    123456, 10, "A test title", &stats2
  );
  
  // Use an inner joing and sort on the statistics id to pull out the data
  // ensuring it has the proper associations:
  
  CSqliteStatement getInfo(
    m_db->m_handle,
    "SELECT r.run, st.run_id, st.readout_id, st.timestamp, st.elapsedtime,    \
            st.triggers, st.events, st.bytes                                  \
            FROM readout_statistics AS st                                     \
            INNER JOIN readout_program AS p ON p.id = st.readout_id           \
            INNER JOIN run_info AS r ON r.id = st.run_id                      \
            ORDER BY st.id ASC                                                \
    "
  );
  
  // First record is the stats1 data:
  
  ++getInfo;
  ASSERT(!getInfo.atEnd());
  
  EQ(10, getInfo.getInt(0));              // r.run
  EQ(1, getInfo.getInt(1));               // st.run_id  -- link to run table
  EQ(1, getInfo.getInt(2));               // st.readout_id - link to readout table
  EQ(123466, getInfo.getInt(3));          // st.timestamp
  EQ(10, getInfo.getInt(4));              // st.elapsedtime.
  EQ(150, getInfo.getInt(5));             // st.triggers.
  EQ(100, getInfo.getInt(6));             // st.events.
  EQ(1024, getInfo.getInt(7));            // st.bytes.
    
  
  // Second record is the stats2 data
  
  ++getInfo;
  ASSERT(!getInfo.atEnd());
  
  EQ(10,   getInfo.getInt(0));              // r.run
  EQ(1,    getInfo.getInt(1));              // st.run_id  -- link to run table
  EQ(1,    getInfo.getInt(2));              // st.readout_id - link to readout table
  EQ(123476, getInfo.getInt(3));            // st.timestamp
  EQ(20,   getInfo.getInt(4));              // st.elapsedtime.
  EQ(200,  getInfo.getInt(5));              // st.triggers.
  EQ(175,  getInfo.getInt(6));              // st.events.
  EQ(2048, getInfo.getInt(7));              // st.bytes.
  
  
  // No third record.
  
  ++getInfo;
  ASSERT(getInfo.atEnd());
  
  
}
// Two programs each with their own statistics records:

void RdoDbStatTests::twoprogramswithstats()
{
  CStatusDefinitions::ReadoutStatCounters stats1 = {
    123466, 10, 150, 100, 1024  
  };
  CStatusDefinitions::ReadoutStatCounters stats2 = {
    123476, 20, 200, 175, 2048  
  };
  m_db->addReadoutStatistics(
    CStatusDefinitions::SeverityLevels::INFO, "Readout", "charlie.nscl.msu.edu",
    123456, 10, "A test title", &stats1
  );
  m_db->addReadoutStatistics(
    CStatusDefinitions::SeverityLevels::INFO, "Readout", "spdaq20.nscl.msu.edu",
    123456, 10, "A test title", &stats2
  );
  
  // Use an inner joing and sort on the statistics id to pull out the data
  // ensuring it has the proper associations:
  
  CSqliteStatement getInfo(
    m_db->m_handle,
    "SELECT r.run, st.run_id, st.readout_id, st.timestamp, st.elapsedtime,    \
            st.triggers, st.events, st.bytes                                  \
            FROM readout_statistics AS st                                     \
            INNER JOIN readout_program AS p ON p.id = st.readout_id           \
            INNER JOIN run_info AS r ON r.id = st.run_id                      \
            ORDER BY st.id ASC                                                \
    "
  );
  
  // First record is the stats1 data:
  
  ++getInfo;
  ASSERT(!getInfo.atEnd());
  
  EQ(10, getInfo.getInt(0));              // r.run
  EQ(1, getInfo.getInt(1));               // st.run_id  -- link to run table
  EQ(1, getInfo.getInt(2));               // st.readout_id - link to readout table
  EQ(123466, getInfo.getInt(3));          // st.timestamp
  EQ(10, getInfo.getInt(4));              // st.elapsedtime.
  EQ(150, getInfo.getInt(5));             // st.triggers.
  EQ(100, getInfo.getInt(6));             // st.events.
  EQ(1024, getInfo.getInt(7));            // st.bytes.
    
  
  // Second record is the stats2 data
  
  ++getInfo;
  ASSERT(!getInfo.atEnd());
  
  EQ(10,   getInfo.getInt(0));              // r.run
  EQ(2,    getInfo.getInt(1));              // st.run_id  -- link to run table
  EQ(2,    getInfo.getInt(2));              // st.readout_id - link to readout table
  EQ(123476, getInfo.getInt(3));            // st.timestamp
  EQ(20,   getInfo.getInt(4));              // st.elapsedtime.
  EQ(200,  getInfo.getInt(5));              // st.triggers.
  EQ(175,  getInfo.getInt(6));              // st.events.
  EQ(2048, getInfo.getInt(7));              // st.bytes.
  
  
  // No third record.
  
  ++getInfo;
  ASSERT(getInfo.atEnd());
  
}