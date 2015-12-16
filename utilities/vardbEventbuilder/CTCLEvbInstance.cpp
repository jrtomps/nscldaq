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
# @file   CTCLEvbInstance.cpp
# @brief  Implementation of evb connection instance command.
# @author <fox@nscl.msu.edu>
*/

#include "CTCLEvbInstance.h"
#include <TCLInterpreter.h>
#include <TCLObject.h>
#include "CVardbEventBuilder.h"
#include <stdexcept>
#include <stdlib.h>


/**
 * constructor
 *    Construct a new instance command.
 *
 *  @param interp - the interpreter the command is registered on.
 *  @param cmd    - Command name string.
 *  @param pApi   - API object used to perform the operations.
 *                  note that this object is assumed dynamically created
 *                  and we own it.
 */
CTCLEvbInstance::CTCLEvbInstance(
    CTCLInterpreter& interp, const char* cmd, CVardbEventBuilder* pApi
) : CTCLObjectProcessor(interp, cmd, true),
    m_pApi(pApi)
{
}

/**
 * destructor
 */
CTCLEvbInstance::~CTCLEvbInstance()
{
    delete m_pApi;
}
/**
 * operator()
 *  Subcommand dispatcher.
 *  We use exception handling to set the error/TCL_ERROR stuff.
 *
 *  @param interp - interpreter running the command.
 *  @param objv   - vector of command line words.
 *  @return int - TCL_OK success, TCL_ERROR failure.
 *  @note the interpreter result will vary depending on the failure
 *        (error message) or success (depends on subcommand).
 */
int
CTCLEvbInstance::operator()(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    try {
        bindAll(interp, objv);
        requireAtLeast(objv, 2, "Insufficient number of parameters");
        std::string subcommand = objv[1];
        
        if (subcommand == "haveSchema") {
            haveSchema(interp, objv);
        } else if (subcommand == "createSchema") {
            createSchema(interp, objv);
        } else if (subcommand == "createEventBuilder") {
            createEventBuilder(interp, objv);
        } else if (subcommand == "evbSetHost") {
            evbSetHost(interp, objv);
        } else if (subcommand == "evbSetCoincidenceInterval") {
            evbSetCoincidenceInterval(interp, objv);
        } else if (subcommand == "evbSetSourceId") {
            evbSetSourceId(interp, objv);
        } else if (subcommand == "evbSetServicePrefix") {
            evbSetServicePrefix(interp, objv);
        } else if (subcommand == "evbDisableBuild") {
            evbDisableBuild(interp, objv);
        } else if (subcommand == "evbEnableBuild") {
            evbEnableBuild(interp, objv);
        } else if (subcommand == "evbSetTimestampPolicy") {
            evbSetTimestampPolicy(interp, objv);
        } else if (subcommand == "evbSetServiceSuffix") {
            evbSetServiceSuffix(interp, objv);
        } else if (subcommand == "rmevb") {
            rmevb(interp, objv);
        } else if (subcommand == "evbInfo") {
            evbInfo(interp, objv);
        } else if (subcommand == "evbList") {
            evbList(interp, objv);
        } else {
            throw std::runtime_error("invalid subcommand");
        }
    }
    catch (std::exception& e) {
        interp.setResult(e.what());
        return TCL_ERROR;
    }
    catch (std::string msg) {
        interp.setResult(msg);
        return TCL_ERROR;
    }
    catch (const char* msg) {
        interp.setResult(msg);
        return TCL_ERROR;
    }
    catch (...) {
        interp.setResult("Exception caught of an unanticipated type");
        return TCL_ERROR;
    }
    return TCL_OK;
}

/*------------------------------------------------------------
 * Subcommand executors.
 */

/**
 * haveSchema
 *   Set the interpreter result to boolean true if there's
 *   already an event builder schema else fals.
 *
 *  @param interp - interpreter running the command.
 *  @param objv   - vector of command line words.
 */
void
CTCLEvbInstance::haveSchema(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    requireExactly(objv, 2);
    bool result = m_pApi->schemaExists();
    interp.setResult(result ? "1" : "0");
}
/**
 *  createSchema
 *    Create the structure to support the event builder database.
 *  @param interp - interpreter running the command.
 *  @param objv   - vector of command line words.
 */
void
CTCLEvbInstance::createSchema(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    requireExactly(objv, 2);
    m_pApi->createSchema();
}
/**
 * createEventBuilder
 *    Create a new event builder.  Required command parameters are
 *    the name of the event bulder and the host it runs in.
 *    An optional parameter, that is a dict can override the default
 *    values for the other parameters.
 *
 *  @param interp - interpreter running the command.
 *  @param objv   - vector of command line words.
 */
void
CTCLEvbInstance::createEventBuilder(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    requireAtLeast(objv, 4);
    std::string name = objv[2];
    std::string host = objv[3];
    
    // If there's an override dict set that otherwise just
    // use an empty dict for simplicity.
    
    requireAtMost(objv, 5);
    Tcl_Obj* overrides;
    if (objv.size() == 5) {
        overrides = objv[4].getObject();
    } else {
        overrides = Tcl_NewDictObj();   
    }
    // Set default values for all the other parameters:
    
    unsigned dt           = 1;
    unsigned sourceId     = 0;
    std::string svcPrefix = "ORDERER";
    bool build            = true;
    CVardbEventBuilder::TimestampPolicy tsPolicy = CVardbEventBuilder::earliest;
    std::string svcSuffix = "";
    
    
    // Apply overrides for any keys in the dict that matter.
    // Note that the code for timestamp is currently a bit wonky
    // because we tried to not include CVardbEventBuilder.h in our
    // header.
    
    std::string strOverride;
    unsigned    usOverride;
    bool        fOverride;
    
    if (getDictValue(usOverride, interp, overrides, "dt")) {
        dt = usOverride;
    }
    if (getDictValue(usOverride, interp, overrides, "sourceId")) {
        sourceId = usOverride;
    }
    if (getDictValue(strOverride, interp, overrides, "prefix")) {
        svcPrefix = strOverride;
    }
    if (getDictValue(fOverride, interp, overrides, "build")) {
        build = fOverride;
    }
    if (getDictValue(strOverride, interp, overrides, "suffix")) {
        svcSuffix = strOverride;
    }
    if (getDictValue(strOverride, interp, overrides, "tspolicy")) {
        tsPolicy = textToTsPolicy(strOverride);
    }
    
    // Create the event builder.
    
    m_pApi->createEventBuilder(
        name.c_str(), host.c_str(), dt, sourceId, svcPrefix.c_str(),
        build, tsPolicy, svcSuffix.c_str()
    );
}
/**
 * evbSetHost
 *    Sets a new host value for an existing event builder.
 *
 *  @param interp - interpreter running the command.
 *  @param objv   - vector of command line words.
 */
void
CTCLEvbInstance::evbSetHost(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    requireExactly(objv, 4);
    std::string name = objv[2];
    std::string host = objv[3];
    
    m_pApi->evbSetHost(name.c_str(), host.c_str());
}

/**
 * evbSetCoincidenceInterval
 *    Set a new coincidence interval for an existing event builder.
 *  @param interp - interpreter running the command.
 *  @param objv   - vector of command line words.
 */
void
CTCLEvbInstance::evbSetCoincidenceInterval(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    requireExactly(objv, 4);
    std::string name = objv[2];
    int         dt   = objv[3];
    
    m_pApi->evbSetCoincidenceInterval(name.c_str(), dt);
}
/**
 * evbSetSourceId
 *    SEt a new source id for an existing event builder.
 *  @param interp - interpreter running the command.
 *  @param objv   - vector of command line words.
 */
void
CTCLEvbInstance::evbSetSourceId(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    requireExactly(objv, 4);
    std::string name = objv[2];
    int         id   = objv[3];
    
    m_pApi->evbSetSourceId(name.c_str(), id);
}
/**
 *  evbSetServicePrefix
 *     Set a new value for a service prefix. This is part of what goes
 *     into determining how the event builder advertises itself to
 *     the port manager.
 *  @param interp - interpreter running the command.
 *  @param objv   - vector of command line words.
 */
void
CTCLEvbInstance::evbSetServicePrefix(
        CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    requireExactly(objv, 4);
    std::string name = objv[2];
    std::string prefix = objv[3];
    
    m_pApi->evbSetServicePrefix(name.c_str(), prefix.c_str());
}
/**
 * evbDisableBuild
 *   Turn off event building for a event builder description
 *  @param interp - interpreter running the command.
 *  @param objv   - vector of command line words.
 */
void
CTCLEvbInstance::evbDisableBuild(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    requireExactly(objv, 3);
    std::string name = objv[2];
    m_pApi->evbDisableBuild(name.c_str());
}
/**
 * evbEnableBuild
 *   Turn on event building for an event builder description.
 *
 *  @param interp - interpreter running the command.
 *  @param objv   - vector of command line words.
 */
void
CTCLEvbInstance::evbEnableBuild(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    requireExactly(objv, 3);
    std::string name = objv[2];
    m_pApi->evbEnableBuild(name.c_str());
}
/**
 * evbSetTimestampPolicy
 *    Set a new timestamp policy for an existing event builder.
 *  @param interp - interpreter running the command.
 *  @param objv   - vector of command line words.
 */
void
CTCLEvbInstance::evbSetTimestampPolicy(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    requireExactly(objv, 4);
    std::string name = objv[2];
    std::string strPolicy = objv[3];
    
    CVardbEventBuilder::TimestampPolicy policy;
    
    policy = textToTsPolicy(strPolicy);
    
    m_pApi->evbSetTimestampPolicy(name.c_str(), policy);
}
/**
 * evbSetServiceSuffix
 *    Set a new service suffix for an already defined event builder.
 *
 *  @param interp - interpreter running the command.
 *  @param objv   - vector of command line words.
 */
void
CTCLEvbInstance::evbSetServiceSuffix(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    requireExactly(objv, 4);
    std::string name = objv[2];
    std::string suff = objv[3];
    
    m_pApi->evbSetServiceSuffix(name.c_str(), suff.c_str());
}
/**
 * rmevb
 *     Remove an event builder.
 *
 *  @param interp - interpreter running the command.
 *  @param objv   - vector of command line words.
 */
void
CTCLEvbInstance::rmevb(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    requireExactly(objv, 3);
    std::string name = objv[2];
    
    m_pApi->rmEventBuilder(name.c_str());
}
/**
 * evbInfo
 *   Return information about an event builder in a dict.
 *
 *  @param interp - interpreter running the command.
 *  @param objv   - vector of command line words.
 *
 *  @note The interpreter return value for this, on success
 *        is a dict with the following key/values:
 *        *  name     - name of the event builder.
 *        *  host     - Host the event builder lives in.
 *        *  dt       - Ticks in the event builder coincidence interval.
 *        *  prefix   - service name prefix.
 *        *  suffix   - serivce name suffix.
 *        *  build    - boolean that is true if event bulding is enabled
 *                      false if not.
 *        *  sourceid - Id of fragments output by the event builder
 *                      pipeline.  This is only relevant for  hierarchical
 *                      event building,.
 *        *  tspolicy - timestamp policy chosen for the output fragments
 */
void
CTCLEvbInstance::evbInfo(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    requireExactly(objv, 3);
    std::string name = objv[2];
    CVardbEventBuilder::EvbDescription info = m_pApi->evbInfo(name.c_str());
    CTCLObject result;
    result.Bind(interp);
    evbInfoToDict(result, interp, &info);
    interp.setResult(result);
}
/**
 * evbList
 *     Create a list of info dicts for all of the known event builders
 *     and set it as the interpreter's result.
 *  @param interp - interpreter running the command.
 *  @param objv   - vector of command line words.
 */
void
CTCLEvbInstance::evbList(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    requireExactly(objv, 2);
    
    std::vector<CVardbEventBuilder::EvbDescription> descs =
        m_pApi->listEventBuilders();
        
    CTCLObject result;
    result.Bind(interp);
    
    for (int i = 0; i < descs.size(); i++) {
        CTCLObject info;
        info.Bind(interp);
        evbInfoToDict(info, interp, &(descs[i]));
        result += info;
    }
    
    interp.setResult(result);
}

/*-------------------------------------------------------------------
 * utility methods
 */

/**
 * getDictvalue
 *    Overload that returns a string value from a dict
 *    given its key.
 *
 *   @param[out] value - the output, resulting string.
 *   @param interp     - interpreter used to process the dict.
 *   @param dict       - Tcl_Obj* pointer that is the dict.
 *   @param key        - keyword string.
 *   @return bool  - true if the  dict was a dict and had the
 *                   requested key.  In that case, value can be
 *                   trusted.
 */
bool
CTCLEvbInstance::getDictValue(
    std::string& value, CTCLInterpreter& interp, Tcl_Obj* dict,
    const char* key
)
{
    // Value must be turned into a Tcl object:
    
    Tcl_Obj* oKey = Tcl_NewStringObj(key, -1);
    Tcl_Obj* oValue;
    
    int status =
        Tcl_DictObjGet(interp.getInterpreter(), dict, oKey, &oValue);
    if ((status != TCL_OK) || (!oValue)) {
        return false;
    }
    value = Tcl_GetString(oValue);
    return true;
}
/**
 * getDictValue
 *    same as above but the value is an unsigned integer.
 */
bool
CTCLEvbInstance::getDictValue(
    unsigned& value, CTCLInterpreter& interp, Tcl_Obj* dict,
    const char* key
)
{
    
    // First get the value as a string:
    
    std::string strValue;
    bool status = getDictValue(strValue, interp, dict, key);
    if (!status) return status;
    
    // If the string can be interpreted as an unsigned we're cool.
    
    char* endptr;
    value = strtoul(strValue.c_str(), &endptr, 0);
    
    return endptr != strValue.c_str();
}
/**
 * getDictValue
 *   Same as above, but value is a boolean.
 */
bool
CTCLEvbInstance::getDictValue(
    bool& value, CTCLInterpreter& interp, Tcl_Obj* dict,
    const char* key
)
{
    // First get this as a string:
    
    int iBool;
    std::string strValue;
    bool ok = getDictValue(strValue, interp, dict, key);
    if (!ok) return ok;
    
    // If the stringt is a valid bool we can return its value:
    
    int status =
        Tcl_GetBoolean(interp.getInterpreter(), strValue.c_str(), &iBool);
    if (status != TCL_OK) return false;
    
    value = bool(iBool);
    
}
/**
 * setDictValue
 *    Set the value of a dict key.  In this overload, the value
 *    is an std::string
 *
 * @param interp - intepreter that will manipulate the dict.
 * @param pDict  - the dict Tcl_Obj*
 * @param key    - The key
 * @param value  - The value.
 */
void
CTCLEvbInstance::setDictValue(
        CTCLInterpreter& interp, Tcl_Obj* pDict, const char* key,
        const char* value
    )
{
    Tcl_Obj* pKey = Tcl_NewStringObj(key, -1);
    Tcl_Obj* pValue = Tcl_NewStringObj(value, -1);
    
    int status = Tcl_DictObjPut(interp.getInterpreter(), pDict, pKey, pValue);
    if (status != TCL_OK) {
        throw std::runtime_error("Failed to set the value of a dict key");
    }
}
/**
 * setDictValue
 *   Same as above, however in this overload, the value is an
 *   unsigned integer.
 */
void
CTCLEvbInstance::setDictValue(
        CTCLInterpreter& interp, Tcl_Obj* pDict, const char* key,
        unsigned value
)
{
    Tcl_Obj* pValue = Tcl_NewLongObj(value);
    setDictValue(interp, pDict, key, Tcl_GetStringFromObj(pValue, NULL));
}
/**
 * setDictValue
 *    Same as above but the value is a bool.
 */
void
CTCLEvbInstance::setDictValue(
        CTCLInterpreter& interp, Tcl_Obj* pDict, const char* key,
        bool value
)
{
   setDictValue(interp, pDict, key, value ? "true" : "false");
    
}
/**
 * tsPolicyToText
 *    Convert a timestamp policy to text.  If the the value is somehow
 *    not a timestamp policy, std::invalid_argument is thrown.
 *
 *  @param policy - timestamp policy value:
 *  @return std::string - string representation of policy
 */
std::string
CTCLEvbInstance::tsPolicyToText(CVardbEventBuilder::TimestampPolicy policy)
{
    switch (policy) {
    case CVardbEventBuilder::earliest:
        return std::string("earliest");
    case CVardbEventBuilder::average:
        return std::string("latest");
    case CVardbEventBuilder::latest:
        return std::string("latest");
    default:
        throw std::invalid_argument("Invalid timestamp policy value");
    }
}
/**
 * textToTsPolicy
 *   Convert a string into the corresponding timestamp policy.
 *   If there's no correspondence, an std::invalid_argument is thrown.
 *
 * @param policy - text representation of a timestamp policy.
 * @return CVardbEventBuilder::TimestampPolicy - corresponding to policy
 */
CVardbEventBuilder::TimestampPolicy
CTCLEvbInstance::textToTsPolicy(std::string policy)
{
    if (policy == "earliest") {
        return CVardbEventBuilder::earliest;
    }
    if (policy == "latest") {
        return CVardbEventBuilder::latest;
    }
    if (policy == "average") {
        return CVardbEventBuilder::average;
    }
    throw std::invalid_argument("Invalid time stamp policy string rep.");
}
/**
 * evbInfoToDict
 *    Produce a dict that describes an event builder info struct.
 *    See evbInfo for a description of the keys.
 *
 * @param[out] infoDict - the CTCLObject that will be the dict.
 *                        this is considered bound to the interpreter.
 * @param      interp   - Interpreter object.
 * @param      pInfo    - Pointer to the event builder information.
 */
void
CTCLEvbInstance::evbInfoToDict(
        CTCLObject& infoDict, CTCLInterpreter& interp,
        CVardbEventBuilder::pEvbDescription pInfo
)
{
    // We must use direct tcl obj calls so:
    
    Tcl_Obj* pDict = Tcl_NewDictObj();
    
    setDictValue(interp, pDict, "name", pInfo->s_name.c_str());
    setDictValue(interp, pDict, "host", pInfo->s_host.c_str());
    setDictValue(interp, pDict, "dt",   pInfo->s_coincidenceInterval);
    setDictValue(interp, pDict, "prefix", pInfo->s_servicePrefix.c_str());
    setDictValue(interp, pDict, "suffix", pInfo->s_serviceSuffix.c_str());
    setDictValue(interp, pDict, "build", pInfo->s_build);
    setDictValue(interp, pDict, "sourceId", pInfo->s_sourceId);
    setDictValue(
        interp, pDict, "tspolicy",
        tsPolicyToText(pInfo->s_timestampPolicy).c_str()
    );

    infoDict = pDict;   
}