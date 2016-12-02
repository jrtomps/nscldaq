/**

#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2013.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Author:
#            Ron Fox
#            NSCL
#            Michigan State University
#            East Lansing, MI 48824-1321

##
# @file   CStatusDb.cpp
# @brief  Implement the status database API.
# @author <fox@nscl.msu.edu>
*/

#include "CStatusDb.h"
#include <CSqlite.h>
#include <CSqliteStatement.h>
#include <CSqliteTransaction.h>
#include "CStatusMessage.h"

#include <cstring>


/**
 *  constructor
 *     Attaches and potentially creates a database file.
 *     If the database is attached writable then, if needed the schema is
 *     created.
 *
 * @param dbSpec - The specification of the database. Note that:
 *                 -  :memory: creates an in memory database.
 *                 -  ""       Creates a database in a temp file.
 *                 -  other - specifies the path to a file that will hold the
 *                            database.
 * @param flags - Database open flags (see constructor for CSqlite)
 */
CStatusDb::CStatusDb(const char* dbSpec, int flags) :
    m_handle(*(new CSqlite(dbSpec, flags))),
    m_pLogInsert(0),
    m_addRingBuffer(0), m_addRingClient(0), m_addRingStats(0),
    m_getRingId(0), m_getClientId(0),
    m_getSCAppId(0), m_addSCApp(0), m_addSC(0),
    m_getReadoutId(0), m_addReadout(0), m_getRunId(0), m_addRun(0),
    m_addRunStats(0)
{
    if (flags &  CSqlite::readwrite) {
        createSchema();
    }
}
/**
 * destructor
 *   Kill off the database handle:
 */
CStatusDb::~CStatusDb() {
    delete m_pLogInsert;
    delete m_addRingBuffer;
    delete m_addRingClient;
    delete m_addRingStats;
    delete m_getRingId;
    delete m_getClientId;
    delete m_getSCAppId;
    delete m_addSCApp;
    delete m_addSC;
    delete m_getReadoutId;
    delete m_addReadout;
    delete m_getRunId;
    delete m_addRun;
    delete m_addRunStats;
    
    delete &m_handle;
}

/**
 * addLogMessage
 *     Adds a new log message to the database.  This is just an insertion into the
 *     log_messages table.
 *
 *   @param severity  - Message severity, should be one of
 *                      CStatusDefinitions::SeverityLevels
 *   @param app       - Application name.
 *   @param src       - Where the message came from (fqdn).
 *   @param time      - time_t at which the message was emitted.
 *   @param message   - message text.
 */
void
CStatusDb::addLogMessage(uint32_t severity, const char* app, const char* src,
                            int64_t time, const char* message)
{
    // only parse this insert once:
    
    if (!m_pLogInsert) {
        m_pLogInsert = new CSqliteStatement(
            m_handle,
            "INSERT INTO log_messages                                           \
                (severity, application, source, timestamp, message)            \
                VALUES (?,?,?,?,?)"
        );
    }
    // single insert is atomic so no need for a transaction.
    
    m_pLogInsert->bind(
        1, CStatusDefinitions::severityToString(severity).c_str(),
        -1, SQLITE_STATIC
    );
    m_pLogInsert->bind(2, app, -1, SQLITE_STATIC);
    m_pLogInsert->bind(3, src, -1, SQLITE_STATIC);
    m_pLogInsert->bind(4, time);
    m_pLogInsert->bind(5, message, -1, SQLITE_STATIC);
    
    ++(*m_pLogInsert);
    m_pLogInsert->reset();

}
/**
 * addRingStatistics
 *    Add a ring statistics entry.  If need be, the ring identification
 *    and ring client records are created for the entry.
 *
 *   @param severity - Severity of the message (ought to be info).
 *   @param app      - Application that emitted the message.
 *   @param src      - host that emitted the message (where the ring lives).
 *   @param ringId   - Ring identifiation message part struct.
 *   @param clients  - Vector of ring client statistics message parts.
 */
void
CStatusDb::addRingStatistics(
    uint32_t severity, const char* app, const char* src,
    const CStatusDefinitions::RingStatIdentification& ringInfo,
    const std::vector<CStatusDefinitions::RingStatClient*>& clients
)
{
    // all of this is done as a transaction:
    
    CSqliteTransaction t(m_handle);
    
    try {
        // Get the id of the ring buffer or create it if it does not yet
        // exist.
        
        int ringId = getRingId(ringInfo.s_ringName, src);
        if (ringId == -1) {
            ringId = addRingBuffer(ringInfo.s_ringName, src);
        }
        
        for (int i = 0; i < clients.size(); i++) {
            const CStatusDefinitions::RingStatClient& client(*(clients[i]));
            int clientId = getRingClientId(ringId, client);
            if (clientId == -1) {
                clientId = addRingClient(ringId, client);
            }
            addRingClientStatistics(ringId, clientId, ringInfo.s_tod, client);
        }
    }
    catch(...) {
        t.rollback();            // Exceptions fail the inserts.
        throw;                   // propagate the exception upwards.
    }
    // destruction of t commits the changes.
}
/**
 * addStateChange
 *    Add a state change record.  If necessary a state change application is
 *    added.  The state change from/to states are then linked to that application.
 *    The strategy is very much like what's done with ringbuffers.
 *
 *  @param severity   - Severity of the operation (ignored).
 *  @param app        - Name of the app that emitted the message.
 *  @param src        - System in which app is running.
 *  @param tod        - Unix timestamp indicating when the messages was emitted.
 *  @param from       - State being left.
 *  @param to         - New state transtioned to.l
 */
void
CStatusDb::addStateChange(
    uint32_t severity, const char* app, const char* src,
    int64_t  tod, const char* from, const char* to    
)
{
    CSqliteTransaction t(m_handle);
    try {
        int appId = getStateChangeAppId(app, src);
        if (appId == -1) {
            appId = addStateChangeApp(app, src);
        }
        addStateChange(appId, tod, from, to);
    }
    catch (...) {
        t.rollback();
        throw;
    }
}
/**
 * addReadoutStatistics
 *    Adds readout statistics entries. If necessary adds an entry to the
 *    readout_program table for new application/host pairs.
 *    If necessary, add a new run_info record, for a new bit of run information.
 *    If provided add an entry to the readout_statistics table.
 *
 * @param severity   - Message severity (INFO).
 * @param app        - Name of application emitting the message.
 * @param src        - FQDN of h ost emitting the message.
 * @param start      - unix timestamp for the start of the run.
 * @param runNumber  - Number of the run.
 * @param title      - Run title string.
 * @param pCounters  - Pointer (possibly null) to the counters record.
 *                     If null, no counter record is produced.
 */
void
CStatusDb::addReadoutStatistics(
        uint32_t severity, const char* app, const char* src,
        int64_t startTime, uint32_t runNumber, const char* title,
        const CStatusDefinitions::ReadoutStatCounters* pCounters
)
{
        
    
    CSqliteTransaction t(m_handle);
    try {
        // If necessary add the readout_program:
        
        int pgmId = getReadoutProgramId(app, src);
        if (pgmId == -1) {
            pgmId = addReadoutProgram(app, src);
        }
        
        // If necessary add the run id:
        
        int runId = getRunInfoId(pgmId, runNumber, title, startTime);
        if (runId == -1) {
            runId = addRunInfo(pgmId, runNumber, title, startTime);
        }
        // Add the statistics:
        
        if (pCounters) {
            addRdoStats(
                pgmId, runId, pCounters->s_tod, pCounters->s_elapsedTime,
                pCounters->s_triggers, pCounters->s_events, pCounters->s_bytes
            );
        }
    }
    catch (...) {
        t.rollback();
        throw;
    }
}
/*------------------------------------------------------------------------------
 * private utilities
 */

/**
 * createSchema
 *    Creates the database schema (tables indices etc).   This these are all done
 *    via CREATE objtype IF EXISTS so it's safe to run this on databases where the
 *    required tables, indices etc. already exist.
 */
void
CStatusDb::createSchema()
{
    // Log message table is just a set of fields.  All interesting fields are
    // indexed:
    
    CSqliteStatement::execute(
        m_handle,
        "CREATE TABLE IF NOT EXISTS log_messages (          \
            id          INTEGER PRIMARY KEY AUTOINCREMENT,   \
            severity    TEXT(10),                            \
            application TEXT(32),                            \
            source      TEXT(128),                           \
            timestamp   INTEGER,                             \
            message     TEXT                                \
        )"
    );

    CSqliteStatement::execute(
        m_handle,
        "CREATE INDEX IF NOT EXISTS                         \
                idx_log_severity ON log_messages (severity)"
    );
    CSqliteStatement::execute(
        m_handle,
        "CREATE INDEX IF NOT EXISTS                         \
                idx_log_application ON log_messages (application)"
    );
    CSqliteStatement::execute(
        m_handle,
        "CREATE INDEX IF NOT EXISTS                         \
            idx_log_source ON  log_messages (source)"
    );
    CSqliteStatement::execute (
        m_handle,
        "CREATE INDEX IF NOT EXISTS                         \
            idx_log_timestamp ON log_messages (timestamp)"
    );
    
    // Ring buffer statistics - there's a ring_buffer table
    // a consumer table and a statistics table that contains statistics
    // that update.
    
    CSqliteStatement::execute(
        m_handle,
        "CREATE TABLE IF NOT EXISTS  ring_buffer (              \
            id        INTEGER PRIMARY KEY AUTOINCREMENT,         \
            name      TEXT(64),                                  \
            host      TEXT(32)                                  \
        )"
    );
        // Ring clients are associated with rings.
        
    CSqliteStatement::execute(
        m_handle,
        "CREATE TABLE IF NOT EXISTS ring_client (               \
            id        INTEGER PRIMARY KEY AUTOINCREMENT,         \
            ring_id   INTEGER,                                  \
            pid       INTEGER,                                   \
            producer  INTEGER,                                   \
            command   TEXT,                                      \
            FOREIGN KEY (ring_id) REFERENCES ring_buffer (id)      \
        )"
    );
        // statistics are on rings and are also associated with
        // clients.  While the additional ring_id FK is not a necessity
        // it helps produce double joins and also supports statistics
        // aggregation over a ring.
    
    CSqliteStatement::execute(
        m_handle,
        "CREATE TABLE IF NOT EXISTS ring_client_statistics (    \
            id      INTEGER PRIMARY KEY AUTOINCREMENT,           \
            ring_id INTEGER ,                                    \
            client_id INTEGER,                                   \
            timestamp INTEGER,                                   \
            operations INTEGER,                                  \
            bytes   INTEGER,                                     \
            backlog INTEGER,                                     \
            FOREIGN KEY (ring_id) REFERENCES ring_buffer (id),   \
            FOREIGN KEY (client_id) REFERENCES ring_client (id)  \
        )"
    );
        // Ring buffer indices:
    
    CSqliteStatement::execute(
        m_handle,
        "CREATE INDEX IF NOT EXISTS ring_name_index             \
            ON ring_buffer (name)"
    );
    CSqliteStatement::execute(
        m_handle,
        "CREATE INDEX IF NOT EXISTS ring_host_index             \
            ON ring_buffer (host)"    
    );
    
    CSqliteStatement::execute(
        m_handle,
        "CREATE INDEX IF NOT EXISTS ring_client_rid_index       \
            ON ring_client (ring_id)"
    );
    CSqliteStatement::execute(
        m_handle,
        "CREATE INDEX IF NOT EXISTS ring_client_pid_index       \
            ON ring_client (pid)"  
    );
    
    CSqliteStatement::execute(
        m_handle,
        "CREATE INDEX IF NOT EXISTS ring_stats_rid_index        \
            ON ring_client_statistics (ring_id)"
    );
    CSqliteStatement::execute(
        m_handle,
        "CREATE INDEX IF NOT EXISTS ring_stats_cid_index        \
            ON ring_client_statistics (client_id)"
    );
    CSqliteStatement::execute(
        m_handle,
        "CREATE INDEX IF NOT EXISTS ring_stats_time_index       \
            ON ring_client_statistics (timestamp)"
    );
    
    // State change schema:
    
    CSqliteStatement::execute(
        m_handle,
        "CREATE TABLE IF NOT EXISTS state_application (         \
            id          INTEGER PRIMARY KEY AUTOINCREMENT,       \
            name        TEXT(32),                                \
            host        TEXT(128)                               \
        )"
    );
    CSqliteStatement::execute(
        m_handle,
        "CREATE TABLE IF NOT EXISTS state_transitions (         \
            id          INTEGER PRIMARY KEY AUTOINCREMENT,       \
            app_id      INTEGER,                                \
            timestamp   INTEGER,                                 \
            leaving     TEXT(32),                                \
            entering    TEXT(32),                                \
            FOREIGN KEY (app_id) REFERENCES state_application (id) \
        )"
    );
            // Indices for state changes:
            
    CSqliteStatement::execute(
        m_handle,
        "CREATE INDEX IF NOT EXISTS  state_application_name_idx \
            ON state_application (name)"
    );
    CSqliteStatement::execute(
        m_handle,
        "CREATE INDEX IF NOT EXISTS state_application_host_idx    \
            ON state_application (host)"
    );
    
    CSqliteStatement::execute(
        m_handle,
        "CREATE INDEX IF NOT EXISTS state_transition_app_idx     \
            ON state_transitions (app_id)"
    );
    CSqliteStatement::execute(
        m_handle,
        "CREATE INDEX IF NOT EXISTS state_transition_time_idx   \
            ON state_transitions (timestamp)"
    );
    
    // Readout Statistics schema.
    
    CSqliteStatement::execute(
        m_handle,
        "CREATE TABLE IF NOT EXISTS readout_program (           \
            id      INTEGER PRIMARY KEY AUTOINCREMENT,           \
            name    TEXT(32),                                    \
            host    TEXT(128)                                  \
        )"
    );
    CSqliteStatement::execute(
        m_handle,
        "CREATE TABLE IF NOT EXISTS  run_info (                 \
            id      INTEGER PRIMARY KEY AUTOINCREMENT,           \
            readout_id INTEGER,                                  \
            start   INTEGER,                                     \
            run     INTEGER,                                     \
            title   TEXT(80),                                    \
            FOREIGN KEY (readout_id) REFERENCES readout_program (id) \
        )"
    );
    CSqliteStatement::execute(
        m_handle,
        "CREATE TABLE IF NOT EXISTS readout_statistics (        \
            id      INTEGER PRIMARY KEY AUTOINCREMENT,           \
            run_id  INTEGER,                                    \
            readout_id INTEGER,                                 \
            timestamp   INTEGER,                                 \
            elapsedtime INTEGER,                                 \
            triggers    INTEGER,                                 \
            events  INTEGER,                                     \
            bytes   INTEGER,                                     \
            FOREIGN KEY (run_id) REFERENCES run_info (id),         \
            FOREIGN KEY (readout_id) REFERENCES readout_program (id) \
        )"
    );
    
        // Keys for run statistics schema.
        
    CSqliteStatement::execute(
        m_handle,
        "CREATE INDEX IF NOT EXISTS readout_program_name_idx   \
            ON readout_program (name)"   
    );
    CSqliteStatement::execute(
        m_handle,
        "CREATE INDEX IF NOT EXISTS readout_run_start_time_idx \
            ON run_info (start)"
    );
    CSqliteStatement::execute(
        m_handle,
        "CREATE INDEX IF NOT EXISTS readout_run_readout_idx    \
            ON run_info (readout_id)"
    );
    CSqliteStatement::execute(
        m_handle,
        "CREATE INDEX IF NOT EXISTS readout_stats_run_idx       \
            ON readout_statistics (run_id)"
    );
    CSqliteStatement::execute(
        m_handle,
        "CREATE INDEX IF NOT EXISTS readout_stats_readout_idx   \
            ON readout_statistics (readout_id)"
    );
    CSqliteStatement::execute(
        m_handle,
        "CREATE INDEX IF NOT EXISTS readout_stats_time_idx      \
            ON readout_statistics (timestamp)"
    );
}
/**
 * getRingId
 *   Returns the primary key (ring id) of a ring buffer.  This is
 *   used to link items to that record (e.g. set FK fields in related
 *   tables).
 *
 *  @param name - Name of the ring buffer.
 *  @param host - Host in which the ringbuffer lives.
 *  @return int - If the ringbuffer exists, returns its primary key
 *  @retval -1 if the ringbuffer does not exist.
 *  @note - if the m_getRingId statement does not exist is is created here.
 *          that's a cached statement with slots into which the ring name
 *          and host can be plugged for the query.
 */
int
CStatusDb::getRingId(const char* name, const char* host)
{
    int result = -1;               // Assume there's no match.
    
    if (!m_getRingId) {
        m_getRingId = new CSqliteStatement(
            m_handle,
            "SELECT id FROM ring_buffer WHERE name = ? AND host = ?"
        );
    }
    m_getRingId->bind(1, name, -1, SQLITE_STATIC);
    m_getRingId->bind(2, host, -1, SQLITE_STATIC);
    
    // Step the statement - we'll be at end if there's no rows:
    
    ++(*m_getRingId);
    if (!m_getRingId->atEnd()) {                 // Got a match
        result = m_getRingId->getInt(0);
    }
    m_getRingId->reset();                        // So we can re-use the statement.
    
    return result;
}
/**
 * addRingBuffer
 *   Add a new ring buffer item to the ring_buffer table.
 *
 *  @param name - name of the ring buffer.
 *  @param host - host in which the ring buffer lives.
 *  @return int - Id of the created ring buffer.
 *  @note the m_addRingBuffer saved/prepared statement is created if needed.
 */
int
CStatusDb::addRingBuffer(const char* name, const char* host)
{
    if (!m_addRingBuffer) {
        m_addRingBuffer = new CSqliteStatement(
            m_handle,
            "INSERT INTO ring_buffer (name, host) VALUES (?,?)"
        );
    }
    // Bind the parameters, execute the statement and return the generated id:
    
    m_addRingBuffer->bind(1, name, -1, SQLITE_STATIC);
    m_addRingBuffer->bind(2, host, -1, SQLITE_STATIC);
    
    ++(*m_addRingBuffer);
    int result = m_addRingBuffer->lastInsertId();
    m_addRingBuffer->reset();                       // This might wipe out last id.
    
    return result;
}
/**
 * getRingClientId
 *   Returns the id (primary key) of the specified ring buffer client record
 *   (ring_client) table.
 *
 *  @param ringid - primary key of the ring buffer this is a client of.
 *  @param client - The message part that describes the client.
 *  @return int   - Primary key of the client if found.
 *  @retval -1    - No such record.
 *  @note m_getClientId is created (saved/prepared statement) if necessary.
 */
int
CStatusDb::getRingClientId(
    int ringId, const CStatusDefinitions::RingStatClient& client
)
{
    int result = -1;
    
    if (!m_getClientId) {
        m_getClientId = new CSqliteStatement(
            m_handle,
            "SELECT id FROM ring_client                                  \
                       WHERE ring_id = ? AND pid = ? AND command = ?    \
            "
        );
    }
    // We need to marshall the client's command into a string.
    // By convention the string is space separated words.
    
    std::string command = marshallWords(client.s_command);
    
    // Bind the query parameters.
    
    m_getClientId->bind(1, ringId);
    m_getClientId->bind(2, client.s_pid);
    m_getClientId->bind(3, command.c_str(), -1, SQLITE_STATIC);
    
    // See if we have a match and pull out the id if so:
    
    ++(*m_getClientId);
    if (!m_getClientId->atEnd()) {   // Match
        result = m_getClientId->getInt(0);
    }
    m_getClientId->reset();
    
    return result;
}
/**
 * int addRingClient
 *    Add a ring_client record.  These records are linked via ring_id to
 *    entries in the ring_buffer table.
 *
 *  @param ringId  - ring_id foreign key value for the new record.
 *  @param client  - ring client message part.
 *  @return int    - Primary key of the inserted record.
 *  @note If neceersary the m_addRingClient prepared statement is created.
 *        for this and future use.
 */
int
CStatusDb::addRingClient(
    int ringId, const CStatusDefinitions::RingStatClient& client
)
{
    if (!m_addRingClient) {
        m_addRingClient = new CSqliteStatement(
            m_handle,
            "INSERT INTO ring_client (ring_id, pid, producer, command) \
                    VALUES (?, ?, ?, ?)"
        );
    }
    // Turn the command into a string that can be bound:
    
    std::string command = marshallWords(client.s_command);
    
    // Bind the prepared statement parameters and run the insert:
    
    m_addRingClient->bind(1, ringId);
    m_addRingClient->bind(2, static_cast<int>(client.s_pid));
    m_addRingClient->bind(3, client.s_isProducer ? 1 : 0);
    m_addRingClient->bind(4, command.c_str(), -1, SQLITE_STATIC);
    
    ++(*m_addRingClient);
    int result = m_addRingClient->lastInsertId();
    m_addRingClient->reset();
    
    return result;

}
/**
 * addRingClientStatistics
 *    Adds a new set of client statistics associated with both a client and
 *    a ring buffer.  While technically we only need foreign keys to the client,
 *    containing a FK for the ring allows us to be a joint table for the ring/client
 *    correspondence for queries that benefit from that so...
 *
 *  @param ringId - primary key of the ring buffer that this statistics
 *                  entry is for.
 *  @param clientId - primary key for the client that this statistics entry is for.
 *  @param client   - the full client message part.
 *  @param timestamp - time stamp for the statistics record.
 *  @return int     - Primary key of the added record.
 *  @note   if needed, the m_addRingStats prepared statement is created.
 */
int
CStatusDb::addRingClientStatistics(
    int ringId, int clientId, uint64_t timestamp,
    const CStatusDefinitions::RingStatClient& client
)
{
    if (!m_addRingStats) {
        m_addRingStats = new CSqliteStatement(
            m_handle,
            "INSERT INTO ring_client_statistics                                    \
            (ring_id, client_id, timestamp, operations, bytes, backlog)     \
            VALUES (?,?,?,?,?,?)                                            \
            "
        );
    }
    // Bind the parameters to the prepared statement:
    
    m_addRingStats->bind(1, ringId);
    m_addRingStats->bind(2, clientId);
    m_addRingStats->bind(3, static_cast<int>(timestamp));
    m_addRingStats->bind(4, static_cast<int>(client.s_operations));
    m_addRingStats->bind(5, static_cast<int>(client.s_bytes));
    m_addRingStats->bind(6, static_cast<int>(client.s_backlog));
    
    ++(*m_addRingStats);
    int result = m_addRingStats->lastInsertId();
    m_addRingStats->reset();
    
    return result;
}
/**
 * getStateChangeAppId
 *   Return the id (primary key value) of the specified state change application.
 *
 * @param name  - Name of the application.
 * @param host  - Host in which the application is running.
 * @return int  - ID of the app.
 * @retval -1   - App did not exist.
 */
int
CStatusDb::getStateChangeAppId(const char* appName, const char* host)
{
    int result = -1;
    // If necessary, prepare the statement and save it in m_getSCAppId:
    
    if (!m_getSCAppId) {
        m_getSCAppId = new CSqliteStatement(
            m_handle,
            "SELECT id FROM state_application WHERE name = ? and host = ?"
        );
    }
    // Bind the new parameters to the query:
    
    m_getSCAppId->bind(1, appName, -1, SQLITE_STATIC);
    m_getSCAppId->bind(2, host,    -1, SQLITE_STATIC);
    
    // return the match if there is one:
    
    ++(*m_getSCAppId);
    if (!m_getSCAppId->atEnd()) {
        result = m_getSCAppId->getInt(0);
    }
    
    m_getSCAppId->reset();                   // Make the statement re-usable.
    return result;
}
/**
 * addStateChangeApp
 *   Add a new state change application to the state_appliction table.
 *
 * @param appName - name of the application.
 * @param host    - host in which the application runs.
 * @return int    - the id (Primary key) of the newly added record.
 * @note If necessary, the m_addSCApp prepared statement is created to
 *                  support this query.
 */
int
CStatusDb::addStateChangeApp(const char* appName, const char* host)
{
    if (!m_addSCApp) {
        m_addSCApp = new CSqliteStatement(
            m_handle,
            "INSERT INTO state_application (name, host)                      \
                    VALUES (?, ?)"
        );
    }
    // Bind the parameters to the query:
    
    m_addSCApp->bind(1, appName, -1, SQLITE_STATIC);
    m_addSCApp->bind(2, host,    -1, SQLITE_STATIC);
    
    ++(*m_addSCApp);
    int id = m_addSCApp->lastInsertId();
    
    m_addSCApp->reset();
    
    return id;
}
/**
 * addStateChange
 *   Add a state change record to the state_change_transitions table.
 *   state changes are linked back to applications in the state_application table.
 *
 *  @param appId   - ID of the application that made the state change.
 *  @param timestamp - Time at which the state change occured.
 *  @param from      - The state being left.
 *  @param to        - The new state being entered.
 *  @return int      - Primary key of the added record (id).
 *  @note if m_addSC is null a prepared insert query is created and saved in it
 *                   and used in this and future calls.
 */
int
CStatusDb::addStateChange(
    int appId, int64_t timestamp, const char* from, const char* to
)
{
    if (!m_addSC) {
        m_addSC = new CSqliteStatement(
            m_handle,
            "INSERT INTO state_transitions (app_id, timestamp, leaving, entering) \
                    VALUES (?, ?, ?, ?)"
        );
    }
    // Bind the parameters and perform the query.
    
    m_addSC->bind(1, appId);
    m_addSC->bind(2, static_cast<int>(timestamp));
    m_addSC->bind(3, from, -1, SQLITE_STATIC);
    m_addSC->bind(4, to,   -1, SQLITE_STATIC);
    
    ++(*m_addSC);
    
    // Fetch the id of the added record and make the query useable next time
    
    int id = m_addSC->lastInsertId();
    m_addSC->reset();
    
    return id;
}
/**
 * getReadoutProgramId
 *    Returns the id (primary key value) of a specific readout program.
 *
 * @param app   - Name of the application.
 * @param src   - FQDN of the host it is running in.
 * @return int  - Primary key of the associated record.
 * @retval  -1  - No match
 */
int
CStatusDb::getReadoutProgramId(const char* app, const char* src)
{
    // if necessary create the prepared statement and save it in m_getReadoutId
    
    if (!m_getReadoutId) {
        m_getReadoutId = new CSqliteStatement(
            m_handle,
            "SELECT id FROM readout_program WHERE name = ? AND host  = ?"
        );
    }
    // Bind the search parameters:
    
    m_getReadoutId->bind(1, app, -1, SQLITE_STATIC);
    m_getReadoutId->bind(2, src, -1, SQLITE_STATIC);
    
    ++(*m_getReadoutId);
    int result = -1;
    if (!m_getReadoutId->atEnd()) {
        result = m_getReadoutId->getInt(0);
    }
    
    m_getReadoutId->reset();               // Make the statement ready to reuse.
    
    return result;
}
/**
 * addReadoutProgram
 *    Adds a new record to the readout_program table.
 *
 * @param name   - name of the program/application.
 * @param src    - host in which the program/application runs.
 * @return int   - Id (Primary key value) of the new record.
 */
int
CStatusDb::addReadoutProgram(const char* app, const char* src)
{
    // If necessary create the insertion prepared statement and save it in
    // m_addReadout
    
    if (!m_addReadout) {
        m_addReadout = new CSqliteStatement(
            m_handle,
            "INSERT INTO readout_program (name, host) VALUES (?,?)"
        );
    }
    // bind the query parameters:
    
    m_addReadout->bind(1, app, -1, SQLITE_STATIC);
    m_addReadout->bind(2, src, -1, SQLITE_STATIC);
    
    ++(*m_addReadout);
    
    int result = m_addReadout->lastInsertId();
    m_addReadout->reset();
    return result;
}
/**
 * getRunInfoId
 *    Returns the id (primary key value) of a run identification record.
 *    These records represent data taking runs.
 *
 *  @param rdoId   - Id of the readout program for which this run began.
 *  @param runNumber - Number of the run.
 *  @param title    - Run title string.
 *  @param startTime - UNIX timestamp describing when the run started.
 *  @return int - id of matching record.
 *  @retval -1  - No record matched.
 */
int
CStatusDb::getRunInfoId(
    int rdoId, int runNumber, const char* title, int64_t startTime
)
{
    // If necessary create the query and save it in m_getRunId
    
    if (!m_getRunId) {
        m_getRunId = new CSqliteStatement(
            m_handle,
            "SELECT id FROM run_info                        \
                WHERE readout_id = ? AND start = ?  AND run = ? AND title = ?"
        );
    }
    // Bind the parameters to the statement:
    
    m_getRunId->bind(1, rdoId);
    m_getRunId->bind(2, static_cast<int>(startTime));
    m_getRunId->bind(3, runNumber);
    m_getRunId->bind(4, title, -1, SQLITE_STATIC);
    
    // Run the query and see what falls out:
    
    ++(*m_getRunId);
    int result = -1;
    if(!m_getRunId->atEnd()) {
        result = m_getRunId->getInt(0);
    }
    m_getRunId->reset();
    
    return result;
}
/**
 * addRunInfo
 *    Adds information associated with a run in a specific readout program.
 *
 *  @param rdoId   - Id of the program that initiated the run.
 *  @param runNumber - run n umber.
 *  @param title     - Run title.
 *  @param startTime - Time at which the run started.
 *  @return int - id (primary key value) of the new record).
 */
int
CStatusDb::addRunInfo(
    int rdoId, int runNumber, const char* title, int64_t startTime
)
{
    // If necessary create the prepared query and stor it in m_addRun:
    
    if (!m_addRun) {
        m_addRun = new CSqliteStatement(
            m_handle,
            "INSERT INTO run_info (readout_id, start, run, title)       \
                    VALUES (?, ?, ?, ?)"  
        );
    }
    // Bind the parameters to the query:'
    
    m_addRun->bind(1, rdoId);
    m_addRun->bind(2, static_cast<int>(startTime));
    m_addRun->bind(3, runNumber);
    m_addRun->bind(4, title, -1, SQLITE_STATIC);
    
    // Run the query and return the inserted record's id
    
    ++(*m_addRun);
    int result = m_addRun->lastInsertId();
    m_addRun->reset();
    
    return result;
}
/**
 * addRdoStats
 *    Add a readout statistics record to the readout_statistics table.  This
 *    record is associated both with a run and a readout program.
 *
 *  @param readoutId   - Id of the readout program that created the statistics.
 *  @param runId       - Id of the run for which the record was emitted.
 *  @param timestamp   - Time at which the entry was emitted.
 *  @param elapsedTime - Number of seconds into the run at which the record was emitted.
 *  @param triggers    - Number of triggers processed by the readout program.
 *  @param events      - Number of events generated by the readout program.
 *  @param bytes       - Number of bytes of event data emitted by the program.
 *  @return int   - Id (Primary key value) of the inserted record.
 */
int
CStatusDb::addRdoStats(
    int readoutId, int runId, int64_t timestamp, int64_t elapsedTime,
    int64_t triggers, int64_t events, int64_t bytes
)
{
    // if necessary prepare the statement and save it in m_addRunStats.
    
    if (!m_addRunStats) {
        m_addRunStats = new CSqliteStatement(
            m_handle,
            "INSERT INTO readout_statistics                                 \
                (run_id, readout_id, timestamp,                             \
                elapsedtime, triggers, events, bytes)                       \
                VALUES (?, ?, ?, ?, ?, ?, ?)                                \
            "
        );
    }
    // Bind the parameters and run the query:
    
    m_addRunStats->bind(1, readoutId);
    m_addRunStats->bind(2, runId);
    m_addRunStats->bind(3, static_cast<int>(timestamp));
    m_addRunStats->bind(4, static_cast<int>(elapsedTime));
    m_addRunStats->bind(5, static_cast<int>(triggers));
    m_addRunStats->bind(6, static_cast<int>(events));
    m_addRunStats->bind(7, static_cast<int>(bytes));
    
    ++(*m_addRunStats);
    
    // Get the insert id, prepare the query for re-use and return the id:
    
    int result = m_addRunStats->lastInsertId();
    m_addRunStats->reset();
    
    return result;
}
/**
 * marshallWords
 *    Takes a C word list and turns it into a space separated string.  A word
 *    list is a pointer to a set of strings.  Each string is null terminated and
 *    an additional null terminates the list.
 *
 *  @param words - the word list.
 *  @return std::string - the result string.
 */
std::string
CStatusDb::marshallWords(const char* words)
{
    std::string result;
    bool initial(true);
    while (*words) {
        // All but the first word is led in by a space:
        
        if (!initial) {
            result += " ";
            
        } else {
            initial = false;
        }
        result += words;
        words += std::strlen(words) + 1;              // Next string or null if done.
    }
    return result;
}