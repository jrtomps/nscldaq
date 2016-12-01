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
# @file   CStatusDb.h
# @brief  Class to encapsulate status database.
# @author <fox@nscl.msu.edu>
*/

#ifndef CSTATUSDB_H
#define CSTATUSDB_H
#include <zmq.hpp>
#include <vector>
#include "CStatusMessage.h"


class CSqlite;
class CSqliteStatement;

/**
  * @class CSqlite
  *   Provides a high level interface to a status database for use by
  *   inserters and readers.   The database can be opened in readrwite
  *   or readonly mode.  If opened in readwrite mode the database
  *   schema is created if it does not exist yet.  This also allows the
  *   status database to live on top of an existing database.
  *
  *   The reasons we have a status database at all are:
  *   -   To maintain a persisten record of all status items that have been
  *       aggregated.
  *   -   To simplify filtering for status display applications.
  *   -   To provide the ability to generate reports/queries that span
  *       status message types (e.g. what log messages were emitted during
  *       run 23).
  *
  *   table cases simpler.
  */
class CStatusDb {
    // Caching structs:
private:
    
private:
    CSqlite&        m_handle;             // Database handle.
    
    // Stored creation queries.
    
    CSqliteStatement* m_pLogInsert;      // Insert log message.

    CSqliteStatement* m_addRingBuffer;   // Insert into ring_buffer.
    CSqliteStatement* m_addRingClient;   // Insert into ring client.
    CSqliteStatement* m_addRingStats;    // Add statistics for a ring/client.
        
    CSqliteStatement* m_getRingId;    // Check existence of a ring buffer.
    CSqliteStatement* m_getClientId;  // Check for sqlite client.

        
    
public:
    CStatusDb(const char* dbSpec, int flags);
    virtual ~CStatusDb();

    // Insertion operations:
    
public:    
    void insert(const std::vector<zmq::message_t*>& message);
    void addRingStatistics(
        uint32_t severity, const char* app, const char* src,
        const CStatusDefinitions::RingStatIdentification& ringId,
        const std::vector<CStatusDefinitions::RingStatClient*>& clients
    );
    void addReadoutStatistics(
        uint32_t severity, const char* app, const char* src,
        int64_t startTime, uint32_t runNumber, const char* title,
        const CStatusDefinitions::ReadoutStatCounters* pCounters = NULL
    );
    void addLogMessage(
        uint32_t severity, const char* app, const char* src,
        int64_t  time, const char* message
    );
    void addStateChange(
        uint32_t severity, const char* app, const char* src,
        int64_t  tod, const char* from, const char* to
    );

private:
    void createSchema();
    
    int getRingId(const char* name, const char* host);
    int addRingBuffer(const char* name, const char* host);
    int getRingClientId(
        int ringId, const CStatusDefinitions::RingStatClient& client
    );
    int addRingClient(
        int ringId, const CStatusDefinitions::RingStatClient& client
    );
    int addRingClientStatistics(
        int ringId, int clientId, uint64_t timestamp,
        const CStatusDefinitions::RingStatClient& client
    );
    
    std::string marshallWords(const char* words);
};  

#endif