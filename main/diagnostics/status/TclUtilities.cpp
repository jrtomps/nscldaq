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
# @file   TclUtilties.cpp
# @brief  Implements common utitilties for the package.
# @author <fox@nscl.msu.edu>
*/
#include "TclUtilities.h"
#include "CStatusMessage.h"
#include <tcl.h>
#include <TCLInterpreter.h>
#include <TCLObject.h>
#include <TCLException.h>
#include <stdexcept>
#include <map>
#include <cstring>



zmq::context_t& TclMessageUtilities::m_zmqContext(*(new zmq::context_t(1)));

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
 * stringVectorFromList
 *    Turn a CTCLObject that contains a list to an std::vector<std::string.
 *    The object is assumed bound to an interpreter.
 *  @param obj - The object being analyzed.
 *  @return std::vector<std::string>
 */
std::vector<std::string>
TclMessageUtilities::stringVectorFromList(CTCLObject& obj)
{
    std::vector<std::string> result;
        for (int i = 0; i < obj.llength(); i++) {
        result.push_back(std::string(obj.lindex(i)));
    }
    return result;
}
/*
 * uint64FromObject
 *    Fetches a uint64_t from a CTCLObject.
 *
 *  @param interp - interpreter to use to parse the object.
 *  @param obj    - Object we're getting the value from.
 *  @param pDoing - String documenting what's being done.  This is part of the
 *                  error exception if the parse fails.
 *  @return uint64_t
 *  @throw  CTCLException if the parse fails.
 */
uint64_t
TclMessageUtilities::uint64FromObject(
    CTCLInterpreter& interp, CTCLObject& obj, const char* pDoing
)
{
    static_assert(
        sizeof(long) >= sizeof(uint64_t),
        "Long is not wide enough for a uint64_t"
    );   // Ensure we're not chopping.
    
    uint64_t result;
    Tcl_Obj* tclObj = obj.getObject();
    int status = Tcl_GetLongFromObj(interp.getInterpreter(), tclObj, reinterpret_cast<long*>(&result));
    if (status != TCL_OK) {
        throw CTCLException(
            interp, 0, "Failed to get number of operations from command line"
        );       
    }
    return result;
}

/**
 * messageTypeToString
 *    Convert a message type value to a string.
 *
 *  @param type         - the message type value.
 *  @return std::string - the stringified version of it.
 *  @throw std::invalid_argument if the type value is invalid.
 */
std::string
TclMessageUtilities::messageTypeToString(uint32_t type)
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
TclMessageUtilities::stringToMessageType(const char* typeString)
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
TclMessageUtilities::severityToString(uint32_t severity)
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
TclMessageUtilities::stringToSeverity(const char* severityString)
{
    auto p = messageSeverityLookup.find(std::string(severityString));
    
    // Throw if lookup failed:
    
    if ( p == messageSeverityLookup.end()) {
        throw std::invalid_argument("Invalid severity string");
    }
    
    return p->second;
}
/**
 * addToDictionary
 *    This overload adds a const char* value (string)  to a dictionary
 *    for the specified key.
 *
 *  @param interp - interpreter used for dict operations.
 *  @param dict   - reference to CTCLObject that is the dict we're building.
 *  @param key    - New dict keywoprd.
 *  @param value  - New dict value to associate with 'key'.
 *  
 */
void
TclMessageUtilities::addToDictionary(
    CTCLInterpreter& interp, CTCLObject& dict, const char* key, const char* value
)
{
    Tcl_Obj*   obj       = dict.getObject();
    Tcl_Interp*rawInterp = interp.getInterpreter();
    
    // Turn key/value into Tcl_Obj*'s as that's what the API needs:
    
    Tcl_Obj* keyObj   = Tcl_NewStringObj(key, -1);
    Tcl_Obj* valueObj = Tcl_NewStringObj(value, -1);
    
    // Now we can add to the dict:
    
    Tcl_DictObjPut(rawInterp, obj, keyObj, valueObj);
}

/**
 * addToDictionary
 *    This overload adds a uint64_t to a dictionary for a specific key.
 *
 *  @param  interp -  interpreter used for dictionary operations.
 *  @param  dict   -   dictionary being built.
 *  @param  key    -   Dictionary key to add.
 *  @param  value  -   Dictionary value to associate with the new key.
 */
void
TclMessageUtilities::addToDictionary(
    CTCLInterpreter& interp, CTCLObject& dict,
    const char* key, uint64_t value
)
{
    // Since we directly use the Tcl API:
    
    Tcl_Obj*  obj         = dict.getObject();
    Tcl_Interp* rawInterp = interp.getInterpreter();
    
    // We'll use a wide int as hopefully that should hold a 64 bit integer
    // just in case, however check it out:
    
    static_assert(
        sizeof(uint64_t) <= sizeof(Tcl_WideInt),
        "uint64_t is not compatible with a wide int."
    );
    
    Tcl_Obj* keyObj = Tcl_NewStringObj(key, -1);
    Tcl_Obj* valObj = Tcl_NewWideIntObj(value);
    
    // Add to the dict:
    
    Tcl_DictObjPut(rawInterp, obj, keyObj, valObj);
}
/**
 * addToDictionary
 *   Add a CTCLObject to a dictionary.   This is simpler because a
 *   CTCLObject is just a wrapping of Tcl_Obj*
 *
 * @param interp - interpreter used for dictionary operations.
 * @param dict   - Dictionary we're building up.
 * @param key    - Key to add to the dictionary.
 * @param value  - Value to associate with the key.
 */
void
TclMessageUtilities::addToDictionary(
    CTCLInterpreter& interp, CTCLObject& dict,
    const char* key, CTCLObject& value    
)
{
    Tcl_Obj*    obj       = dict.getObject();
    Tcl_Interp* rawInterp = interp.getInterpreter();
    
    Tcl_Obj* keyObj = Tcl_NewStringObj(key, -1);
    
    Tcl_DictObjPut(rawInterp, obj, keyObj, value.getObject());
}
/**
 *  listFromStringList
 *     Many of the messages store a list of strings as null terminated strings
 *     one after another terminated by an additional null.  This method
 *     turns such string lists into a Tcl List of strings.
 *
 * @param interp - interpreter used to manipulate the CTCLObjects involved.
 * @param const char* strings pointer to the list of strings to convert.
 * @return CTCLObject - The newly created object.
 */
CTCLObject
TclMessageUtilities::listFromStringList(CTCLInterpreter& interp, const char* strings)
{
    CTCLObject result;
    result.Bind(interp);
    
    while (*strings) {
        CTCLObject item;
        item.Bind(interp);
        item = std::string(strings);
        
        result += item;
        
        strings += std::strlen(strings) + 1;  // +1 to get past the null.
    }
    
    
    return result;
}
