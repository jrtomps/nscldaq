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
    m_pLogInsert(0)
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