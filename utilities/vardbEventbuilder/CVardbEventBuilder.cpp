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
        
        // boolean is a very common data type so we define it in a try block
        // in case it's already defined.
        
        CVarMgrApi::EnumValues boolTexts;
        boolTexts.push_back("true");
        boolTexts.push_back("false");
        
        try {
            m_pApi->defineEnum("bool", boolTexts);
        }
        catch(...) {}
        
    }
}
/**
 * cresteEventBuilder
 *
 *    Create a new event builder.
 *    -   Create the diretory (throws if it already exists).
 *    -   add variables that describe the event builder based on the
 *       parameters to the call.
 *
 *  @param name -  Event builder name
 *  @param host -  Host in which the event builder will live.
 *  @param coincidenceInterval - Event glom time interval in timestamp ticks.
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
    unsigned outputSourceId, const char* servicePrefix, bool build,
    TimestampPolicy tsPolicy, const char* serviceSuffix
)
{
    // Errors will get thrown from deeper layers of the code so:
    
    try {
        // Make the event builder directory and cd to it:
        
        std::string evbDirname = "/";
        evbDirname += dirname;
        evbDirname += "/";
        evbDirname += name;
        m_pApi->mkdir (evbDirname.c_str());
        m_pApi->cd(evbDirname.c_str());
        
        // Create the variables with the right stuff.
        
        m_pApi->declare("host", "string", host);
        m_pApi->declare("servicePrefix", "string", servicePrefix);
        m_pApi->declare("serviceSuffix", "string", serviceSuffix);
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
        
    }
    catch (...) {
        // reset the wd to "/" and rethrow
        
        m_pApi->cd("/");
        throw;
    }
    m_pApi->cd("/");
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