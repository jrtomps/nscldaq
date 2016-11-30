// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"


#define private public
#include "CStatusDb.h"
#undef private
#include "CStatusMessage.h"
#include <CSqlite.h>
#include <CSqliteStatement.h>


class StatusSchemaTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(StatusSchemaTests);
  CPPUNIT_TEST(nowriteEmpty);
  CPPUNIT_TEST(logmsgTable);
  CPPUNIT_TEST(ringbuffer);
  CPPUNIT_TEST(statechange);
  CPPUNIT_TEST(readoutstats);
  CPPUNIT_TEST_SUITE_END();


private:
  CStatusDb* m_pDb;
public:
  void setUp() {
    m_pDb = new CStatusDb(":memory:", CSqlite::readwrite);  //  makes schema.
  }
  void tearDown() {
    delete m_pDb;
  }
protected:
  void nowriteEmpty();
  void logmsgTable();
  void ringbuffer();
  void statechange();
  void readoutstats();
private:
  bool checkIndex(const char* table, const char* index);
  bool checkTable(const char* table);
};

CPPUNIT_TEST_SUITE_REGISTRATION(StatusSchemaTests);

/*-----------------------------------------------------------------
 * utilities:
 */

/**
 * return true if an index exists:
*/
bool StatusSchemaTests::checkIndex(const char* table, const char* index)
{
  CSqliteStatement probe(
      m_pDb->m_handle,
      "SELECT COUNT(*) FROM sqlite_master \
                       WHERE type = 'index' AND tbl_name = ? AND name = ?"
  );
  // Bind the table and index name:
  
  probe.bind(1, table, -1, SQLITE_STATIC);
  probe.bind(2, index, -1, SQLITE_STATIC);
  
  ++probe;
  return (probe.getInt(0) > 0);
}
/**
 * return true if a table exists:
 */
bool StatusSchemaTests::checkTable(const char* table)
{
  CSqliteStatement probe(
    m_pDb->m_handle,
    "SELECT COUNT(*) FROM sqlite_master \
                     WHERE type = 'table' AND name = ?" 
  );
  probe.bind(1, table, -1, SQLITE_STATIC);
  
  
  ++probe;
  return (probe.getInt(0) > 0);
}
/*-----------------------------------------------------------------
 * Tests:
 */

// Opening the database readonly results in no schema generation.

void StatusSchemaTests::nowriteEmpty() {
  CStatusDb test(":memory:", CSqlite::readonly);
  CSqliteStatement probeSchema(test.m_handle, "SELECT COUNT(*)  FROM sqlite_master");
  ++probeSchema;
  EQ(0, probeSchema.getInt(0));
}
// Opening readwrite - generates the log_messages table:

void StatusSchemaTests::logmsgTable()
{
  ASSERT(checkTable("log_messages"));
  ASSERT(checkIndex("log_messages", "idx_log_severity"));
  ASSERT(checkIndex("log_messages", "idx_log_application"));
  ASSERT(checkIndex("log_messages", "idx_log_source"));
  ASSERT(checkIndex("log_messages", "idx_log_timestamp"));
  
}
// Check the ringbuffer schema:

void StatusSchemaTests::ringbuffer()
{
    ASSERT(checkTable("ring_buffer"));
    ASSERT(checkTable("ring_client"));
    ASSERT(checkTable("ring_client_statistics"));
    
    ASSERT(checkIndex("ring_buffer", "ring_name_index"));
    ASSERT(checkIndex("ring_buffer", "ring_host_index"));
    
    ASSERT(checkIndex("ring_client", "ring_client_rid_index"));
    ASSERT(checkIndex("ring_client", "ring_client_pid_index"));
    
    ASSERT(checkIndex("ring_client_statistics", "ring_stats_rid_index"));
    ASSERT(checkIndex("ring_client_statistics", "ring_stats_cid_index"));
    ASSERT(checkIndex("ring_client_statistics", "ring_stats_time_index"));
    
}

// Check the state change schema was created:

void StatusSchemaTests::statechange()
{
  ASSERT(checkTable("state_application"));
  ASSERT(checkTable("state_transitions"));
  
  ASSERT(checkIndex("state_application", "state_application_name_idx"));
  ASSERT(checkIndex("state_application", "state_application_host_idx"));
  
  ASSERT(checkIndex("state_transitions", "state_transition_app_idx"));
  ASSERT(checkIndex("state_transitions", "state_transition_time_idx"));
}

// Check readout statistics schema.

void StatusSchemaTests::readoutstats()
{
  ASSERT(checkTable("readout_program"));
  ASSERT(checkTable("run_info"));
  ASSERT(checkTable("readout_statistics"));
  
  ASSERT(checkIndex("readout_program", "readout_program_name_idx"));
  ASSERT(checkIndex("run_info", "readout_run_start_time_idx"));
  ASSERT(checkIndex("run_info", "readout_run_readout_idx"));
  ASSERT(checkIndex("readout_statistics", "readout_stats_readout_idx"));
  ASSERT(checkIndex("readout_statistics", "readout_stats_time_idx"));
  ASSERT(checkIndex("readout_statistics", "readout_stats_run_idx"));
}