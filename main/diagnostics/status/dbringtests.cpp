// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"


#include "CStatusMessage.h"

#define private public
#include "CStatusDb.h"
#undef private


#include <CSqlite.h>
#include <CSqliteStatement.h>
#include <cstdlib>
#include <cstring>

class RingTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(RingTests);
//  CPPUNIT_TEST(addMinimal);
//  CPPUNIT_TEST(addMinimal2);
//  CPPUNIT_TEST(addMinimalDup);
//  CPPUNIT_TEST(addClient1);
  CPPUNIT_TEST(addClient2);
//  CPPUNIT_TEST(addMultiStat);
  CPPUNIT_TEST_SUITE_END();


private:
  CStatusDb*  m_pDb;
public:
  void setUp() {
    m_pDb = new CStatusDb(":memory:", CSqlite::readwrite |CSqlite::create);
  }
  
  void tearDown() {
    delete m_pDb;
  }
protected:
  void addMinimal();
  void addMinimal2();
  void addMinimalDup();
  void addClient1();
  void addClient2();
  void addMultiStat();
};

CPPUNIT_TEST_SUITE_REGISTRATION(RingTests);

// A minimal insertion has no clients:
// Should, however create a new ring_buffer table entry.

void RingTests::addMinimal() {
  std::vector<const CStatusDefinitions::RingStatClient*> empty;
  CStatusDefinitions::RingStatIdentification*  pRingId  =
    reinterpret_cast<CStatusDefinitions::RingStatIdentification*>(std::malloc(
      sizeof(CStatusDefinitions::RingStatIdentification) +
      std::strlen("testring") + 1
    ));
  try {
    CStatusDefinitions::RingStatIdentification& ringId(*pRingId);
    ringId.s_tod = 1234;
    std::strcpy(ringId.s_ringName, "testring");
    
    m_pDb->addRingStatistics(
      CStatusDefinitions::SeverityLevels::INFO, "ringdaemon",
      "charlie.nscl.msu.edu", ringId, empty
    );
    // See if we can pull the record back out:
    
    CSqliteStatement probe(
      m_pDb->m_handle,
      "SELECT id, name, host FROM ring_buffer"           // Should be the only entry.
    );
    
    ++probe;
    ASSERT(!probe.atEnd());                   // Should be a record.
    
    // Record contents:
    
    EQ(1, probe.getInt(0));                  // id.
    EQ(
      std::string("testring"),
      std::string(reinterpret_cast<const char*>(probe.getText(1)))
    );                                      // name.
    EQ(
      std::string("charlie.nscl.msu.edu"),
      std::string(reinterpret_cast<const char*>(probe.getText(2)))
    );                                      // host
  } catch (...) {
    std::free(pRingId);
    throw;
  }
  std::free(pRingId);
}

//    Add two different ring buffers -- cheat with same name different host.

void RingTests::addMinimal2() {
  std::vector<const CStatusDefinitions::RingStatClient*> empty;
  CStatusDefinitions::RingStatIdentification*  pRingId  =
    reinterpret_cast<CStatusDefinitions::RingStatIdentification*>(std::malloc(
      sizeof(CStatusDefinitions::RingStatIdentification) +
      std::strlen("testring") + 1
    ));
  try {
    CStatusDefinitions::RingStatIdentification& ringId(*pRingId);
    ringId.s_tod = 1234;
    std::strcpy(ringId.s_ringName, "testring");
    
    m_pDb->addRingStatistics(
      CStatusDefinitions::SeverityLevels::INFO, "ringdaemon",
      "charlie.nscl.msu.edu", ringId, empty
    );
    m_pDb->addRingStatistics(
      CStatusDefinitions::SeverityLevels::INFO, "ringdaemon",
      "spdaq20.nscl.msu.edu", ringId, empty
    );                                       // distinct from previous.
    
    // Should have two hits ordered by id gives them in creation order:
    
    CSqliteStatement probe(
      m_pDb->m_handle,
      "SELECT id, name, host FROM ring_buffer ORDER BY id ASC"  // Should be the only entry.
    );
    
    ++probe;
    ASSERT(!probe.atEnd());                   // Should be a record.
    
    // Record contents:
    
    EQ(1, probe.getInt(0));                  // id.
    EQ(
      std::string("testring"),
      std::string(reinterpret_cast<const char*>(probe.getText(1)))
    );                                      // name.
    EQ(
      std::string("charlie.nscl.msu.edu"),
      std::string(reinterpret_cast<const char*>(probe.getText(2)))
    );                                      // host
    
    ++probe;
    ASSERT(!probe.atEnd());                   // Should be a record.
    
    // Record contents:
    
    EQ(2, probe.getInt(0));                  // id.
    EQ(
      std::string("testring"),
      std::string(reinterpret_cast<const char*>(probe.getText(1)))
    );                                      // name.
    EQ(
      std::string("spdaq20.nscl.msu.edu"),
      std::string(reinterpret_cast<const char*>(probe.getText(2)))
    );         
  }
  catch (...) {
    std::free(pRingId);
    throw;
  }
  std::free(pRingId);
  
}

// If I add a pure duplicate, that should be detected and only give me
// a single ring_buffer table entry.

void RingTests::addMinimalDup() {
  std::vector<const CStatusDefinitions::RingStatClient*> empty;
  CStatusDefinitions::RingStatIdentification*  pRingId  =
    reinterpret_cast<CStatusDefinitions::RingStatIdentification*>(std::malloc(
      sizeof(CStatusDefinitions::RingStatIdentification) +
      std::strlen("testring") + 1
    ));
  try {
    CStatusDefinitions::RingStatIdentification& ringId(*pRingId);
    ringId.s_tod = 1234;
    std::strcpy(ringId.s_ringName, "testring");
    
    m_pDb->addRingStatistics(
      CStatusDefinitions::SeverityLevels::INFO, "ringdaemon",
      "charlie.nscl.msu.edu", ringId, empty
    );
    m_pDb->addRingStatistics(
      CStatusDefinitions::SeverityLevels::INFO, "ringdaemon",
      "charlie.nscl.msu.edu", ringId, empty
    );                                       // Dup.
    // Table should only have one entry:
    
    CSqliteStatement probe(
      m_pDb->m_handle,
      "SELECT COUNT(*) FROM ring_buffer"
    );
    ++probe;
    ASSERT(!probe.atEnd());
    EQ(1, probe.getInt(0));
  }
  catch (...) {
    std::free(pRingId);
    throw;
  }
  std::free(pRingId);
  
}
// adding a single client with statistics
//   - populates the ring_buffer table.
//   - populates the ring_client
//   - populates the ring_statistics table.

void RingTests::addClient1()
{
  CStatusDefinitions::RingStatIdentification* pRingId =
    reinterpret_cast<CStatusDefinitions::RingStatIdentification*>(std::malloc(
      strlen("testring") + sizeof(CStatusDefinitions::RingStatIdentification) + 1
  ));
  pRingId->s_tod = 1234;
  std::strcpy(pRingId->s_ringName, "testring");
  
  CStatusDefinitions::RingStatClient* pStats =
    reinterpret_cast<CStatusDefinitions::RingStatClient*>(std::malloc(
      std::strlen("this is a test ") + sizeof(CStatusDefinitions::RingStatClient) + 1
  ));
  pStats->s_operations = 100;
  pStats->s_bytes      = 2048;
  pStats->s_backlog    = 8192;
  pStats->s_pid        = 666;
  pStats->s_isProducer = 0;
  memcpy(
    pStats->s_command, "this\0is\0a\0test\0\0",
    std::strlen("this is a test ") + 1
  );
  std::vector<const CStatusDefinitions::RingStatClient*> clients = {pStats};
  
  try {
    m_pDb->addRingStatistics(
      CStatusDefinitions::SeverityLevels::INFO,
      "ringdaemon", "charlie.nscl.msu.edu",
      *pRingId, clients
    );
    // We think ring buffers get put in properly so just see there's one:
    
    CSqliteStatement countRbuffers(
      m_pDb->m_handle,
      "SELECT COUNT(*) from ring_buffer"
    );
    ++countRbuffers;
    EQ(1, countRbuffers.getInt(0));
    
    // Should be an entry in ring_client
    
    CSqliteStatement fetchClients(
      m_pDb->m_handle,
      "SELECT ring_id, pid, producer, command FROM ring_client"
    );
    ++fetchClients;
    ASSERT(!fetchClients.atEnd());
    EQ(1, fetchClients.getInt(0));                // link to ring id 1.
    EQ(666, fetchClients.getInt(1));              // pid
    EQ(0,   fetchClients.getInt(2));              // not a producer.
    EQ(std::string("this is a test"), std::string(reinterpret_cast<const char*>(fetchClients.getText(3))));
    
    ++fetchClients;
    ASSERT(fetchClients.atEnd());                // There can be only one.
    
    // Should also be statistics linked to the ring and client:
    
    CSqliteStatement fetchStats(
      m_pDb->m_handle,
      "SELECT ring_id, client_id, timestamp, operations, bytes, backlog        \
              FROM ring_client_statistics"
    );
    ++fetchStats;
    ASSERT(!fetchStats.atEnd());
    EQ(1, fetchStats.getInt(0));                 // Link to ring id 1.
    EQ(1, fetchStats.getInt(1));                 // link to client id 1.
    EQ(1234, fetchStats.getInt(2));              // timestamp.
    EQ(100, fetchStats.getInt(3));               // operations.
    EQ(2048, fetchStats.getInt(4));              // bytes.
    EQ(8192, fetchStats.getInt(5));              // backog.
    
    ++fetchClients;                 // only one so:
    ASSERT(fetchClients.atEnd());
    
  }
  catch (...) {
    std::free(pRingId);
    std::free(pStats);
    throw;
  }
  std::free(pRingId);
  std::free(pStats);
}
  // If I add  with a pair of statistics that are for different clients I
  // should get two distinct client and statistics entries.
  
void RingTests::addClient2()
{
  CStatusDefinitions::RingStatIdentification* pRingId =
    reinterpret_cast<CStatusDefinitions::RingStatIdentification*>(std::malloc(
      strlen("testring") + sizeof(CStatusDefinitions::RingStatIdentification) + 1
  ));
  pRingId->s_tod = 1234;
  std::strcpy(pRingId->s_ringName, "testring");
  
  CStatusDefinitions::RingStatClient* pStats =
    reinterpret_cast<CStatusDefinitions::RingStatClient*>(std::malloc(
      std::strlen("this is a test ") + sizeof(CStatusDefinitions::RingStatClient) + 1
  ));
  pStats->s_operations = 100;
  pStats->s_bytes      = 2048;
  pStats->s_backlog    = 8192;
  pStats->s_pid        = 666;
  pStats->s_isProducer = 0;
  memcpy(
    pStats->s_command, "this\0is\0a\0test\0\0",
    std::strlen("this is a test ") + 1
  );
  
  CStatusDefinitions::RingStatClient* pStats2 =
    reinterpret_cast<CStatusDefinitions::RingStatClient*>(std::malloc(
      std::strlen("this is another test ") + sizeof(CStatusDefinitions::RingStatClient) + 1
  ));
  pStats2->s_operations = 123;
  pStats2->s_bytes      = 4567;
  pStats2->s_backlog    = 0;
  pStats2->s_pid        = 111;
  pStats2->s_isProducer = 1;
  memcpy(
    pStats2->s_command, "this\0is\0another\0test\0\0",
    std::strlen("this is another test ") + 1
  );
  
  std::vector<const CStatusDefinitions::RingStatClient*> clients = {pStats, pStats2};
  
  try {
    m_pDb->addRingStatistics(
      CStatusDefinitions::SeverityLevels::INFO, "ringdaemon", "charlie.nscl.msu.edu",
      *pRingId, clients
    );
    
    // There should be 1 ring and two clients:
    
    CSqliteStatement countRings(
      m_pDb->m_handle,
      "SELECT COUNT(*) FROM ring_buffer"
    );
    ++countRings;
    EQ(1, countRings.getInt(0));
    
    CSqliteStatement countClients(
      m_pDb->m_handle,
      "SELECT COUNT(*) FROM ring_client"
    );
    ++countClients;
    EQ(2, countClients.getInt(0));
    
    // Get the ring statistics items -- ordered by ID:
    
    CSqliteStatement fetchStats(
      m_pDb->m_handle,
      "SELECT s.ring_id, client_id, timestamp, operations, bytes, backlog, \
              c.pid, c.producer                                             \
              FROM ring_client_statistics AS s                               \
              INNER JOIN ring_client  AS c                                  \
              ON s.client_id  = c.id                                        \
              ORDER BY s.id ASC"
    );
    // First one is client 1 (not producer).
    
    ++fetchStats;
    ASSERT(!fetchStats.atEnd());
    EQ(1, fetchStats.getInt(0));     //ring_id
    EQ(1, fetchStats.getInt(1));     // client_id
    EQ(1234, fetchStats.getInt(2)); //  timestamp.
    EQ(100, fetchStats.getInt(3));  // operations
    EQ(2048, fetchStats.getInt(4)); // bytes.
    EQ(8192, fetchStats.getInt(5)); // backlog
    EQ(666,  fetchStats.getInt(6)); // c.pid
    EQ(0,    fetchStats.getInt(7)); // c.producer.
    
    // second one is client 2 (producer).
    
    ++fetchStats;
    ASSERT(!fetchStats.atEnd());
    EQ(2, fetchStats.getInt(1));   // Client id.
    EQ(111, fetchStats.getInt(6)); // client PID.
    EQ(1,   fetchStats.getInt(7)); // c.producer.
    
    // There isn't a third one.
    
    ++fetchStats;
    ASSERT(fetchStats.atEnd());
  }
  catch (...) {
    std::free(pStats2);
    std::free(pStats);
    std::free(pRingId);
    throw;
  }
  std::free(pStats2);
  std::free(pStats);
  std::free(pRingId);

}
// We're going to do two add ring stats with the same ring, client
//(differing counter values).  This should give us two statistics linked
// to the same ring buffer and ring client.
void RingTests::addMultiStat()
{
  CStatusDefinitions::RingStatIdentification* pRingId =
    reinterpret_cast<CStatusDefinitions::RingStatIdentification*>(std::malloc(
      strlen("testring") + sizeof(CStatusDefinitions::RingStatIdentification) + 1
  ));
  pRingId->s_tod = 1234;
  std::strcpy(pRingId->s_ringName, "testring");
  
  CStatusDefinitions::RingStatClient* pStats =
    reinterpret_cast<CStatusDefinitions::RingStatClient*>(std::malloc(
      std::strlen("this is a test ") + sizeof(CStatusDefinitions::RingStatClient) + 1
  ));
  pStats->s_operations = 100;
  pStats->s_bytes      = 2048;
  pStats->s_backlog    = 8192;
  pStats->s_pid        = 666;
  pStats->s_isProducer = 0;
  memcpy(
    pStats->s_command, "this\0is\0a\0test\0\0",
    std::strlen("this is a test ") + 1
  );
  std::vector<const CStatusDefinitions::RingStatClient*> clients = {pStats};
  try {
    m_pDb->addRingStatistics(
      CStatusDefinitions::SeverityLevels::INFO, "ringdaemon", "charlie.nscl.msu.edu",
      *pRingId, clients
    );
    pRingId->s_tod = 1235;   // Two seconds later in time:
    pStats->s_operations = 125;
    pStats->s_bytes      = 3000;   // Only counters change:
    pStats->s_backlog    = 1024;
    m_pDb->addRingStatistics(
      CStatusDefinitions::SeverityLevels::INFO, "ringdaemon", "charlie.nscl.msu.edu",
      *pRingId, clients
    );
    
    
    // Should be one ring, one client, two statistics entries.
    
    CSqliteStatement countRings(
      m_pDb->m_handle,
      "SELECT COUNT(*) FROM ring_buffer"
    );
    ++countRings;
    EQ(1, countRings.getInt(0));
    
    CSqliteStatement countClients(
      m_pDb->m_handle,
      "SELECT COUNT(*) FROM ring_client"
    );
    ++countClients;
    EQ(1, countClients.getInt(0));
    
    // -- ensure the links, timestamps and counters are right for the two
    //    entries in ring_client_statistics:
    
    CSqliteStatement loadCounters(
      m_pDb->m_handle,
      "SELECT ring_id, client_id, timestamp, operations, bytes, backlog     \
          FROM ring_client_statistics                                       \
          ORDER BY id"
    );
    ++loadCounters;
    ASSERT(!loadCounters.atEnd());
    
    EQ(1, loadCounters.getInt(0));              // Ring id linkage.
    EQ(1, loadCounters.getInt(1));              // client id linkage.
    EQ(1234, loadCounters.getInt(2));           // timestamp
    EQ(100,  loadCounters.getInt(3));           // operations count.
    EQ(2048, loadCounters.getInt(4));           // bytes transferred.
    EQ(8192, loadCounters.getInt(5));           // backlog
    
    ++loadCounters;
    ASSERT(!loadCounters.atEnd());
    EQ(1, loadCounters.getInt(0));              // Ring id linkage.
    EQ(1, loadCounters.getInt(1));              // client id linkage.
    EQ(1235, loadCounters.getInt(2));           // timestamp.
    EQ(125,  loadCounters.getInt(3));           // operations.
    EQ(3000, loadCounters.getInt(4));           // bytes.
    EQ(1204, loadCounters.getInt(5));           // backlog.

    ++loadCounters;
    ASSERT(loadCounters.atEnd());
    
  }
  catch (...) {
    std::free(pStats);
    std::free(pRingId);
    throw;
  }
  std::free(pStats);
  std::free(pRingId);
}