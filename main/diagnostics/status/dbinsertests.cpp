// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include <zmq.hpp>
#include "Asserts.h"


#define private public
#include "CStatusDb.h"
#undef private

#include<CSqlite.h>
#include <CSqliteStatement.h>


#include <stdexcept>
#include <CStatusMessage.h>
#include <utility>
#include <vector>
#include <cstring>
#include <cstdlib>



class InsertTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(InsertTests);
  CPPUNIT_TEST(emptymesage);
  CPPUNIT_TEST(badtype);
  CPPUNIT_TEST(hdronlyring);
  CPPUNIT_TEST(ringinsert);
  
  CPPUNIT_TEST(hdronlystate);
  CPPUNIT_TEST(extrapartstate);
  CPPUNIT_TEST(stateinsert);
  
  CPPUNIT_TEST(hdronlyrunstat);
  CPPUNIT_TEST(toomanyrunstat);
  CPPUNIT_TEST(runstatinsertonlyinfo);
  CPPUNIT_TEST(runstatinsertstats);
  
  CPPUNIT_TEST(hdronlylog);
  CPPUNIT_TEST(toomanylog);
  CPPUNIT_TEST(loginsert);
  CPPUNIT_TEST_SUITE_END();


private:
CStatusDb* m_db;
public:
  void setUp() {
    m_db = new CStatusDb(":memory:", CSqlite::readwrite | CSqlite::create);
  }
  void tearDown() {
    delete m_db;
  }
protected:
  void emptymesage();
  void badtype();
  void hdronlyring();
  void ringinsert();
  
  void hdronlystate();
  void extrapartstate();
  void stateinsert();
  
  void hdronlyrunstat();
  void toomanyrunstat();
  void runstatinsertonlyinfo();
  void runstatinsertstats();
  
  void hdronlylog();
  void toomanylog();
  void loginsert();
private:
  void fillHeader(
    CStatusDefinitions::Header& hdr, uint32_t type, uint32_t sev,
    const char* app, const char* source
  );
  std::pair<CStatusDefinitions::RingStatIdentification*, size_t> makeRingId(const char* ringName);
  size_t stringListLength(const char* stringList);
  std::pair<CStatusDefinitions::RingStatClient*, size_t> makeRingClient(
    uint64_t operations, uint64_t bytes, uint64_t backlog, uint64_t pid,
    bool producer, const char* commandList
  );
  std::string stringListToString(const char* stringList);
  
  std::pair<CStatusDefinitions::LogMessageBody*, size_t> makeLogbody(const char* msg);
};

// Turn a string list into an std::string:

std::string
InsertTests::stringListToString(const char* stringList)
{
  std::string result;
  bool first(true);
  
  while (*stringList) {
    if(!first) {
      result += " ";                         // space separated.
    } else {
      first = false;
    }
    result += stringList;
    stringList += std::strlen(stringList) + 1;
  }
  return result;
}

// fillHeader - fill in a header struct.

void
InsertTests::fillHeader(
  CStatusDefinitions::Header& hdr, uint32_t type, uint32_t sev,
  const char* app, const char* source
)
{
  hdr.s_type = type;
  hdr.s_severity = sev;
  std::strcpy(hdr.s_application, app);
  std::strcpy(hdr.s_source, source);
}

// Create ring statistics identifier (dynamically) and fill it in.

std::pair<CStatusDefinitions::RingStatIdentification*, size_t>
InsertTests::makeRingId(const char* ringName)
{
  size_t totalSize =
    sizeof(CStatusDefinitions::RingStatIdentification*) + std::strlen(ringName) + 1;
  std::time_t now = std::time(nullptr);
  
  CStatusDefinitions::RingStatIdentification* pResult =
    reinterpret_cast<CStatusDefinitions::RingStatIdentification*>(malloc(totalSize));
  
  pResult->s_tod = now;
  std::strcpy(pResult->s_ringName, ringName);
  
  return std::pair<CStatusDefinitions::RingStatIdentification*, size_t>(pResult, totalSize);
}

// Number of bytes in a string list:

size_t
InsertTests::stringListLength(const char* stringList)
{
  size_t result(0);
  
  while(*stringList) {
    size_t itemSize = std::strlen(stringList);
    result += itemSize;
    stringList += itemSize;
  }
  
  
  return result + 1;                       // +1 for the ending zero.
}

std::pair<CStatusDefinitions::RingStatClient*, size_t>
InsertTests::makeRingClient(
    uint64_t operations, uint64_t bytes, uint64_t backlog, uint64_t pid,
    bool producer, const char* commandList
  )
{
  size_t commandLen= stringListLength(commandList);
  size_t totalSize = sizeof(CStatusDefinitions::RingStatClient) + commandLen;
  
  CStatusDefinitions::RingStatClient* pResult =
    reinterpret_cast<CStatusDefinitions::RingStatClient*>(std::malloc(totalSize));
  
  pResult->s_operations = operations;
  pResult->s_bytes      = bytes;
  pResult->s_backlog    = backlog;
  pResult->s_pid        = pid;
  pResult->s_isProducer = producer ? true : false;
  std::memcpy(pResult->s_command, commandList, commandLen);
    
  return std::pair<CStatusDefinitions::RingStatClient*, size_t>(pResult, totalSize);
}
// makeLogbody -
//   Create a log message body and its size.

std::pair<CStatusDefinitions::LogMessageBody*, size_t>
InsertTests::makeLogbody(const char* msg)
{
  size_t totalSize =
    sizeof(CStatusDefinitions::LogMessageBody) + std::strlen(msg) + 1;
  
  CStatusDefinitions::LogMessageBody* pBody =
    reinterpret_cast<CStatusDefinitions::LogMessageBody*>(std::malloc(totalSize));
  
  pBody->s_tod = std::time(nullptr);
  std::strcpy(pBody->s_message, msg);
  
  return std::pair<CStatusDefinitions::LogMessageBody*, size_t>(pBody, totalSize);
}
CPPUNIT_TEST_SUITE_REGISTRATION(InsertTests);

// Passing an empty message to insert fails.

void InsertTests::emptymesage() {
  std::vector<zmq::message_t*> message;
  CPPUNIT_ASSERT_THROW(
    m_db->insert(message),
    std::length_error
  );
}
// Unrecognized type is invalid argument:

void InsertTests::badtype()
{
  CStatusDefinitions::Header hdr;
  hdr.s_type = 1234;
  hdr.s_severity = CStatusDefinitions::SeverityLevels::INFO;
  std::strcpy(hdr.s_application, "TestApp");
  std::strcpy(hdr.s_source, "charlie.nscl.msu.edu");
  fillHeader(
    hdr, 1234, CStatusDefinitions::SeverityLevels::INFO,
    "TestApp", "charlie.nscl.msu.edu"
  );
  
  std::vector<zmq::message_t*> msg;
  zmq::message_t header(&hdr, sizeof(hdr), nullptr);
  msg.push_back(&header);
  
  CPPUNIT_ASSERT_THROW(
    m_db->insert(msg),
    std::invalid_argument
  );
}
/**
 * hdronlyring
 *   Making a ring requires more than a header:
 */
void InsertTests::hdronlyring()
{
  CStatusDefinitions::Header hdr;
  fillHeader(
    hdr,
    CStatusDefinitions::MessageTypes::RING_STATISTICS,
    CStatusDefinitions::SeverityLevels::INFO,
    "TestApp", "charlie.nscl.msu.edu"
  );
  
  std::vector<zmq::message_t*> msg;
  zmq::message_t header(&hdr, sizeof(hdr), nullptr);
  msg.push_back(&header);
  
  CPPUNIT_ASSERT_THROW(
    m_db->insert(msg),
    std::length_error
  );
}


// Insert a ring buffer statistics with producer and consumer:

void InsertTests::ringinsert()
{
  // Message parts in 'struct form'.
  
  CStatusDefinitions::Header                 hdr;

  
  // Fill them in:
  
  fillHeader(
    hdr, CStatusDefinitions::MessageTypes::RING_STATISTICS,
    CStatusDefinitions::SeverityLevels::INFO, "RingDaemon", "charlie.nscl.msu.edu"
  );
  std::pair<CStatusDefinitions::RingStatIdentification*, size_t> ringId   =
    makeRingId("fox");
  std::pair<CStatusDefinitions::RingStatClient*, size_t> producer = makeRingClient(
    100, 2000, 0, 60, true,
    "$DAQBIN/VMUSBReadout\0--daqconfig=~/config/daq.tcl\0--ctlconfig=/dev/null\0"
  );
  std::pair<CStatusDefinitions::RingStatClient*, size_t> consumer = makeRingClient(
    50, 1000, 1000, 75, false,
    "$DAQBIN/dumper\0--source=tcp://localhost/fox\0--count=10\0"
  );
  // Build the message vector:
  
  std::vector<zmq::message_t*> message;
  
  zmq::message_t hdrPart(&hdr, sizeof(hdr), nullptr);
  message.push_back(&hdrPart);
  
  zmq::message_t idPart(ringId.first, ringId.second, nullptr);
  message.push_back(&idPart);
  
  zmq::message_t prodPart(producer.first, producer.second, nullptr);
  message.push_back(&prodPart);
  
  zmq::message_t consPart(consumer.first, consumer.second, nullptr);
  message.push_back(&consPart);
  
  CPPUNIT_ASSERT_NO_THROW(m_db->insert(message));
  

  
  // Now check the database contents.  The join is getting all information
  // for the ring, the consumer and the statistics in one fell swoop:
  
  CSqliteStatement get(
    m_db->m_handle,
    "SELECT r.id, r.name, r.host,                                 \
            c.ring_id, c.pid, c.producer, c.command,               \
            s.ring_id, s.client_id, s.timestamp, s.operations, s.bytes, s.backlog \
      FROM ring_buffer AS r                                       \
      INNER JOIN ring_client AS c ON c.ring_id = r.id             \
      INNER JOIN ring_client_statistics AS s ON s.client_id = c.id \
      ORDER BY s.id ASC                                           \
    "
  );
  ++get;
  ASSERT(!get.atEnd());              // Should be producer data.
  
  EQ(1, get.getInt(0));               // r.id
  EQ(
    std::string("fox"),
    std::string(reinterpret_cast<const char*>(get.getText(1)))
  );                                // r.name
  EQ(
    std::string("charlie.nscl.msu.edu"),
    std::string(reinterpret_cast<const char*>(get.getText(2)))
  );                                               // r.host
  
  EQ(1, get.getInt(3));                            // c.ring_id must match r.id
  EQ(producer.first->s_pid, static_cast<uint64_t>(get.getInt64(4)));   // c.pid
  EQ(producer.first->s_isProducer, static_cast<uint32_t>(get.getInt(5))); // c.producer
  EQ(
    stringListToString(producer.first->s_command),
    std::string(reinterpret_cast<const char*>(get.getText(6)))
  );
  
  EQ(1, get.getInt(7));                          // s.ring_id
  EQ(1, get.getInt(8));                          // s.client_id
  EQ(ringId.first->s_tod, static_cast<uint64_t>(get.getInt64(9))); // s.timestamp.
  EQ(producer.first->s_operations, static_cast<uint64_t>(get.getInt64(10))); // s.operations.
  EQ(producer.first->s_bytes, static_cast<uint64_t>(get.getInt64(11)));   // s.bytes.
  EQ(producer.first->s_backlog, static_cast<uint64_t>(get.getInt64(12)));   // s.backlog
  
  ++get;
  ASSERT(!get.atEnd());              // should be consumer data.

    EQ(1, get.getInt(0));               // r.id
  EQ(
    std::string("fox"),
    std::string(reinterpret_cast<const char*>(get.getText(1)))
  );                                // r.name
  EQ(
    std::string("charlie.nscl.msu.edu"),
    std::string(reinterpret_cast<const char*>(get.getText(2)))
  );                                               // r.host
  
  EQ(1, get.getInt(3));                            // c.ring_id must match r.id
  EQ(consumer.first->s_pid, static_cast<uint64_t>(get.getInt64(4)));   // c.pid
  EQ(consumer.first->s_isProducer, static_cast<uint32_t>(get.getInt(5))); // c.producer
  EQ(
    stringListToString(consumer.first->s_command),
    std::string(reinterpret_cast<const char*>(get.getText(6)))
  );
  
  EQ(1, get.getInt(7));                          // s.ring_id
  EQ(2, get.getInt(8));                          // s.client_id
  EQ(ringId.first->s_tod, static_cast<uint64_t>(get.getInt64(9))); // s.timestamp.
  EQ(consumer.first->s_operations, static_cast<uint64_t>(get.getInt64(10))); // s.operations.
  EQ(consumer.first->s_bytes, static_cast<uint64_t>(get.getInt64(11)));   // s.bytes.
  EQ(consumer.first->s_backlog, static_cast<uint64_t>(get.getInt64(12)));   // s.backlog

  
  ++get;
  ASSERT(get.atEnd());              // Should only be the two records.
  
  // Clean up our dynamic allocations:
  
  std::free(ringId.first);
  std::free(producer.first);
  std::free(consumer.first);
}

/**
 * hdronlystate
 *    Error to try to insert a state transition for state with only a header:
 */
 void InsertTests::hdronlystate()
{
  std::vector<zmq::message_t*>  message;
  CStatusDefinitions::Header h;
  fillHeader(
    h, CStatusDefinitions::MessageTypes::STATE_CHANGE,
    CStatusDefinitions::SeverityLevels::INFO, "Readout", "charlie.nscl.msu.edu"
  );
  
  zmq::message_t hPart(&h, sizeof(h), nullptr);
  message.push_back(&hPart);
  
  CPPUNIT_ASSERT_THROW(
    m_db->insert(message),
    std::length_error
  );  
}
// Too many message parts is also a length error:

void InsertTests::extrapartstate()
{
  // Just make a message with three headers:
  
  std::vector<zmq::message_t*>  message;
  CStatusDefinitions::Header h;
  fillHeader(
    h, CStatusDefinitions::MessageTypes::STATE_CHANGE,
    CStatusDefinitions::SeverityLevels::INFO, "Readout", "charlie.nscl.msu.edu"
  );
  
  zmq::message_t hPart(&h, sizeof(h), nullptr);
  message.push_back(&hPart);
  message.push_back(&hPart);
  message.push_back(&hPart);
  
  CPPUNIT_ASSERT_THROW(
    m_db->insert(message),
    std::length_error
  );
}
// stateinsert - create a state change entry:

void InsertTests::stateinsert()
{
  CStatusDefinitions::Header h;
  CStatusDefinitions::StateChangeBody b;
  fillHeader(
    h,
    CStatusDefinitions::MessageTypes::STATE_CHANGE,
    CStatusDefinitions::SeverityLevels::INFO,
    "readout", "charlie.nscl.msu.edu"
  );
  b.s_tod = std::time(nullptr);
  std::strcpy(b.s_leaving, "Readying");
  std::strcpy(b.s_entering, "Ready");
  
  std::vector<zmq::message_t*> message;
  zmq::message_t hPart(&h, sizeof(h), nullptr);
  zmq::message_t bPart(&b, sizeof(b), nullptr);
  message.push_back(&hPart);
  message.push_back(&bPart);
  
  CPPUNIT_ASSERT_NO_THROW(
    m_db->insert(message)
  );
  
  // Fetch  the record back out of the db:
  
  CSqliteStatement get(
    m_db->m_handle,
    "SELECT a.id, a.name, a.host, t.app_id, t.timestamp, t.leaving, t.entering \
        FROM state_application AS a                                             \
        INNER JOIN state_transitions AS t ON t.app_id = a.id                    \
    "
  );
  ++get;
  ASSERT(!get.atEnd());
  
  EQ(1, get.getInt(0));                        // a.id
  EQ(
    std::string("readout"),
    std::string(reinterpret_cast<const char*>(get.getText(1)))
  );                                          // a.name
  EQ(
    std::string("charlie.nscl.msu.edu"),
    std::string(reinterpret_cast<const char*>(get.getText(2)))
  );                                          // a.host
  EQ(1, get.getInt(3));                       // s.app_id.
  EQ(b.s_tod, get.getInt64(4));                // s.timestamp
  EQ(
    std::string("Readying"),
    std::string(reinterpret_cast<const char*>(get.getText(5)))
  );                                       // s.leaving
  EQ(
    std::string("Ready"),
    std::string(reinterpret_cast<const char*>(get.getText(6)))
  );                                      // s.entering
  
  
  ++get;
  ASSERT(get.atEnd());
}
// hdronlyrunstat  - trying an insert with only a run stat header throws:

void InsertTests::hdronlyrunstat()
{
  CStatusDefinitions::Header h;
  fillHeader(
    h,
    CStatusDefinitions::MessageTypes::READOUT_STATISTICS,
    CStatusDefinitions::SeverityLevels::INFO,
    "DDAS_Readout", "charlie.nscl.msu.edu"
  );
  
  zmq::message_t m(&h, sizeof(h), nullptr);
  std::vector<zmq::message_t*> message;
  message.push_back(&m);
  
  CPPUNIT_ASSERT_THROW(
    m_db->insert(message),
    std::length_error
  );
  
}
// toomanyrunstat - four message parts also throws:

void InsertTests::toomanyrunstat()
{
  CStatusDefinitions::Header h;
  fillHeader(
    h,
    CStatusDefinitions::MessageTypes::READOUT_STATISTICS,
    CStatusDefinitions::SeverityLevels::INFO,
    "DDAS_Readout", "charlie.nscl.msu.edu"
  );
  
  zmq::message_t m(&h, sizeof(h), nullptr);
  std::vector<zmq::message_t*> message;
  
  // Just put the header in four times:
  
  message.push_back(&m);
  message.push_back(&m);
  message.push_back(&m);
  message.push_back(&m);
  
  
  CPPUNIT_ASSERT_THROW(
    m_db->insert(message),
    std::length_error
  );
}

// runstatinsertonlyinfo - no statistics -- just run info:

void InsertTests::runstatinsertonlyinfo()
{
  CStatusDefinitions::Header h;
  fillHeader(
    h,
    CStatusDefinitions::MessageTypes::READOUT_STATISTICS,
    CStatusDefinitions::SeverityLevels::INFO,
    "DDAS_Readout", "charlie.nscl.msu.edu"
  );
  
  CStatusDefinitions::ReadoutStatRunInfo info;
  info.s_startTime = std::time(nullptr);
  info.s_runNumber = 777;
  std::strcpy(info.s_title, "Some test title");
  
  // build the message vector:
  
  zmq::message_t hPart(&h, sizeof(h), nullptr);
  zmq::message_t iPart(&info, sizeof(info), nullptr);
  
  std::vector<zmq::message_t*> message;
  message.push_back(&hPart);
  message.push_back(&iPart);
  
  CPPUNIT_ASSERT_NO_THROW(
    m_db->insert(message)
  );
  
  // Should have a linked readout_rogram and run_info record but
  // no readout_statistics records:
  
  CSqliteStatement get(
    m_db->m_handle,
    "SELECT a.id, a.name, a.host, r.readout_id, r.start, r.run, r.title       \
        FROM readout_program AS a                                             \
        INNER JOIN run_info AS r ON r.readout_id = a.id                       \
      "
  );
  CSqliteStatement count(
    m_db->m_handle,
    "SELECT COUNT(*) FROM readout_statistics"
  );
  // Check the join - one record comes back:
  
  ++get;
  ASSERT(!get.atEnd());
  
  EQ(1, get.getInt(0));    //a.id
  EQ(
    std::string("DDAS_Readout"),
    std::string(reinterpret_cast<const char*>(get.getText(1)))
  );                                                 // a.name
  EQ(
    std::string("charlie.nscl.msu.edu"),
    std::string(reinterpret_cast<const char*>(get.getText(2)))
  );                                               // a.host
  EQ(1, get.getInt(3));                            // r.readout_id
  EQ(info.s_startTime, get.getInt64(4));           // r.start
  EQ(777, get.getInt(5));                          // r.run
  EQ(
    std::string("Some test title"),
    std::string(reinterpret_cast<const char*>(get.getText(6)))
  );                                              // r.title
  
  ++get;
  ASSERT(get.atEnd());
  
  // Check the count of readout_statistics -- sb. 0:
  
  ++count;
  EQ(0, count.getInt(0));
  
}
// runstatinsertstats - insert readout statistics with statistics.

void InsertTests::runstatinsertstats()
{
  CStatusDefinitions::Header h;
  fillHeader(
    h,
    CStatusDefinitions::MessageTypes::READOUT_STATISTICS,
    CStatusDefinitions::SeverityLevels::INFO,
    "DDAS_Readout", "charlie.nscl.msu.edu"
  );
  
  CStatusDefinitions::ReadoutStatRunInfo info;
  info.s_startTime = std::time(nullptr);
  info.s_runNumber = 777;
  std::strcpy(info.s_title, "Some test title");
  
  CStatusDefinitions::ReadoutStatCounters stats = {
    std::time(nullptr), 10, 100, 75, 2048
  };
  
  // Make the message parts and vector:
  
  zmq::message_t hPart(&h, sizeof(h), nullptr);
  zmq::message_t iPart(&info, sizeof(info), nullptr);
  zmq::message_t sPart(&stats, sizeof(stats), nullptr);
  
  std::vector<zmq::message_t*> message;
  message.push_back(&hPart);
  message.push_back(&iPart);
  message.push_back(&sPart);
  
  CPPUNIT_ASSERT_NO_THROW(
    m_db->insert(message)
  );
  // Check the data:
  
  CSqliteStatement get(
    m_db->m_handle,
    "SELECT a.id, a.name, a.host,                                             \
            r.readout_id, r.start, r.run, r.title,                            \
            s.run_id, s.readout_id, s.timestamp, s.elapsedtime, s.triggers,   \
            s.events, s.bytes                                                 \
      FROM readout_program AS a                                               \
      INNER JOIN run_info AS r ON r.readout_id = a.id                         \
      INNER JOIN readout_statistics AS s                                      \
           ON (s.run_id = r.id) AND (s.readout_id = a.id)                     \
    "
  );
  
  // Only one monster result:
  
  ++get;
  ASSERT(!get.atEnd());
  
  EQ(1, get.getInt(0));                                               // a.id.
  EQ(
     std::string("DDAS_Readout"),
     std::string(reinterpret_cast<const char*>(get.getText(1)))
  );                                                                 // a.name
  EQ(
    std::string("charlie.nscl.msu.edu"),
    std::string(reinterpret_cast<const char*>(get.getText(2)))
  );                                                              // a.host
  EQ(1, get.getInt(3));                                           // r.readout_id
  EQ(info.s_startTime, get.getInt64(4));                           // r.start
  EQ(info.s_runNumber, static_cast<uint32_t>(get.getInt(5)));     // r.run
  EQ(
    std::string(info.s_title),
    std::string(reinterpret_cast<const char*>(get.getText(6)))
  );                                                             // r.title
  EQ(1, get.getInt(7));                                          // s.run_id
  EQ(1, get.getInt(8));                                          // s.readout_id
  EQ(stats.s_tod, get.getInt64(9));                              // s.timestamp
  EQ(stats.s_elapsedTime, static_cast<uint64_t>(get.getInt64(10))); // s.elapsedtime
  EQ(stats.s_triggers, static_cast<uint64_t>(get.getInt64(11))); // s.triggers.
  EQ(stats.s_events, static_cast<uint64_t>(get.getInt64(12)));   // s.events
  EQ(stats.s_bytes, static_cast<uint64_t>(get.getInt64(13)));   // s.bytes.
  
  ++get;
  ASSERT(get.atEnd());
}

// hdronlylog -- LOG MESSAGE header only is invalid:

void InsertTests::hdronlylog()
{
  CStatusDefinitions::Header h;
  fillHeader(
    h,
    CStatusDefinitions::MessageTypes::LOG_MESSAGE,
    CStatusDefinitions::SeverityLevels::INFO,
    "DDAS_Readout", "charlie.nscl.msu.edu"
  );
  
  zmq::message_t hPart(&h, sizeof(h), nullptr);
  
  std::vector<zmq::message_t*> message;
  message.push_back(&hPart);
  
  CPPUNIT_ASSERT_THROW(
    m_db->insert(message),
    std::length_error
  );
}
// toomanylog - log messages can only have two parts:

void InsertTests::toomanylog()
{
  CStatusDefinitions::Header h;
  fillHeader(
    h,
    CStatusDefinitions::MessageTypes::LOG_MESSAGE,
    CStatusDefinitions::SeverityLevels::INFO,
    "DDAS_Readout", "charlie.nscl.msu.edu"
  );
  
  zmq::message_t hPart(&h, sizeof(h), nullptr);
  
  std::vector<zmq::message_t*> message;
  message.push_back(&hPart);
  message.push_back(&hPart);
  message.push_back(&hPart);
  
  CPPUNIT_ASSERT_THROW(
    m_db->insert(message),
    std::length_error
  );
}
// loginsert - insert a good log message and look for it:

void InsertTests::loginsert()
{
  CStatusDefinitions::Header h;
  fillHeader(
    h,
    CStatusDefinitions::MessageTypes::LOG_MESSAGE,
    CStatusDefinitions::SeverityLevels::INFO,
    "DDAS_Readout", "charlie.nscl.msu.edu"
  );
  std::pair<CStatusDefinitions::LogMessageBody*, size_t> body =
    makeLogbody("This is some test message");
    
  // Create the message:
  
  zmq::message_t hPart(&h, sizeof(h), nullptr);
  zmq::message_t bPart(body.first, body.second, nullptr);
  
  std::vector<zmq::message_t*> message;
  message.push_back(&hPart);
  message.push_back(&bPart);
  
  /// do the insertion:
  
  CPPUNIT_ASSERT_NO_THROW(
    m_db->insert(message)
  );
  
  //  Check the insert worked:
  
  CSqliteStatement get(
    m_db->m_handle,
    "SELECT severity, application, source, timestamp, message FROM log_messages"
  );
  
  ++get;
  ASSERT(!get.atEnd());
  
  EQ(
     CStatusDefinitions::severityToString(h.s_severity),
     std::string(reinterpret_cast<const char*>(get.getText(0)))
  );                                                             // severity8.
  EQ(
    std::string(h.s_application),
    std::string(reinterpret_cast<const char*>(get.getText(1)))
  );                                                            // application.
  EQ(
    std::string(h.s_source),
    std::string(reinterpret_cast<const char*>(get.getText(2)))
  );                                                           // source.
  EQ(body.first->s_tod, get.getInt64(3));                      // timestamp
  EQ(
    std::string(body.first->s_message),
    std::string(reinterpret_cast<const char*>(get.getText(4)))
  );                                                        // message
  
  ++get;
  ASSERT(get.atEnd());
  
  free(body.first);                           // Release dynamic storage.
}