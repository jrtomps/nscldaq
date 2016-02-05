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
        } else if (subcommand == "evbSetRing") {
            evbSetRing(interp, objv);
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
        } else if (subcommand == "evbSetEditorPosition") {
            
            evbSetEditorPosition(interp, objv);
        } else if (subcommand == "evbGetEditorXPosition") {
            evbGetEditorXPosition(interp, objv);
        } else if (subcommand == "evbGetEditorYPosition") {
            evbGetEditorYPosition(interp, objv);
        } else if (subcommand == "rmevb") {
            rmevb(interp, objv);
        } else if (subcommand == "evbInfo") {
            evbInfo(interp, objv);
        } else if (subcommand == "evbList") {
            evbList(interp, objv);
        } else if (subcommand == "addSource") {
            addSource(interp, objv);
        } else if (subcommand == "dsSetHost") {
            dsSetHost(interp, objv);
        } else if (subcommand == "dsSetPath") {
            dsSetPath(interp, objv);
        } else if (subcommand == "dsSetRingUri") {
            dsSetRingUri(interp, objv);
        }  else if (subcommand == "dsSetIds") {
            dsSetIds(interp, objv);
        } else if (subcommand == "dsSetInfo") {
            dsSetInfo(interp, objv);
        } else if (subcommand == "dsSetDefaultId") {
            dsSetDefaultId(interp, objv);
        } else if (subcommand == "dsExpectBodyHeaders") {
            dsEnableBodyHeaders(interp, objv);
        } else if (subcommand == "dsDontExpectBodyHeaders") {
            dsDisableBodyHeaders(interp, objv);
        } else if (subcommand == "dsSetTimestampExtractor") {
            dsSetTimestampExtractor(interp, objv);
        } else if (subcommand == "dsSetEditorPosition") {
            
            dsSetEditorPosition(interp, objv);
        } else if (subcommand == "dsGetEditorXPosition") {
            dsGetEditorXPosition(interp, objv);
        } else if (subcommand == "dsGetEditorYPosition") {
            dsGetEditorYPosition(interp, objv);
        } else if (subcommand == "dsInfo") {
            dsInfo(interp, objv);
        } else if (subcommand == "listSources") {
            listSources(interp, objv);
        } else if (subcommand == "rmSource") {
            rmSource(interp, objv);
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
    requireAtLeast(objv, 5);
    std::string name = objv[2];
    std::string host = objv[3];
    std::string ring = objv[4];
    
    // If there's an override dict set that otherwise just
    // use an empty dict for simplicity.
    
    requireAtMost(objv, 6);
    Tcl_Obj* overrides;
    if (objv.size() == 6) {
        overrides = objv[5].getObject();
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
        name.c_str(), host.c_str(), dt, ring.c_str(), sourceId, svcPrefix.c_str(),
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
 * evbSetRing
 *    Set a new event builder output ring buffer.
 *
 *  @param interp - interpreter running the command.
 *  @param objv   - command words vector.
 */
void
CTCLEvbInstance::evbSetRing(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    requireExactly(objv, 4);
    std::string name = objv[2];
    std::string ring = objv[3];
    
    m_pApi->evbSetRing(name.c_str(), ring.c_str());
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
/**
 * addSource
 *    Add an event source for an existing event builder.
 *   At a minimum this needs parameters for the
 *   -  Event builder name.
 *   -  New source name.
 *   -  Host in which the source runs.
 *   -  New source path
 *   -  ring URL
 *   -  List of source ids.
 *
 *   Additionally there can be an optional dict that can override
 *   default values for the remaining data source parameters.
 *
 *   -  defaultId - default data source id for items without body headers.
 *   -  timestampExtractor - Path to timestamp extractor .so which is used
 *      to get timestamps from ring items with no body header.
 *   -  expectBodyHeaders - true if all ring items are expected to have
 *      boy headers, false if not.
 *
 *  @param interp - interpreter running the command.
 *  @param objv   - vector of command line words.
 */
void
CTCLEvbInstance::addSource(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    requireAtLeast(objv, 8);            // without override dicst.
    requireAtMost(objv, 9);             // with override dict.
    
    std::string evbName = objv[2];
    std::string srcName = objv[3];
    std::string host    = objv[4];
    std::string path    = objv[5];
    std::string ringUri = objv[6];
    
    
    // Marshall objv[6] into a vector<unsigned>
    
    std::vector<unsigned> srcIds;
    for (int i =0;  i < objv[7].llength(); i++) {
        int id = objv[7].lindex(i);
        srcIds.push_back(unsigned(id));
    }
    // The override dict will either be an empty dict or
    // objv[8] if there are 9 parameters.
    
    Tcl_Obj* overrides;
    if (objv.size() == 9) {
       overrides = objv[8].getObject(); 
    } else {
        overrides = Tcl_NewDictObj();
    }
    // Set default values for the remaining parameters:
    
    std::string info;
    bool        expectBodyHeaders(true);
    unsigned    defaultId(0);
    std::string tsExtractLib;
    
    // Overide with dict items.
    
    std::string strValue;
    unsigned    usValue;
    bool        bValue;
    
    if(getDictValue(strValue, interp, overrides, "info")) {
        info = strValue;
    }
    if (getDictValue(bValue, interp, overrides, "expectBodyHeaders")) {
        expectBodyHeaders = bValue;
    }
    if (getDictValue(usValue, interp, overrides, "defaultId")) {
        defaultId = usValue;
    }
    if (getDictValue(strValue, interp, overrides, "timestampExtractor")) {
        tsExtractLib = strValue;
    }
    
    // If the evbName is an empty string this is a ronin event builder:'
    //
    const char *pEvbName = evbName.c_str();
    if (evbName == "") {
        pEvbName = 0; 
    }
    
    m_pApi->addDataSource(
        pEvbName, srcName.c_str(), host.c_str(),
        path.c_str(), ringUri.c_str(),
        srcIds, info.c_str(), expectBodyHeaders, defaultId, tsExtractLib.c_str()
        
    );
    
    
}
/**
 * dsSetHost
 *    set the host of an existing event source.
 *
 *  @param interp - interpreter running the command.
 *  @param objv   - vector of command line words.
 */
void
CTCLEvbInstance::dsSetHost(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    requireExactly(objv, 5);
    
    std::string evbName = objv[2];
    std::string dsName  = objv[3];
    std::string host    = objv[4];
    
    m_pApi->dsSetHost(evbName.c_str(), dsName.c_str(), host.c_str());
}
/**
 * dsSetPath
 *   Change the path to an existing event source.
 *
 *  @param interp - interpreter running the command.
 *  @param objv   - vector of command line words.
 */
void
CTCLEvbInstance::dsSetPath(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
requireExactly(objv, 5);
    
    std::string evbName = objv[2];
    std::string dsName  = objv[3];
    std::string path    = objv[4];
    
    m_pApi->dsSetPath(evbName.c_str(), dsName.c_str(), path.c_str());
}
/** dsSetRingUri
 *    Change the input ring URI for an existing data source.
 *
 *  @param interp - interpreter running the command.
 *  @param objv   - vector of command line words.
 */
void
CTCLEvbInstance::dsSetRingUri(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    requireExactly(objv, 5);
    std::string evbName = objv[2];
    std::string dsName  = objv[3];
    std::string ring    = objv[4];
    
    m_pApi->dsSetRingUri(evbName.c_str(), dsName.c_str(), ring.c_str());
}
/**
 * dsSetIds
 *    Set a new list of data source ids for an existing data source.
 *
 *  @param interp - interpreter running the command.
 *  @param objv   - vector of command line words.
 */
 void
 CTCLEvbInstance::dsSetIds(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
 {
    requireExactly(objv, 5);
    std::string evbName = objv[2];
    std::string dsName  = objv[3];
    
    std::vector<unsigned> ids;
    for (int i =0; i < objv[4].llength(); i++) {
        ids.push_back(int(objv[4].lindex(i)));
    }
    m_pApi->dsSetIds(evbName.c_str(), dsName.c_str(), ids);
 }
 /**
  * dsSetInfo
  *    Sets the info string of an existing data source.
  *
  *  @param interp - interpreter running the command.
  *  @param objv   - vector of command line words.
 */
void
CTCLEvbInstance::dsSetInfo(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    requireExactly(objv, 5);
    std::string evb = objv[2];
    std::string src = objv[3];
    std::string info = objv[4];
    
    m_pApi->dsSetInfo(evb.c_str(), src.c_str(), info.c_str());
}

/**
 * dsSetDefaultId
 *    Set a new default source id for an existing data source.
 *
  *  @param interp - interpreter running the command.
  *  @param objv   - vector of command line words.
 */
void
CTCLEvbInstance::dsSetDefaultId(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    requireExactly(objv, 5);
    std::string evb = objv[2];
    std::string src = objv[3];
    int         id  = objv[4];
    
    m_pApi->dsSetDefaultId(evb.c_str(), src.c_str(), id);
}
/**
 * dsEnableBodyHeaders
 *    Turn on the expect body headers flag.
  *  @param interp - interpreter running the command.
  *  @param objv   - vector of command line words.
 */
void
CTCLEvbInstance::dsEnableBodyHeaders(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    requireExactly(objv, 4);
    std::string evb = objv[2];
    std::string ds  = objv[3];
    
    m_pApi->dsExpectBodyHeaders(evb.c_str(), ds.c_str());
}
/**
 * disableBodyHeaders
 *    turn off the expect body headers flag.
 *
  *  @param interp - interpreter running the command.
  *  @param objv   - vector of command line words.
 */
void
CTCLEvbInstance::dsDisableBodyHeaders(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    requireExactly(objv, 4);
    std::string evb = objv[2];
    std::string ds  = objv[3];
    
    m_pApi->dsDontExpectBodyHeaders(evb.c_str(), ds.c_str());
}

/**
 * dsSetTimestampExtractor
 *    modify an data source's timestamp extractor library
 *    spec.
 *
  *  @param interp - interpreter running the command.
  *  @param objv   - vector of command line words.
 */
void
CTCLEvbInstance::dsSetTimestampExtractor(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
     requireExactly(objv, 5);
    std::string evb = objv[2];
    std::string ds  = objv[3];
    std::string ts  = objv[4];
    
    m_pApi->dsSetTimestampExtractor(evb.c_str(), ds.c_str(), ts.c_str());
}

/**
 * dsInfo
 *
 *  Sets the interpreter result to a dict that describes
 *  a data source.  The dict has the following keys:
 *
 *  *  name - Name of the data source.
 *  *  host - host in which the data source runs.
 *  *  path - Path to the data source program in host.
 *  *  info - Information string.
 *  *  ring - Ring buffer URI from which pogram takes data.
 *  *  ids  - list of ids
 *  *  defaultId - data source id for items that don't have body headers.
 *  *  timestampExtractor - path to the timestamp extractor.
 *  *  expectBodyHeaders - true if data are supposed to all have bodfy
 *  *                      headers.
 *
   *  @param interp - interpreter running the command.
  *   @param objv   - vector of command line words.
 */
void
CTCLEvbInstance::dsInfo(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    requireExactly(objv, 4);
    std::string evb = objv[2];
    std::string src = objv[3];
    
    CVardbEventBuilder::DsDescription desc =
        m_pApi->dsInfo(evb.c_str(), src.c_str());
        
        // convert to tcl dict.
    
    CTCLObject result;
    result.Bind(interp);
    dsInfoToDict(result, interp, desc);
    
    interp.setResult(result);
}
/**
 * listSources
 *    Produce a list of dicts that contain information about the data
 *    sources for an event builder.  The dicts have the same form as those
 *    returned by dsInfo for a specific data source
 *
 *  @param interp - interpreter running the command.
 *   @param objv   - vector of command line words.
*/
void
CTCLEvbInstance::listSources(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    requireExactly(objv, 3);
    std::string evb = objv[2];
    
    std::vector<CVardbEventBuilder::DsDescription> descriptions =
        m_pApi->listDataSources(evb.c_str());
        
    CTCLObject result;
    result.Bind(interp);
    for (int i = 0; i < descriptions.size(); i++) {
        CTCLObject element;
        element.Bind(interp);
        dsInfoToDict(element, interp, descriptions[i]);
        result += element;
    }
    interp.setResult(result);
}
/**
 * rmSource
 *   Remove a data source.
 *
 *  @param interp - interpreter running the command.
 *   @param objv   - vector of command line words.
*/
void
CTCLEvbInstance::rmSource(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    requireExactly(objv, 4);
    std::string evb = objv[2];
    std::string ds  = objv[3];
    
    m_pApi->rmDataSource(evb.c_str(), ds.c_str());
}


/**
 * evbSetEditorPosition
 *    Sets the position of an event builder on the editor canvas.
 *
 *  @param interp - interpreter running the command.
 *  @param objv   - command line words (api evbSetEditorPosition name x y).
 */
void CTCLEvbInstance::evbSetEditorPosition(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    requireExactly(objv, 5);
    std::string name = objv[2];
    int x            = objv[3];
    int y            = objv[4];
    
    m_pApi->evbSetEditorPosition(name.c_str(), x, y);
}

/**
 * evbGetEditorXPosition
 *     Set the interpreter result with the x coordinate of the last saved
 *     editor canvas position for this object.
 *
 * @param interp - interpreter running he command.
 * @param objv   - command line words (api evbGetEditorXPosition name)
 */
void
CTCLEvbInstance::evbGetEditorXPosition(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    requireExactly(objv, 3);
    std::string name = objv[2];
    
    CTCLObject result;
    result.Bind(interp);
    result = m_pApi->evbGetEditorXPosition(name.c_str());
    interp.setResult(result);
}
/**
 * evbGetEditorYPosition
 *     Set the interpreter result with the y coordinate of the last saved
 *     editor canvas position for this object.
 *
 * @param interp - interpreter running he command.
 * @param objv   - command line words (api evbGetEditorXPosition name)
 */
void
CTCLEvbInstance::evbGetEditorYPosition(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    requireExactly(objv, 3);
    std::string name = objv[2];
    
    CTCLObject result;
    result.Bind(interp);
    result = m_pApi->evbGetEditorYPosition(name.c_str());
    interp.setResult(result);
}


/**
 * dsSetEditorPosition
 *   Set the position of a data source object on the editor canvas.
 *
 * @param interp - interpreter running the command.
 * @param objv   - Command line words (api dsSetEditorPosition evb ds x y)
 */
void
CTCLEvbInstance::dsSetEditorPosition(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    requireExactly(objv, 6);
    std::string evb = objv[2];
    std::string ds  = objv[3];
    int x           = objv[4];
    int y           = objv[5];
    
    m_pApi->dsSetEditorPosition(evb.c_str(), ds.c_str(), x, y);
}
/**
 *  dsGetEditorXPosition
 *     Return the X coordinate of the position of a data source  on the editor
 *     canvas
 *
 * @param interp - interpreter running the command.
 * @param objv   - command words (api evbGetEditorXPosition evb ds)
 */
void
CTCLEvbInstance::dsGetEditorXPosition(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    requireExactly(objv, 4);
    
    std::string evb = objv[2];
    std::string ds  = objv[3];
    
    CTCLObject result;
    result.Bind(interp);
    result = m_pApi->dsGetEditorXPosition(evb.c_str(), ds.c_str());
    interp.setResult(result);
}

/**
 *  dsGetEditorYPosition
 *     Return the Y coordinate of the position of a data source  on the editor
 *     canvas
 *
 * @param interp - interpreter running the command.
 * @param objv   - command words (api evbGetEditorXPosition evb ds)
 */
void
CTCLEvbInstance::dsGetEditorYPosition(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    requireExactly(objv, 4);
    
    std::string evb = objv[2];
    std::string ds  = objv[3];
    
    CTCLObject result;
    result.Bind(interp);
    result = m_pApi->dsGetEditorYPosition(evb.c_str(), ds.c_str());
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
    return true;
    
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
    if (pInfo->s_name != " ") {
        setDictValue(interp, pDict, "host", pInfo->s_host.c_str());
        setDictValue(interp, pDict, "dt",   pInfo->s_coincidenceInterval);
        setDictValue(interp, pDict, "ring", pInfo->s_ring.c_str());
        setDictValue(interp, pDict, "prefix", pInfo->s_servicePrefix.c_str());
        setDictValue(interp, pDict, "suffix", pInfo->s_serviceSuffix.c_str());
        setDictValue(interp, pDict, "build", pInfo->s_build);
        setDictValue(interp, pDict, "sourceId", pInfo->s_sourceId);
        setDictValue(
            interp, pDict, "tspolicy",
            tsPolicyToText(pInfo->s_timestampPolicy).c_str()
        );

    }
    infoDict = pDict;   
}

/**
 * dsInfoToDict
 *    Convert data source information (DsDescription) to a dict that
 *    describes the data source in a natural way for Tcl scripts.
 *
 *  @param[out] result - CTCLObject reference that encapsulates the resulting
 *                       dict.  The caller should have bound this to an
 *                       interpreter
 *  @param interp      - Interpreter used by the dict functions in the Tcl API
 *  @param desc        - Description of the data source.
 *  @note when completed, result will be a dict with the following keys:
 *  *  name - Name of the data source.
 *  *  host - host in which the data source runs.
 *  *  path - Path to the data source program in host.
 *  *  info - Information string.
 *  *  ring - Ring buffer URI from which pogram takes data.
 *  *  ids  - list of ids
 *  *  defaultId - data source id for items that don't have body headers.
 *  *  timestampExtractor - path to the timestamp extractor.
 *  *  expectBodyHeaders - true if data are supposed to all have bodfy
 *  *                      headers.
 */
void
CTCLEvbInstance::dsInfoToDict(
        CTCLObject& result, CTCLInterpreter& interp,
        const CVardbEventBuilder::DsDescription& desc
)
{
    Tcl_Obj* dict = Tcl_NewDictObj();       // Build the dict in this.
    setDictValue(interp, dict, "name", desc.s_name.c_str());
    setDictValue(interp, dict, "host", desc.s_host.c_str());
    setDictValue(interp, dict, "path", desc.s_path.c_str());
    setDictValue(interp, dict, "info", desc.s_info.c_str());
    setDictValue(interp, dict, "ring", desc.s_ringUri.c_str());
    // Marshall the ids into a list:
    
    CTCLObject ids;
    ids.Bind(interp);
    for (int i = 0; i < desc.s_ids.size(); i++) {
        ids += int(desc.s_ids[i]);
    }
    setDictValue(interp, dict, "ids", std::string(ids).c_str());
    setDictValue(interp, dict, "defaultId", desc.s_defaultId);
    setDictValue(
        interp, dict, "timestampExtractor", desc.s_timestampExtractor.c_str()
    );
    setDictValue(interp, dict, "expectBodyHeaders", desc.s_expectBodyheaders);
    
    // Set the resulting CTCLObject
    
    result = dict;
    
}