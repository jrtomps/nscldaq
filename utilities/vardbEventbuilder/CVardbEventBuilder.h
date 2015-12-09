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
# @file   CVardbEventBuilder.h
# @brief  Header for C++ api to maniuplate event builder defs in variable database.
# @author <fox@nscl.msu.edu>
*/
#ifndef CVARDBEVENTBUILDER_H
#define CVARDBEVENTBUILDER_H

#include <string>
#include <vector>

class CVarMgrApi;

/**
 * @class CVardbEventBuilder
 *    Provides a C++ API to the event builder part of the variable database.
 *    Using this class you can:
 *    -   Check for/create the directory structure used to maintain event builder
 *        definitions.
 *    -   Create new event builder definitions.
 *    -   Create new data sources for existing event builder definitions.
 *    -   Obtain information about the existing event builder definitions.
 *    -   Obtain informationa bout the data sources that have been defined for
 *        existing event builders.
 *    -   Modify the definitions of event builders.
 *    -   Modify the definitions of event builder data sources.
 *    -   Delete event builder data sources
 *    -   Delete event builders (and by implication their data sources).
 */
class CVardbEventBuilder
{
    // Data types:
public:
    typedef enum _TimestampPolicy {
        earliest, latest, average
    } TimestampPolicy;
    
    typedef struct _EvbDescription {
        std::string     s_name;
        std::string     s_host;
        unsigned        s_coincidenceInterval;
        std::string     s_servicePrefix;
        std::string     s_serviceSuffix;
        bool            s_build;
        unsigned        s_sourceId;
        TimestampPolicy s_timestampPolicy;
        
    } EvbDescription, *pEvbDescription;
    
    // Internal attributes
private:
   CVarMgrApi*   m_pApi;                  // underlying database api.

   // canonicals:

public:
    CVardbEventBuilder(const char* uri);
    ~CVardbEventBuilder();
    
    // schema functions:
    
    bool schemaExists();
    void createSchema();
    
    // Creating event builders:
    
    void createEventBuilder(
        const char* name, const char* host, unsigned coincidenceInterval,
        unsigned outputSourceId = 0, const char* servicePrefix = "ORDERER",
        bool build = true, TimestampPolicy tsPolicy = earliest,
        const char* serviceSuffix = ""
    );
    
    // Modify existing event builders:
    
    void evbSetHost(const char* name, const char* newHost);
    void evbSetCoincidenceInterval(const char* name,unsigned newInterval);
    void evbSetSourceId(const char* name, unsigned newSourceId);
    void evbSetServicePrefix(const char* name, const char* newPrefix);
    void evbEnableBuild(const char* name);
    void evbDisableBuild(const char* name);
    void evbSetTimestampPolicy(const char* name,  TimestampPolicy newPolicy);
    void evbSetServiceSuffix(const char* name, const char* newSuffix);

    // Listing/deleting event builders:
    
    void rmEventBuilder(const char* name);
    EvbDescription evbInfo(const char* name);
    std::vector<EvbDescription> listEventBuilders();
    
    // data sources:
    
    void addDataSource(
        const char* evbName, const char* srcName, const char* host,
        const char* path, const char* ringUri,
        std::vector<unsigned> ids, const char* info="",
        bool expectBodyHeaders = true, unsigned defaultId = 0,
        const char* timestampExtractor = ""
    );
    void dsSetHost(const char* evbName, const char* srcName, const char* host);
    void dsSetPath(const char* evbName, const char* srcName, const char* path);
    void dsSetRingUri(
        const char* evbName, const char* srcName, const char* ringUri
    );
    void dsSetIds(
        const char* evbName, const char* srcName, std::vector<unsigned> ids
    );
    void dsSetInfo(const char* evbName, const char* srcName, const char* info);
    void dsSetDefaultId(const char* evbName, const char* srcName, unsigned id);
    void dsExpectBodyHeaders(const char* evbName, const char* srcName);
    void dsDontExpectBodyHeaders(const char* evbName, const char* srcName);
    void dsSetTimstampExtractor(
        const char* evbName, const char* srcName, const char* path
    );
    
    
    
    
    
    // Utility methods:
private:
    std::string uIntToString(unsigned value);
    std::string boolToString(bool value);
    std::string tsPolicyToString(TimestampPolicy value);
    void definePolicies();
    std::string evbDirname(const char* name);
    void rmTree(const char* name);
    TimestampPolicy textToPolicy(std::string value);
    
};

#endif 