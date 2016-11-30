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
# @file   CStatusDefinitions.cpp
# @brief  Executable members of the outer CStatusDefinitions class
# @author <fox@nscl.msu.edu>
*/

#include "CStatusMessage.h"
#include <string>
#include <map>
#include <os.h>

static std::map<std::string, std::uint32_t> messageTypeLookup = {
        {"RING_STATISTICS", CStatusDefinitions::MessageTypes::RING_STATISTICS},
        {"EVENT_BUILDER_STATISTICS",
                CStatusDefinitions::MessageTypes::EVENT_BUILDER_STATISTICS},
        {"READOUT_STATISTICS", CStatusDefinitions::MessageTypes::READOUT_STATISTICS},
        {"LOG_MESSAGE", CStatusDefinitions::MessageTypes::LOG_MESSAGE},
        {"STATE_CHANGE", CStatusDefinitions::MessageTypes::STATE_CHANGE}
    };
static std::map<uint32_t, std::string> messageTypeStringLookup = {
        {CStatusDefinitions::MessageTypes::RING_STATISTICS, "RING_STATISTICS"},
        {CStatusDefinitions::MessageTypes::EVENT_BUILDER_STATISTICS,
            "EVENT_BUILDER_STATISTICS",
        },
        {CStatusDefinitions::MessageTypes::READOUT_STATISTICS, "READOUT_STATISTICS"},
        {CStatusDefinitions::MessageTypes::LOG_MESSAGE, "LOG_MESSAGE"},
        {CStatusDefinitions::MessageTypes::STATE_CHANGE, "STATE_CHANGE"}
    
};


static std::map<std::string, uint32_t> messageSeverityLookup = {
        {"DEBUG", CStatusDefinitions::SeverityLevels::DEBUG},
        {"INFO", CStatusDefinitions::SeverityLevels::INFO},
        {"WARNING", CStatusDefinitions::SeverityLevels::WARNING},
        {"SEVERE", CStatusDefinitions::SeverityLevels::SEVERE},
        {"DEFECT", CStatusDefinitions::SeverityLevels::DEFECT}
};

static std::map<uint32_t, std::string> messageSeverityStringLookup = {
        {CStatusDefinitions::SeverityLevels::DEBUG, "DEBUG"},
        {CStatusDefinitions::SeverityLevels::INFO, "INFO"},
        {CStatusDefinitions::SeverityLevels::WARNING, "WARNING"},
        {CStatusDefinitions::SeverityLevels::SEVERE, "SEVERE"},
        {CStatusDefinitions::SeverityLevels::DEFECT, "DEFECT"}
};
/**
 * messageTypeToString
 *    Convert a message type value to a string.
 *
 *  @param type         - the message type value.
 *  @return std::string - the stringified version of it.
 *  @throw std::invalid_argument if the type value is invalid.
 */
std::string
CStatusDefinitions::messageTypeToString(uint32_t type)
{
    auto p = messageTypeStringLookup.find(type);
    
    // If not found throw:
    
    if (p == messageTypeStringLookup.end()) {
        throw std::invalid_argument("Invalid message type value");    
    }
    return p->second;
}
/**
 *  stringToMessageType
 *     Converts a string value into a message type id.
 *
 *  @param typeString  - Stringified message type to convert.
 *  @return uint32_t   - corresponding message type.
 *  @throw std::invalid_argument - if the string is not a type string.
 */
uint32_t
CStatusDefinitions::stringToMessageType(const char* typeString)
{
    auto p = messageTypeLookup.find(std::string(typeString));
    
    // throw if lookup failed:
    
    if (p == messageTypeLookup.end()) {
        throw std::invalid_argument("Invalid message type string");
    }
    return p->second;
}
/**
 *  severityToString
 *     Convert a message severity value to a string.
 *
 *  @param severity - the severity value.
 *  @return std::string - The corresponding stringified value.
 *  @throw std::invalid_argument if severity is not a valid  severity value.
 */
std::string
CStatusDefinitions::severityToString(uint32_t severity)
{
    auto p = messageSeverityStringLookup.find(severity);
    
    // Throw if lookup failed:
    
    if (p == messageSeverityStringLookup.end()) {
        throw std::invalid_argument("Invalid message severity value");
    }
    
    return p->second;
}

/**
 * stringToSeverity
 *    Convert a stringified severity into is uint32_t value.
 *
 *  @parameter severityString - stringified severit.
 *  @return uint32_t          - Severity value.
 *  @throw std::invalid_argument - severityString has no corresponding severity
 *                                 value.
 */
uint32_t
CStatusDefinitions::stringToSeverity(const char* severityString)
{
    auto p = messageSeverityLookup.find(std::string(severityString));
    
    // Throw if lookup failed:
    
    if ( p == messageSeverityLookup.end()) {
        throw std::invalid_argument("Invalid severity string");
    }
    
    return p->second;
}
/**
 * formatHeader
 *    Formats a message header.  The message header is the first message
 *    segment in a status message.  Therefore this method is used by all of the
 *    nested classes.
 *
 *   @param type     -- Type of message being created.
 *   @param severity -- Severity of the message.
 *   @param appName  -- Name of the application that's formatting this message.
 *   @return CStatusDefinitions::Header - the formatted header object.
 */
void
CStatusDefinitions::formatHeader(Header& hdr, uint32_t type, uint32_t severity, const char* appName)
{
    /* Build/send the header. */
    
    hdr.s_type = type;
    hdr.s_severity = severity;

      // Fill in the application name.
      
    std::strncpy(
        hdr.s_application, appName,
        sizeof(hdr.s_application) - 1
    );
    hdr.s_application[sizeof(hdr.s_application) -1 ] = '\0';
    
        // Fill in the source with the fqdn of this host:
    
    std::string host = Os::hostname();
    std::strncpy(hdr.s_source, host.c_str(), sizeof(hdr.s_source) -1);
    hdr.s_source[sizeof(hdr.s_source) -1]  = 0;
     
}