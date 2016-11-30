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
  */
class CStatusDb {
private:
    CSqlite&        m_handle;             // Database handle.

public:
    CStatusDb(const char* dbSpec, int flags);
    virtual ~CStatusDb();

    // Insertion operations:
    
public:    
    void insert(const std::vector<zmq::message_t*>& message);
    void addRingStatistics(
        uint32_t severity, const char* app, const char* src,
        const CStatusDefinitions::RingStatIdentification& id,
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
};  

#endif