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
# @file   CVardbEventBuilder.cpp
# @brief  Implements the API to the event builder description in variable database.
# @author <fox@nscl.msu.edu>
*/
#include "CVardbEventBuilder.h"
#include <CVarMgrApiFactory.h>
#include <CVarMgrApi.h>
#include <vector>
#include <string>
#include <algorithm>
#include <sstream>
#include <map>
#include <stdlib.h>
#include <stdio.h>

static const char* dirname = "EventBuilder";
static std::map<CVardbEventBuilder::TimestampPolicy, std::string> policyToText;

/**
 * constructor
 *    Construct a new instance of this class.  Specifically, we need
 *    to use the variable manager api factory to create an instance
 *    of our api object.
 *
 *    @param uri - Uri that identifies the connection method to the database.
 */
CVardbEventBuilder::CVardbEventBuilder(const char* uri) :
    m_pApi(0)
{
    if (policyToText.empty()) definePolicies();
    m_pApi = CVarMgrApiFactory::create(uri);   // Let exceptions propagate up.
}
/**
 * destructor
 *    Kill off the api object.
 */
CVardbEventBuilder::~CVardbEventBuilder()
{
    delete m_pApi;
}

/**
 * schemaExists
 *
 *  @return bool - true if the variable database directory (/EventBuilder) exists.
 *                 false if not.
 */
bool
CVardbEventBuilder::schemaExists()
{
    std::vector<std::string> dirs = m_pApi->ls("/");
    std::vector<std::string>::iterator pWhich =
        std::find(dirs.begin(), dirs.end(), std::string(dirname));
        
    return pWhich != dirs.end();
}
/**
 * createSchemaq
 *    Creates the schema.  This is a no-op if the schema already exists.
 */
void
CVardbEventBuilder::createSchema()
{
    if (policyToText.empty()) definePolicies();
    if (!schemaExists()) {
        std::string evbDir("/");
        evbDir += dirname;
        m_pApi->mkdir(evbDir.c_str());
        
        // Define the timestamp policy enum:
        
        CVarMgrApi::EnumValues policies;
        std::map<CVardbEventBuilder::TimestampPolicy, std::string>::iterator p =
            policyToText.begin();
        while (p != policyToText.end()) {
            policies.push_back(p->second);
            p++;
        }
        m_pApi->defineEnum("TimestampPolicy", policies);
        
        
    }
}
/**
 * createEventBuilder
 *
 *    Create a new event builder.
 *    -   Create the diretory (throws if it already exists).
 *    -   add variables that describe the event builder based on the
 *       parameters to the call.
 *
 *  @param name -  Event builder name
 *  @param host -  Host in which the event builder will live.
 *  @param coincidenceInterval - Event glom time interval in timestamp ticks.
 *  @param ring - name of ring buffer (not URI) the event builder will output data
 *                 to.
 *  @param outputSourceId - the source id assigned to output events.  Defaults to 0.
 *  @param servicePrefix  - the event builder service prefix advertised.  This defaults
 *                          to ORDERER.
 *  @param build  -  true if the glomming stage should build events, false to
 *                   just pass totally ordered event fragments out of the pipeline.
 *  @param tsPolicy - describes how output event timestamps are derived from input
 *                  event timestamps.  Defaults to CVardbEventBuilder::earliest
 *  @param serviceSuffix - A suffix text appended to the constructed event buidler
 *                  service.
 *                  
 */
void
CVardbEventBuilder::createEventBuilder(
    const char* name, const char* host, unsigned coincidenceInterval,
    const char* ring,
    unsigned outputSourceId, const char* servicePrefix, bool build,
    TimestampPolicy tsPolicy, const char* serviceSuffix
)
{
    // Errors will get thrown from deeper layers of the code so:
    
    try {
        // Make the event builder directory and cd to it:
        
        std::string dir = evbDirname(name);
        m_pApi->mkdir(dir.c_str());
        m_pApi->cd(dir.c_str());
        
        // Create the variables with the right stuff.
        
        m_pApi->declare("host", "string", host);
        m_pApi->declare("servicePrefix", "string", servicePrefix);
        m_pApi->declare("serviceSuffix", "string", serviceSuffix);
        m_pApi->declare("ring", "string", ring);
        m_pApi->declare(
            "coincidenceInterval", "integer",
            uIntToString(coincidenceInterval).c_str()
        );
        m_pApi->declare("build", "bool", boolToString(build).c_str());
        m_pApi->declare(
            "timestampPolicy", "TimestampPolicy",
            tsPolicyToString(tsPolicy).c_str()
        );
        m_pApi->declare(
            "sourceId", "integer", uIntToString(outputSourceId).c_str()
        );
        
        m_pApi->declare("editorx", "integer", "0");
        m_pApi->declare("editory", "integer", "0");
        
    }
    catch (...) {
        // reset the wd to "/" and rethrow
        
        m_pApi->cd("/");
        throw;
    }
    m_pApi->cd("/");
}

/**
 * evbSetHost
 *    Change the allocation of the host in which an event builder is run.
 *
 * @param name - name of event builder.
 * @param host - new hostname.
 */
void
CVardbEventBuilder::evbSetHost(const char* name, const char* newHost)
{
    std::string dir = evbDirname(name);
    m_pApi->cd(dir.c_str());
    m_pApi->set("host", newHost);
    m_pApi->cd("/");
}
/**
 * evbSetCoincidenceInterval
 *
 * @param name - name of the event builder to modify.
 * @param newIntervanl - new coincidecne interval.
 */
void
CVardbEventBuilder::evbSetCoincidenceInterval(
    const char* name, unsigned newInterval
)
{
    std::string dir = evbDirname(name);
    m_pApi->cd(dir.c_str());
    m_pApi->set("coincidenceInterval", uIntToString(newInterval).c_str());
    m_pApi->cd("/");
}
/**
 * evbSetRing
 *   Set a new output ring for the ringbuffer.
 *
 *   @param name - event builder name.
 *   @param ring - name (not URI) of new output ring.
 */
void
CVardbEventBuilder::evbSetRing(const char* name, const char* ring)
{
    std::string dir = evbDirname(name);
    m_pApi->cd(dir.c_str());
    m_pApi->set("ring", ring);
    m_pApi->cd("/");
}
/**
 * evbSetSourceId
 *
 *  @param name - name of the event builder being edited.
 *  @param newid - New sourceid.
 */
void
CVardbEventBuilder::evbSetSourceId(const char* name, unsigned newid)
{
    std::string dir = evbDirname(name);
    m_pApi->cd(dir.c_str());
    m_pApi->set("sourceId", uIntToString(newid).c_str());
    m_pApi->cd("/");
}
/**
 * evbSetServicePrefix
 *
 *    @param name - Name of the event builder.
 *    @param newPrefix - new service name prefix.
 */
void
CVardbEventBuilder::evbSetServicePrefix(const char* name, const char* newPrefix)
{
    std::string dir = evbDirname(name);
    m_pApi->cd(dir.c_str());
    m_pApi->set("servicePrefix", newPrefix);
    m_pApi->cd("/");
}
/**
 * evbSetServiceSuffix
 *   @param name - name of event builder to modify.
 *   @param newSuffix - new service name suffix.
 */
void
CVardbEventBuilder::evbSetServiceSuffix(const char* name, const char* newSuffix)
{
    std::string dir = evbDirname(name);
    m_pApi->cd(dir.c_str());
    m_pApi->set("serviceSuffix", newSuffix);
    m_pApi->cd("/");
}

/**
 * disableBuild
 *   Turn off event building in glom.
 *
 *  @param name  of event builder affected
 */
void
CVardbEventBuilder::evbDisableBuild(const char* name)
{
    std::string dir = evbDirname(name);
    m_pApi->cd(dir.c_str());
    m_pApi->set("build", "false");
    m_pApi->cd("/");
}
/**
 * enableBuid
 *
 *  @param name - name of event builder affected.
 */
void
CVardbEventBuilder::evbEnableBuild(const char* name)
{
    std::string dir = evbDirname(name);
    m_pApi->cd(dir.c_str());
    m_pApi->set("build", "true");
    m_pApi->cd("/");
}

/**
 * evbSetTimestampPolicy
 *
 *  @param name -name of event builder.
 *  @param newPolic - new timestamp policy.
 */
void
CVardbEventBuilder::evbSetTimestampPolicy(
    const char* name, TimestampPolicy newPolicy
)
{
    std::string dir = evbDirname(name);
    m_pApi->cd(dir.c_str());
    m_pApi->set("timestampPolicy", policyToText[newPolicy].c_str());
    m_pApi->cd("/");
}

/**
 * rmEventBuilder
 *     Remove an existing event builder.
 *
 * @param name - name of event builder to get rid of.
 *
 */
void
CVardbEventBuilder::rmEventBuilder(const char* name)
{
    std::string dir = evbDirname(name);
    rmTree(dir.c_str());

}

/**
 * evbInfo
 *    Return information about a specific event builder.
 *
 *  @param name - name of the event builder.
 *  @return CVardbEventBuilder::EvbDescription
 */
CVardbEventBuilder::EvbDescription
CVardbEventBuilder::evbInfo(const char* name)
{
    EvbDescription result;
    std::string dir = evbDirname(name);
    m_pApi->cd(dir.c_str());
    
    // Fill in the result from the variable values:
    
    result.s_name = name;
    result.s_host          = m_pApi->get("host");
    result.s_servicePrefix = m_pApi->get("servicePrefix");
    result.s_serviceSuffix = m_pApi->get("serviceSuffix");
    result.s_coincidenceInterval = atoi(m_pApi->get("coincidenceInterval").c_str());
    result.s_ring          = m_pApi->get("ring");
    result.s_build         = m_pApi->get("build") == std::string("true") ? true : false;
    result.s_timestampPolicy = textToPolicy(m_pApi->get("timestampPolicy"));
    result.s_sourceId     = atoi(m_pApi->get("sourceId").c_str());
    
    //
    
    m_pApi->cd("/");
    return result;
}

/**
 * listEventBuilders
 *
 *    Lists full information about all the event builders that have been defined.
 *
 *  @return std::vector<EvbDescription>
 */
std::vector<CVardbEventBuilder::EvbDescription>
CVardbEventBuilder::listEventBuilders()
{
    m_pApi->cd(dirname);
    std::vector<std::string> evbs = m_pApi->ls();
    m_pApi->cd("/");
    
    
    std::vector<CVardbEventBuilder::EvbDescription> result;
    for (int i =0; i < evbs.size(); i++) {
        result.push_back(evbInfo(evbs[i].c_str()));    
    }
    
    
    return result;
}

/**
 * addDataSource
 *   Add a new data source to an event builder.
 * See https://swdev-redmine.nscl.msu.edu/projects/sfnscldaq/wiki/Event_builder_schema#Event-builder-data-source-variables
 * (NSCL internal site) for information about how event builder data sources
 * are represented.  Summarizing each event builder has a set of subdirectories,
 * one per data source. The directory name is the event source name and the
 * variables within that directory describe the event source.
 * This API only handles NSCL ring buffer data sources.
 *
 * @param evbName  - Name of the event builder for which this is a data source.
 * @param srcName  - Name of the data source (unique within the event builder)
 *                   being created.
 * @param host     - Host in which the data source program runs.
 * @param path     - Command line path to the data source program.
 * @param ringUri  - URI of the ring buffer the data source takes fragments from.
 * @param ids      - The vector of ids that are the data source ids the source
 *                   expects to get from the ring buffer (body header).
 * @param info     - Information script passed to the event builder from the source
 *                   that describes the source (comment if you will).
 * @param expectBodyHeaders - if true, it is an error for the items not to have
 *                   body headers from which the source id and timestamp can be
 *                   directly obtained.
 * @param defaultId- If a ring item does not have a  body header it is assigned
 *                   this data source id.
 * @param timestampExtractor - If a ring item does not have a body header this
 *                   is a path to a shared library that is supposed to have code
 *                   that can extract the timestamp from the item.
 */
 void
 CVardbEventBuilder::addDataSource(
    const char* evbName, const char* srcName, const char* host, const char* path,
    const char* ringUri, std::vector<unsigned> ids, const char* info,
    bool expectBodyHeaders, unsigned defaultId, const char* timestampExtractor
 )
 {
    // What we do might throw so we're going to do everything in a try/catch
    // block to ensure we get the wd back to "/"
    
    m_pApi->cd(evbDirname(evbName).c_str());
    try {
        m_pApi->mkdir(srcName);
        m_pApi->cd(srcName);
        
        // Set the variables:
        
        m_pApi->declare("host", "string", host);
        m_pApi->declare("path", "string", path);
        m_pApi->declare("info", "string", info);
        m_pApi->declare("ring", "string", ringUri);
        m_pApi->declare("default-id", "integer", uIntToString(defaultId).c_str());
        m_pApi->declare("timestamp-extractor", "string", timestampExtractor);
        m_pApi->declare("expect-bodyheaders", "bool", boolToString(expectBodyHeaders).c_str());
        m_pApi->declare("editorx", "integer", "0");
        m_pApi->declare("editory", "integer", "0");
        
        // Marshall the ids array into idn variables.
        
        for (int i = 0; i < ids.size(); i++) {
            char name[100];
            sprintf(name, "id%d", i);
            m_pApi->declare(name, "integer", uIntToString(ids[i]).c_str());
        }
        
    }
    catch (...) {
        m_pApi->cd ("/");
        throw;
    }
    m_pApi->cd("/");
 }
 /**
  * dsSetHost
  *    Modify the host name of an existing event builder data source.
  *
  *   @param evb -name of event builder.
  *   @param ds  -name of datasource.
  *   @param newHost - new host name.
  */
 void
 CVardbEventBuilder::dsSetHost(const char* evb, const char* ds, const char* newHost)
 {
    std::string dsDir = dsDirName(evb, ds);
    m_pApi->cd(dsDir.c_str());
    m_pApi->set("host", newHost);
    
    m_pApi->cd("/");
 }
 /**
  * dsSetPath
  *    Set the path to the data source program.
  *
  *  @param evbName - name of event builder.
  *  @param dsName  - name of its data source.
  *  @param newPath - New path to the data source's program.
  */
 void
 CVardbEventBuilder::dsSetPath(
    const char* evbName, const char* dsName, const char* newPath
)
 {
    std::string dsDir = dsDirName(evbName, dsName);
    m_pApi->cd(dsDir.c_str());
    m_pApi->set("path", newPath);
    
    m_pApi->cd("/");    
 }
 /**
  * dsSetRingUri
  *    Modifies the URI of the ring from which data will be taken and squirted
  *    to the event builder.
  *
  *  @param evb    - Event builder name.
  *  @param ds     - Name of data source for this event builder.
  *  @param newRing - New URI of the ring.
  */
 void
 CVardbEventBuilder::dsSetRingUri(const char* evb, const char* ds, const char* newRing)
 {
    std::string dsDir = dsDirName(evb, ds);
    m_pApi->cd(dsDir.c_str());
    m_pApi->set("ring", newRing);
    
    m_pApi->cd("/");
 }
 /**
  * dsSetIds
  *    Set the ids produced by the data source to a new set of values.
  *
  *  @param evb    - name of the event builder.
  *  @param ds     - name of that event builder's data source.
  *  @param ids    - new vector of ids.
  */
 void
 CVardbEventBuilder::dsSetIds(const char* evb, const char* ds, std::vector<unsigned> ids)
 {
    m_pApi->cd(dsDirName(evb, ds).c_str());
    
    // First we need to destroy the existing variables whose names start with
    // id.  While we create variables like id1, id2... the specification
    // says that any variable whose name is id* is a source id.  This code
    // ensures that any manually created source ids are also destroed (e.g
    // id-special).
    
    std::vector<CVarMgrApi::VarInfo> vars = m_pApi->lsvar();
    for (int i = 0; i < vars.size(); i++) {
        if (vars[i].s_name.substr(0, 2) == std::string("id")) {
            m_pApi->rmvar(vars[i].s_name.c_str());
        }
    }
    // Now we can create new variables.
    
    for (int i = 0; i < ids.size(); i++) {
        char varname[100];
        sprintf(varname, "id%d", i);
        m_pApi->declare(varname, "integer", uIntToString(ids[i]).c_str());
    }
    
    m_pApi->cd("/");
 }
 /**
  * dsSetInfo
  *    Sets the information string for a data source.
  *   @param evb - event builder name.
  *   @param ds  - Data source name.
  *   @param newInfo - new info string.
  */
 void
 CVardbEventBuilder::dsSetInfo(const char* evb, const char* ds, const char* newInfo)
 {
    m_pApi->cd(dsDirName(evb, ds).c_str());
    m_pApi->set("info", newInfo);
    m_pApi->cd("/");
 }
 
 /**
  * dsSetDefaultId
  *
  *    Set a new value for the default id.  The default id is used to supply
  *    a data source id in the event a ring item does not have a body header.
  *
  *  @param evb    - Event builder being modified.
  *  @param ds     - Data source for that event builder that is modified.
  *  @param newId  - New default id.
  */
 void
 CVardbEventBuilder::dsSetDefaultId(const char* evb, const char* ds, unsigned newId)
 {
    m_pApi->cd(dsDirName(evb, ds).c_str());
    m_pApi->set("default-id", uIntToString(newId).c_str());
    m_pApi->cd("/");
 }
 
 /**
  * dsExpectBodyHeaders
  *
  *     Tells a data source that it should expect body headers in all fragments
  *     when run in this manner,
  *     a ring item without a body header is considered an error.
  *  @param evb  - Event builder name.
  *  @param ds   - Data source name within the event builder
  */
 void
 CVardbEventBuilder::dsExpectBodyHeaders(const char* evb, const char* ds)
 {
    m_pApi->cd(dsDirName(evb, ds).c_str());
    m_pApi->set("expect-bodyheaders", "true");
    
    m_pApi->cd("/");
 }
 /**
  *  dsDontExpectBodyHeaders
  *     Tells a data source there can be ring items that don't have body headers.
  *
  *  @param evb - event builder name.
  *  @param ds - Data source name.
  */
 void
 CVardbEventBuilder::dsDontExpectBodyHeaders(const char* evb, const char* ds)
  {
    m_pApi->cd(dsDirName(evb, ds).c_str());
    m_pApi->set("expect-bodyheaders", "false");
    
    m_pApi->cd("/");
 }
 /**
  * dsSetTimestampExtractor
  *    Change the timestamp extractor shared library used by the source when
  *    an item has no body header from which to extract  its timestamp.
  *
  *  @param evb  - Event builder.
  *  @param ds   - Data Source
  *  @param newLib - New path to timestamp extraction library.
  */
 void
 CVardbEventBuilder::dsSetTimestampExtractor(
    const char* evb, const char* ds, const char* newLib
)
 {
    m_pApi->cd(dsDirName(evb, ds).c_str());
    m_pApi->set("timestamp-extractor", newLib);
    m_pApi->cd("/");
 }
 /**
  * dsInfo
  *    Provide information about a data source.
  *
  * @param evb - the event builder.
  * @param ds  - The data source.
  */
 CVardbEventBuilder::DsDescription
 CVardbEventBuilder::dsInfo(const char* evb, const char* ds)
 {
    m_pApi->cd(dsDirName(evb, ds).c_str());
    DsDescription result;
    try {
        result.s_name = ds;
        result.s_host = m_pApi->get("host");
        result.s_path = m_pApi->get("path");
        result.s_info = m_pApi->get("info");
        result.s_ringUri = m_pApi->get("ring");
        result.s_expectBodyheaders =
            m_pApi->get("expect-bodyheaders") == "true" ? true : false;
        result.s_defaultId = (unsigned)atoi(m_pApi->get("default-id").c_str());
        result.s_timestampExtractor = m_pApi->get("timestamp-extractor");
        
        // Now we need to go over all the variables turning the id* ones int
        // elements of the s_ids vector.
        
        std::vector<CVarMgrApi::VarInfo> vars = m_pApi->lsvar();
        for (int i = 0; i < vars.size(); i++) {
            if (vars[i].s_name.substr(0,2) == std::string("id")) {
                result.s_ids.push_back(unsigned(atoi(vars[i].s_value.c_str())));
            }
        }
    }
    catch (...) {
        m_pApi->cd("/");
        throw;
    }
    
    m_pApi->cd("/");
    return result;
 }
 /**
  * listDataSources
  *    Return a vector of the data source descriptions for the data
  *    sources that are defined for an event builder.
  *
  *  @param evb - event builder name.
  *  @return std::vector<DsDescription> - vector of event source descriptors.
  */
 std::vector<CVardbEventBuilder::DsDescription>
 CVardbEventBuilder::listDataSources(const char* evb)
 {
    // list the event source directories first:
    
    std::vector<std::string> names = m_pApi->ls(evbDirname(evb).c_str());
    
    // Now for each name add an info element to the result vector:
    
    std::vector<DsDescription> result;
    for (int i = 0; i < names.size(); i++) {
        result.push_back(dsInfo(evb, names[i].c_str()));
    }
    return result;
 }
 /**
  * rmDataSource
  *    Remove a data source from an event builder.
  *
  *  @param evb - name of the event builder.
  *  @param ds  - data source name (within the event builder).
  */
 void
 CVardbEventBuilder::rmDataSource(const char* evb, const char* ds)
 {
    std::string dir = dsDirName(evb, ds);
    rmTree(dir.c_str());
 }
/*-----------------------------------------------------------------------------
 *  Utility functions
 */

/**
 * uIntToString
 *    Convert an unsigned integer to an std::string. 
 * @param val  - Value to convert.
 * @return std::string - string representation of the value.
 */
std::string
CVardbEventBuilder::uIntToString(unsigned val)
{
    std::stringstream s;
    s << val;
    return s.str();
}
/**
 * boolToString
 *
 *    @param value -  value to convert.
 *    @return std::string - string rep of the value.
 *    @note we can use std::stringstream and std::boolalpha to do this but I think
 *          the code below is simpler to get and probably faster (though that's not
 *          a consideration).
 */
std::string
CVardbEventBuilder::boolToString(bool value)
{
    return std::string(value ? "true" : "false");
}
/**
 * tsPolicyToString
 *    Convert a timestamp policy to a string.
 *
 *    @param value - value to convert.
 *    @return std::string - string representation.
 */
std::string
CVardbEventBuilder::tsPolicyToString(TimestampPolicy p) {
    return policyToText[p];
}
/**
 * definePolicies
 *   Define the timestamp polices in policyToText:
 *
 */
void
CVardbEventBuilder::definePolicies()
{
    policyToText[earliest] = "earliest";
    policyToText[latest]   = "latest";
    policyToText[average]  = "average";
}
/**
 * evbDirname
 *    Construct/return the absolute path to an event builder directory.
 *
 *   @param name - event builder name.
 *   @return std::string  - path to the directory that's supposed to hold it's info.
 */
std::string
CVardbEventBuilder::evbDirname(const char* name)
{
    std::string result("/");
    result += dirname;
    result += "/";
    result += name;
    
    return result;
}
/**
 * rmTree
 *   Remove a directory tree.
 *
 *   @param name - path to the top of the tree.
 */
void
CVardbEventBuilder::rmTree(const char* name)
{
    std::string wd = m_pApi->getwd();
    m_pApi->cd(name);
    // First delete all the variables in this directory.
    
    std::vector<CVarMgrApi::VarInfo> vars = m_pApi->lsvar();
    for (int i =0; i < vars.size(); i++) {
        m_pApi->rmvar(vars[i].s_name.c_str());
    }
    
    // Then recurse for all subdirectories.
    
    std::vector<std::string> subdirs = m_pApi->ls();
    for (int i = 0; i < subdirs.size(); i++) {
        rmTree(subdirs[i].c_str());
    }
    
    //  restore the wd and kill off the top level dir.
    
    m_pApi->cd(wd.c_str());
    m_pApi->rmdir(name);
}
/**
 * textToPolicy
 *    Convert a textual timestamp policy string to a policy value.
 *
 *  @param value - stringized policy
 *  @return TimestampPolicy - corresponding timestamp policy value.
 */
CVardbEventBuilder::TimestampPolicy
CVardbEventBuilder::textToPolicy(std::string value)
{
    std::map<TimestampPolicy, std::string>::iterator p = policyToText.begin();
    while (p != policyToText.end()) {
        if (p->second == value) return p->first;
        p++;
    }
}
/**
 *  dsDirName
 *     Return the directory in which information about a specific data source
 *     is stored.
 *  @param evbName - name of the event builder.
 *  @param dsName  - Name of the data source.
 *  @return std::string.
 */
std::string
CVardbEventBuilder::dsDirName(const char* evbName, const char* dsName)
{
    std::string result = evbDirname(evbName);
    result += "/";
    result += dsName;
    
    return result;
}