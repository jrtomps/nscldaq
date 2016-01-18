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
# @file   PyEventBuilder.cpp
# @brief  Python bindings to the database event builder api.
# @author <fox@nscl.msu.edu>
*/


#include <Python.h>
#include "CVardbEventBuilder.h"
#include <stdexcept>
#include <string>


/*----------------------------------------------------------------------------
 * object storage for VarDbEvb - there's not really any so:
 */

typedef struct {
    PyObject_HEAD;
    CVardbEventBuilder* m_pApi;
} vardbEvb_Data;

static PyObject* exception;

/*-----------------------------------------------------------------------------
 * Utility Methods
 */


// Pull the api from object storage.

static CVardbEventBuilder* getApi(PyObject* self)
{
    vardbEvb_Data* pThis = reinterpret_cast<vardbEvb_Data*>(self);
    return pThis->m_pApi;
}

// Convert a string to the corresponding timestamp policy.

static CVardbEventBuilder::TimestampPolicy
strToTsPolicy(const char* strPolicy)
{
    std::string policy(strPolicy);
    
    if (policy == "earliest") {
        return CVardbEventBuilder::earliest;
    } else if (policy == "latest") {
        return CVardbEventBuilder::latest;
    } else if (policy == "average") {
        return CVardbEventBuilder::average;
    } else {
        throw std::runtime_error("Invalid timestamp policy");
    }
}

static std::string
tsPolicyToString(CVardbEventBuilder::TimestampPolicy policy)
{
    switch (policy) {
    case CVardbEventBuilder::earliest:
        return std::string("earliest");
    case CVardbEventBuilder::latest:
        return std::string("latest");
    case CVardbEventBuilder::average:
        return std::string("average");
    }
    return std::string("invalid timestamp policy!!");
}

// Functions to add objets to dicts.

static void
AddToDict(PyObject* dict, const char* key, const char* data)
{
    // Turn the character data into a PyObject*
    
    PyObject* objString = PyString_FromString(data);
    PyDict_SetItemString(dict, key, objString);
}

static void
AddToDict(PyObject* dict, const char* key, bool data)
{
    // Convert the boolean into an object:
    
    PyObject* bObject = data ? Py_True: Py_False;
    PyDict_SetItemString(dict, key, bObject);
}

static void
AddToDict(PyObject* dict, const char* key, unsigned data)
{
    PyObject* uObject = PyLong_FromUnsignedLong(data);
    PyDict_SetItemString(dict, key, uObject);
}

/**
 * evbInfoToDict
 *    Given a CVardbEventBuilder::EvbDescription, returns the equivalent
 *    Python Dict for that description.
 *
 *  @param info - Information about an event buider.
 *  @return PyObject - dict, see VardbEvb_evbInfo for the keys.
 */
static PyObject*
evbInfoToDict(const CVardbEventBuilder::EvbDescription& info)
{
    PyObject* result = PyDict_New();
    AddToDict(result, "name", info.s_name.c_str());
    AddToDict(result, "host", info.s_host.c_str());
    AddToDict(result, "coincidenceInterval", info.s_coincidenceInterval);
    AddToDict(result, "ring", info.s_ring.c_str());
    AddToDict(result, "servicePrefix", info.s_servicePrefix.c_str());
    AddToDict(result, "serviceSuffix", info.s_serviceSuffix.c_str());
    AddToDict(result, "build", info.s_build);
    AddToDict(result, "sourceId", info.s_sourceId);
    AddToDict(
        result, "timestampPolicy",
        tsPolicyToString(info.s_timestampPolicy).c_str()
    );
    
    return result;
}


// Turn an iterable into an vector<unsigned>, raises an exception on failure.
// 
static std::vector<unsigned>
iterableToUnsignedVector(PyObject* o)
{
    std::vector<unsigned> result;
    PyObject* p = PyObject_GetIter(o);
    if (!p) {
        PyErr_SetString(exception, "Object must be an interable");
        throw std::string("Object must be an iterable");
    }
    
    
    PyObject* item;
    while(item = PyIter_Next(p)) {
        if (!PyInt_Check(item)) {
            PyErr_SetString(exception, "All items must be integer");
            Py_DECREF(item);
            Py_DECREF(p);
            throw std::string("A non integer item encountered");
        }
        unsigned i = static_cast<unsigned>(PyInt_AsLong(item));
        result.push_back(i);
        
        Py_DECREF(item);
    }
    
    Py_DECREF(p);
    
    return result;
    
}

// convert a std::vector<unsigned> to a PyTuple:

PyObject*
usVectorToTuple(const std::vector<unsigned>& v)
{
    PyObject* result = PyTuple_New(v.size());
    for (int i = 0; i < v.size(); i++) {
        PyObject* element = PyInt_FromLong(v[i]);
        PyTuple_SetItem(result, i, element);
    }
    
    return result;
}

/**
 * dsInfoToDict
 *    Turn a data source description struct into a dict.
 *    See VardbEvb_dsInfo for the keys and their meanings.
 *
 * @param info - the DsDescription of the data source.
 * @return PyDict*
 */
PyObject*
dsInfoToDict(const CVardbEventBuilder::DsDescription& info)
{
    PyObject* result = PyDict_New();
    
    AddToDict(result, "name", info.s_name.c_str());
    AddToDict(result, "host", info.s_host.c_str());
    AddToDict(result, "path", info.s_path.c_str());
    AddToDict(result, "info", info.s_info.c_str());
    PyDict_SetItemString(result, "ids", usVectorToTuple(info.s_ids));
    AddToDict(result, "ring", info.s_ringUri.c_str());
    AddToDict(result, "bodyheaders", info.s_expectBodyheaders);
    AddToDict(result, "defaultId", info.s_defaultId);
    AddToDict(result, "tsextractor", info.s_timestampExtractor.c_str());
    
    return result;

}
/*--------------------------------------------------------------------------
 * Methods on constructed objects of VardbEvb type:
 */



/**
 * VardbEvb_schemaExists
 *    See if the schema for event builders exists in the database we are
 *    connected to.
 *
 *  @param self - Pointer to the object storage.
 *  @param args - N-tuple containing our parameters (none).
 *  @return PyObject*
 *  @retval Py_True - the schema already exists.
 *  @retval Py_False - The schema has not yet been created.
 */
static PyObject*
VardbEvb_schemaExists(PyObject* self, PyObject* args)
{
    // Ensure we don't have any spurious parameters:
    
    if (PyTuple_Size(args) > 0) {
        PyErr_SetString(exception, "schemaExists takes no parameters");
        return NULL;
    }
    CVardbEventBuilder* pApi = getApi(self);
    bool result;
    try {
        result = pApi->schemaExists();
    }
    catch(std::exception& e) {
        PyErr_SetString(exception, e.what());
        return NULL;
    }
    if (result) {
        Py_RETURN_TRUE;
    } else {
        Py_RETURN_FALSE;
    }
}

/**
 * VardbEvb_createSchema
 *    Create the event builder schema in the variable database.
 *
 *   @param self - pointer to our object data.
 *   @param args - Positional parameters.
 *   @return Py_None
 */
static PyObject*
VardbEvb_createSchema(PyObject* self, PyObject* args)
{
    if (PyTuple_Size(args) > 0) {
        PyErr_SetString(exception, "createSchema requires no parameters");
        return NULL;
    }
    
    CVardbEventBuilder* pApi = getApi(self);
    
    try {
        pApi->createSchema();
    }
    catch (std::exception& e) {
        PyErr_SetString(exception, e.what());
        return NULL;
    }
    
    Py_RETURN_NONE;
}
/**
 * VardbEvb_createEventBuilder
 *    Create a new event builder.  A few words about the parameters.
 *    The method requires a set of positional (mandatory) parameters. These
 *    can be augmented by keyword parameters, for which reasonable defaults
 *    exist if the keyword parameter is not supplied.
 *
 *  @param self   - Pointer to our data.
 *  @param args   - Positional parameters.
 *  @param kwargs - Keyword parameters.  The legal keywords and their meaning
 *                  are:
 *                  - sourceId      - The source id for built events.
 *                  - servicePrefix - Prefix for the service name the builder
 *                                    registers with the port manager.
 *                  - build         - True if event building should be done.
 *                  - tsPolicy      - timestamp policy.
 *                  - serviceSuffix - Suffix for the service name th builder
 *                                    registers with the port manager.
 *  @return Py_NONE
 *
 */
static PyObject*
VardbEvb_createEventBuilder(PyObject* self, PyObject* args, PyObject* kwArgs)
{
    // The keyword list.  Seems like we need to invent keywords for the
    // positionals as well.
    static const char* keywords[] = {
        "name", "host", "dt", "ring",                 // Positional actually.
        "sourceId", "servicePrefix", "build", "tsPolicy", "serviceSuffix",
        NULL
    };
    // Parameters and their defaults:

    //      Positional params:
    
    char* name;
    char* host;
    unsigned dt;
    char* ring;
    
    // keyword params need defaults:
    
    int sourceId(0);
    const char* servicePrefix = "ORDERER";
    bool        build         = true;
    PyObject*   oBuild        = Py_True;
    const char* tsPolicy      = "earliest";
    const char* serviceSuffix = "";
    
    // Process the parameters.
    
    if (! PyArg_ParseTupleAndKeywords(
        args, kwArgs, "ssis|isOss", const_cast<char**>(keywords),
        &name, &host, &dt, &ring, &sourceId, &servicePrefix,
        &oBuild, &tsPolicy, &serviceSuffix
        )
    ) {
        return NULL;             // Parse raised appropriate exception.
    }
    // Build must be boolean:
    
    if (!PyBool_Check(oBuild)) {
        PyErr_SetString(exception, "build parameter must be a boolean");
        return NULL;
    }
    build = (oBuild == Py_True) ? true : false;
    
    // Get the API and attempt to register the event builder.
    
    CVardbEventBuilder* pApi = getApi(self);
    try {
        pApi->createEventBuilder(
            name, host, dt, ring, sourceId, servicePrefix, (build != 0),
            strToTsPolicy(tsPolicy), serviceSuffix
        );
    }
    catch (std::exception& e) {
        PyErr_SetString(exception, e.what());
        return NULL;
    }
    Py_RETURN_NONE;
}
/**
 * VardbEvb_setEvbHost
 *    Set the host in which an existing event builder will run.
 *
 *  @param self - Object containing our data.
 *  @param args - Positional args, evbName, newHost
 *  @return Py_NONE
 */
static PyObject*
VardbEvb_setEvbHost(PyObject* self, PyObject* args)
{
    char* evb;
    char* host;
    
    if (!(PyArg_ParseTuple(args, "ss", &evb, &host))) {
        return NULL;
    }
    
    CVardbEventBuilder* pApi = getApi(self);
    try {
        pApi->evbSetHost(evb, host);
    }
    catch (std::exception& e) {
        PyErr_SetString(exception, e.what());
        return NULL;
    }
    Py_RETURN_NONE;
}
/**
 * VardbEvb_setEvbCoincidenceInterval
 *    Set a new coincidence interval for event building in an event builder.
 *
 *  @param self - Object on which this method is being called.
 *  @param args - method positonal arguments : evbname, newInterval
 *  @return Py_None
 */
static PyObject*
VardbEvb_setEvbCoincidenceInterval(PyObject* self, PyObject* args)
{
    char*    evb;
    unsigned interval;
    
    if(!PyArg_ParseTuple(args, "sI", &evb, &interval)) {
        return NULL;
    }
    
    CVardbEventBuilder* pApi = getApi(self);
    try {
        pApi->evbSetCoincidenceInterval(evb, interval);
    }
    catch (std::exception& e) {
        PyErr_SetString(exception, e.what());
        return NULL;
    }
    Py_RETURN_NONE;
}
/**
 * VardbEvb_setEvbRing
 *    set a new output ring for the event builder
 *
 *    @param self - Object on which the method is being invoked.
 *    @param args - position args: event builder name, new ring name.
 *    @return Py_None
 */
static PyObject*
VardbEvb_setEvbRing(PyObject* self, PyObject* args)
{
    char*   name;
    char*   ring;
    
    if (!PyArg_ParseTuple(args, "ss", &name, &ring)) {
        return NULL;
    }
    
    CVardbEventBuilder* pApi = getApi(self);
    
    try {
        pApi->evbSetRing(name, ring);
    }
    catch (std::exception& e) {
        PyErr_SetString(exception, e.what());
        return NULL;
    }
    
    Py_RETURN_NONE;
}
/**
 * VardbEvb_setEvbSourceId
 *    Set the source id of data outputted by an existing event builder
 *    definition.
 *
 *  @param self - Object on which this method is being called.
 *  @param args - method positonal arguments : evbname, newId
 *  @return Py_None
 */
static PyObject*
VardbEvb_setEvbSourceId(PyObject* self, PyObject* args)
{
    char* evb;
    unsigned id;
    
    if (! PyArg_ParseTuple(args, "sI", &evb, &id)) {
        return NULL;
    }
    
    CVardbEventBuilder* pApi = getApi(self);
    try {
        pApi->evbSetSourceId(evb, id);
    }
    catch(std::exception& e) {
        PyErr_SetString(exception, e.what());
    }
    
    Py_RETURN_NONE;
}
/**
 * VardbEvb_setEvbServicePrefix
 *    Set the prefix of the advertised service for an existing event builder
 *    definition.
 *
 *  @param self - Object on which this method is being called.
 *  @param args - method positonal arguments : evbname, newPrefix
 *  @return Py_None
 */
static PyObject*
VardbEvb_setEvbServicePrefix(PyObject* self, PyObject* args)
{
    char* evb;
    char* prefix;
    
    if (!PyArg_ParseTuple(args, "ss", &evb, &prefix)) {
        return NULL;
    }
    
    CVardbEventBuilder* pApi = getApi(self);
    try {
        pApi->evbSetServicePrefix(evb, prefix);
    }
    catch (std::exception& e) {
        PyErr_SetString(exception, e.what());
        return NULL;
    }
    
    Py_RETURN_NONE;
}

/**
 * VardbEvb_disableEvbBuild
 *    Disable event building for a defined event builder.  This means that the
 *    event builder will only emit ordered event fragmnts (GRETIN GEB mode e.g).
 *
 *  @param self - Object on which this method is being called.
 *  @param args - method positonal arguments : evbname.
 *  @return Py_None
 */
static PyObject*
VardbEvb_disableEvbBuild(PyObject* self, PyObject* args)
{
    char* evb;
    
    if (!PyArg_ParseTuple(args, "s", &evb)) {
        return NULL;
    }
    
    CVardbEventBuilder* pApi = getApi(self);
    
    try {
        pApi->evbDisableBuild(evb);
    }
    catch(std::exception& e) {
        PyErr_SetString(exception, e.what());
        return NULL;
    }
    Py_RETURN_NONE;
}
/**
 * VardbEvb_enableEvbBuild
 *    Enable production of built events from an event builder.  If this is true,
 *    the output of an event builder pipeline will be events  built by aggregating
 *    (glomming) fragments that are within the coincidence interval of the first
 *    fragment of a run of fragments.  These events will be given a timestamp
 *    in accordance with the event builder's  timestamp policy and a source
 *    id in accordance with the event builder's source id.
 *
 *  @param self - Object on which this method is being called.
 *  @param args - method positonal arguments : evbname.
 *  @return Py_None
 */
static PyObject*
VardbEvb_enableEvbBuild(PyObject* self, PyObject* args)
{
    char* evb;
    
    if (!PyArg_ParseTuple(args, "s", &evb)) {
        return NULL;
    }
    
    CVardbEventBuilder* pApi = getApi(self);
    try {
        pApi->evbEnableBuild(evb);
    }
    catch (std::exception& e) {
        PyErr_SetString(exception, e.what());
        return NULL;
    }
    Py_RETURN_NONE;
}
/**
 * VardbEvb_setEvbTimestampPolicy
 *    Set a new timestamp tagging policy for an existing event builder. This
 *    value determines how the event builder computes timestamps for output
 *    events when in build mode.
 *  @param self - Object on which this method is being called.
 *  @param args - method positonal arguments : evbname, policy
 *  @return Py_None
 */
static PyObject*
VardbEvb_setEvbTimestampPolicy(PyObject* self, PyObject* args)
{
    char* evb;
    char* strPolicy;
    CVardbEventBuilder::TimestampPolicy policy;
    
    if (!PyArg_ParseTuple(args, "ss", &evb, &strPolicy)) {
        return NULL;
    }
    CVardbEventBuilder* pApi = getApi(self);
    
    
    try {
        policy = strToTsPolicy(strPolicy);
        pApi->evbSetTimestampPolicy(evb, policy);
    }
    catch (std::exception& e) {
        PyErr_SetString(exception, e.what());
        return NULL;
    }
    Py_RETURN_NONE;
}
/**
 * VardbEvb_setEvbServiceSuffix
 *    Set a new service name suffix for the service that is advertised by a
 *    previously defined event builder.
 *
 *  @param self - Object on which this method is being called.
 *  @param args - method positonal arguments : evbname, service-Suffix.
 *  @return Py_None
*/
static PyObject*
VardbEvb_setEvbServiceSuffix(PyObject* self, PyObject* args)
{
    char* evb;
    char* suffix;
    
    if (!PyArg_ParseTuple(args, "ss", &evb, &suffix)) {
        return NULL;
    }
    
    CVardbEventBuilder* pApi = getApi(self);
    try {
        pApi->evbSetServiceSuffix(evb, suffix);
    }
    catch (std::exception& e) {
        PyErr_SetString(exception, e.what());
        return NULL;
    }
    Py_RETURN_NONE;
}
/**
 * VardbEvb_rmEventBuilder
 *    Remove an existing event builder definition.
 *
 *  @param self - Object on which this method is being called.
 *  @param args - method positonal arguments : evbname, service-Suffix.
 *  @return Py_None
 */
static PyObject*
VardbEvb_rmEventBuilder(PyObject* self, PyObject* args)
{
    char* evb;
    
    if(!PyArg_ParseTuple(args, "s", &evb)) {
        return NULL;
    }
    
    CVardbEventBuilder* pApi = getApi(self);
    
    try {
        pApi->rmEventBuilder(evb);
    }
    catch(std::exception& e) {
        PyErr_SetString(exception, e.what());
        return NULL;
    }
    Py_RETURN_NONE;
}

/**
 * VardbEvb_evbInfo
 *    Return information about an existing event builder.
 *
 *  @param self - PyObject representing the API object.
 *  @param args - Positional arguments, evbname
 *  @return PyObject* - dict with the following keys:
 *               - name - name of the event builder.  Matches the parameter.
 *               - host - name of host on which the event builder will be started.
 *               - coincidenceInterval - The number of ticks fragments must be
 *                        within the first fragment to be built into an event.
 *               - servicePrefix - Prefix of the service name under which the
 *                        event builder advertises for data source connections.
 *               - serviceSuffix - Suffix of the service name under which the
 *                        event builder advertises for data source connections.
 *               - build - Flag that is True if the event builder will emit
 *                         built events.  If not the event builder only emits
 *                         time ordered fragments.
 *               - sourceId - Source id placed in built events emitted by this
 *                          event builder.
 *               = timestampPolicy - Policy that describes how the timestamp
 *                          for built events is computed.  This can be one of
 *                          'earliest', 'latest', or 'average' with obvious
 *                          meanings.
 */
static PyObject*
VardbEvb_evbInfo(PyObject* self, PyObject* args)
{
    char* evb;
    
    if (!PyArg_ParseTuple(args, "s", &evb)) {
        return NULL;
    }
    
    CVardbEventBuilder*                pApi;
    CVardbEventBuilder::EvbDescription info;
    
    pApi = getApi(self);
    try {
        info = pApi->evbInfo(evb);
    }
    catch (std::exception& e) {
        PyErr_SetString(exception, e.what());
        return NULL;
    }
    
    PyObject* result = evbInfoToDict(info);
    
    return result;
}
/**
 * VardbEvb_listEventBuilders
 *    Return a list of dicts that provide information about all event
 *    builders.   See VadbEvb_evbInfo for the keys dicts in this list hold.
 *
 *  @param self  - PyObject* pointer to the object running this method.
 *  @param args  - Positional parameters (must be empty).
 *  @return PyObject* list of dicts.
 */
static PyObject*
VardbEvb_listEventBuilders(PyObject* self, PyObject* args)
{
    if (PyTuple_Size(args) > 0) {
        PyErr_SetString(exception, "listEventBuilders takes no parameters");
        return NULL;
    }
    
    CVardbEventBuilder*                              pApi = getApi(self);
    std::vector<CVardbEventBuilder::EvbDescription> vResult;
    try {
        vResult = pApi->listEventBuilders();
    }
    catch (std::exception& e) {
        PyErr_SetString(exception, e.what());
        return NULL;
    }
    
    PyObject* result = PyList_New(vResult.size());
    for (int i  = 0; i < vResult.size(); i++)  {
        PyList_SetItem(result, i, evbInfoToDict(vResult[i]));
    }
    return result;
}

/**
 *  VardbEvb_addDataSource
 *     Add a data source to an existing event builder.
 *     As with createEventbuilder this takes a set of mandatory, positional
 *     parameters and a set of keyword parameters that are all optional.
 *     The keyword parameters are:
 *     -  info  - an information string associated with the event builder
 *     -  bodyHeaders - a bool that indicates whether or not the data should
 *                all have body headers.
 *     -  defaultId - The source id to associate with a fragment if it has no body
 *                header.
 *     -  tsextractor - The the path to the shared library that will provide
 *                 timestamps for items that have no body headers.
 *
 *  @param self   - pointer to our data.
 *  @param args   - Tuple containing positional actual parameters.
 *  @param kwargs - Dict containing keyword parameters present.
 *  @return Py_NONE
 */

static PyObject*
VardbEvb_addDataSource(PyObject* self, PyObject* args, PyObject* kwargs)
{
    // We need to pull this stuff from the parameters - defaults are also provided
    // where appropriate.  Note that for some we need two sets of variables
    // as the stuff provided by PyArg_ParseTupleAndKeywords needs post processing.
    
    char*                 evbName(0);
    char*                 srcName(0);
    char*                 host(0);
    char*                 dsPath(0);
    char*                 ringUri(0);
    std::vector<unsigned> ids;
    PyObject*             oIds(0);                 // iterable of ids.
    const char*           info = "";
    bool                  bodyHeaders(true);
    PyObject*             oBodyHeader(0);         // Py Boolean of body header flag.
    unsigned              defaultId(0);
    const char*           timestampExtractor = "";
    
    // These are the keywords we recognize.  Note that we need to supply keywords
    // for the positional parameters a s well:
    
    static const char* keywords []  = {
        "evbname", "dsname", "host", "path", "ring", "ids", // positional params
        "info", "bodyHeaders", "defaultId", "tsextractor",  // kw params
        NULL  
    };
    
    // Process the parameters:
    // -   Parse them.
    // -   Perform any post procesing needed to marshall them into addDataSource
    //     parameters.
    
    if (!PyArg_ParseTupleAndKeywords(
        args, kwargs, "sssssO|sOIs",  const_cast<char**>(keywords),
        &evbName, &srcName, &host, &dsPath, &ringUri, &oIds,
        &info, &oBodyHeader, &defaultId, &timestampExtractor
    )) {
        return NULL;
    }
    // oids must be iterable -- containng integers that can be pulled into ids.
    
    try {
        ids = iterableToUnsignedVector(oIds);
    }
    catch (...) {
        return NULL;      //i...Vector raised the python exception too.
    }
    // oBody Headers must be bool - update bodyHeaders from it:
    
    if (oBodyHeader) {
        if (!PyBool_Check(oBodyHeader)) {
            PyErr_SetString(exception, "bodyHeaders must be a boolean");
            return NULL;
        }
        bodyHeaders = oBodyHeader == Py_True ? true : false;
    }
    
    // Now do the call:
    
    CVardbEventBuilder* pApi = getApi(self);
    try {
        pApi->addDataSource(
            evbName, srcName, host, dsPath, ringUri, ids, info, bodyHeaders,
            defaultId, timestampExtractor
        );
        
    }
    catch (std::exception& e) {
        PyErr_SetString(exception, e.what());
        return NULL;
    }
    Py_RETURN_NONE;
}

/**
 * VardbEvb_dsSetHost
 *    Set a data source host for an existing data source.
 *
 *  @param self - Object on which this method is being called.
 *  @param args - method positonal arguments : ebname, dsname, newhost
 *  @return Py_None
 */
static PyObject*
VardbEvb_dsSetHost(PyObject* self, PyObject* args)
{
    char* evb;
    char* ds;
    char* host;
    
    if (!PyArg_ParseTuple(args, "sss", &evb, &ds, &host)) {
        return NULL;
    }
    
    CVardbEventBuilder* pApi = getApi(self);
    try {
        pApi->dsSetHost(evb, ds, host);
    }
    catch (std::exception& e) {
        PyErr_SetString(exception, e.what());
        return NULL;
    }
    
    Py_RETURN_NONE;
}

/**
 * VardbEvb_dsSetPath
 *    Set a new program path for an existing data source.
 *
 *  @param self - Object on which this method is being called.
 *  @param args - method positonal arguments : evbname, dsName, newPath
 *  @return Py_None
 */
static PyObject*
VardbEvb_dsSetPath(PyObject* self, PyObject* args)
{
    char* evb;
    char* ds;
    char* path;
    
    if (!PyArg_ParseTuple(args, "sss", &evb, &ds, &path)) {
        return NULL;
    }
    
    CVardbEventBuilder* pApi = getApi(self);
    try {
        pApi->dsSetPath(evb, ds, path);
    }
    catch (std::exception& e) {
        PyErr_SetString(exception, e.what());
        return NULL;
    }
    
    Py_RETURN_NONE;
    
}
/**
 * VardbEvb_dsSetRingUri
 *   Change the value of the ringbuffer uri an existing data source
 *   uses as its source of fragments.
 *  @param self - Object on which this method is being called.
 *  @param args - method positonal arguments : evbname, dsName, newUri.
 *  @return Py_None
 */
static PyObject*
VardbEvb_dsSetRingUri(PyObject* self, PyObject* args)
{
    char* evb;
    char* ds;
    char* ring;
    
    if(!PyArg_ParseTuple(args, "sss", &evb, &ds, &ring)) {
        return NULL;
    }
    
    CVardbEventBuilder* pApi = getApi(self);
    try {
        pApi->dsSetRingUri(evb, ds, ring);
    }
    catch (std::exception& e) {
        PyErr_SetString(exception, e.what());
        return NULL;
    }
    
    Py_RETURN_NONE;
}
/**
 * VardbEvb_dsSetInfo
 *    Change the information string of a data source.
 *
 *  @param self - Object on which this method is being called.
 *  @param args - method positonal arguments : evbname, dsName, newInfo
 *  @return Py_None
 */
static PyObject*
VardbEvb_dsSetInfo(PyObject* self, PyObject* args)
{
    char* evb;
    char* ds;
    char* info;
    
    if (!PyArg_ParseTuple(args, "sss", &evb, &ds, &info)) {
        return NULL;
    }
    
    CVardbEventBuilder* pApi = getApi(self);
    try {
        pApi->dsSetInfo(evb, ds, info);
    }
    catch (std::exception& e) {
        PyErr_SetString(exception, e.what());
        return NULL;
    }
    Py_RETURN_NONE;
}
/**
 * VardbEvb_dsSetDefaultId
 *    Set a new default fragment id for fragments without a body header for
 *    a data source.
 *
 *  @param self - Object on which this method is being called.
 *  @param args - method positonal arguments : evbname, dsName, newId.
 *  @return Py_None
 */
static PyObject*
VardbEvb_dsSetDefaultId(PyObject* self, PyObject* args)
{
    char* evb;
    char* ds;
    unsigned id;
    
    if (!PyArg_ParseTuple(args, "ssI", &evb, &ds, &id)) {
        return NULL;
    }
    
    CVardbEventBuilder* pApi = getApi(self);
    
    try {
        pApi->dsSetDefaultId(evb, ds, id);
    }
    catch (std::exception& e) {
        PyErr_SetString(exception, e.what());
        return NULL;
    }
    Py_RETURN_NONE;
}

/**
 * VardbEvb_dsDontExpectBodyHeaders
 *     Turns of the expect-bodyheaders flag for a data source.  When this
 *     is false, a default-id and timestamp extractor must be provided
 *     to provide the source id and timestamp of items that don't come with a
 *     body header.
 *  @param self - Object on which this method is being called.
 *  @param args - method positonal arguments : evbname, dsName
 *  @return Py_None

 */
static PyObject*
VardbEvb_dsDontExpectBodyHeaders(PyObject* self, PyObject* args)
{
    char* evb;
    char* ds;
    
    if(!PyArg_ParseTuple(args, "ss", &evb, &ds)) {
        return NULL;
    }
    
    CVardbEventBuilder* pApi = getApi(self);
    try {
        pApi->dsDontExpectBodyHeaders(evb, ds);
        
    }
    catch(std::exception& e) {
        PyErr_SetString(exception, e.what());
        return NULL;
    }
    Py_RETURN_NONE;
}

/**
 * VardbEvb_dsExpectBodyHeaders
 *     Turns on the expect-bodyheaders flag for a data source.  When this is
 *     true, the data source assumes that all items have body headers and that
 *     the data source id and the timestamp can be gotten from the body header
 *     without knowning anything about the payload.
 *     
 *  @param self - Object on which this method is being called.
 *  @param args - method positonal arguments : evbname, dsName
 *  @return Py_None
 */
static PyObject*
VardbEvb_dsExpectBodyHeaders(PyObject* self, PyObject* args)
{
    char* evb;
    char* ds;
    
    if(!PyArg_ParseTuple(args, "ss", &evb, &ds)) {
        return NULL;
    }
    
    CVardbEventBuilder* pApi = getApi(self);
    try {
        pApi->dsExpectBodyHeaders(evb, ds);
        
    }
    catch(std::exception& e) {
        PyErr_SetString(exception, e.what());
        return NULL;
    }
    Py_RETURN_NONE;
}
/**
 * VardbEvb_dsSetTimestampExtractor
 *    Set a new timestamp extraction library for the data source.  This is used
 *    when
 *    -   An item does not have a body header.
 *    -   Body headers are not required.
 *    
 *    In that event functions within the library access the body of the event to
 *    extract a timestamp from it.
 *  @param self - Object on which this method is being called.
 *  @param args - method positonal arguments : evbname, dsName, extractorlib.
 *  @return Py_None
 *
 */
static PyObject*
VardbEvb_dsSetTimestampExtractor(PyObject* self, PyObject* args)
{
    char* evb;
    char* ds;
    char* tsLibName;
    
    if(!PyArg_ParseTuple(args, "sss", &evb, &ds, &tsLibName)) {
        return NULL;
    }
    
    CVardbEventBuilder* pApi = getApi(self);
    try {
        pApi->dsSetTimestampExtractor(evb, ds, tsLibName);
    }
    catch (std::exception& e) {
        PyErr_SetString(exception, e.what());
        return NULL;
    }
    
    Py_RETURN_NONE;
}
/**
 *  VardbEvb_dsSetIds
 *     Change the set of data source ids a data source may emit.
 *
 *  @param self - object onwhich this method is being invoked.
 *  @param args - method positional parameters, evbname, dsname, sources
 *  @return Py_None
 *  @note the sources can be any type that supports iteration.
 */
static PyObject*
VardbEvb_dsSetIds(PyObject* self, PyObject* args)
{
    char*                 evb;
    char*                 ds;
    PyObject*             idsObj;
    std::vector<unsigned> ids;
    
    if (!PyArg_ParseTuple(args, "ssO", &evb, &ds, &idsObj)) {
        return NULL;
    }
    
    CVardbEventBuilder* pApi = getApi(self);
    try {
        ids = iterableToUnsignedVector(idsObj);
        pApi->dsSetIds(evb, ds, ids);
    }
    catch(std::exception& e) {
        PyErr_SetString(exception, e.what());
        return NULL;
    }
    Py_RETURN_NONE;
}

/**
 * VardbEvb_dsInfo
 *    Returns a dict that describes a data source.  The dict has the
 *    following (text) keys:
 *    -   name  - Name of the data source.
 *    -   host  - Host running the data source.
 *    -   path  - Path to the program that is the data source.
 *    -   info  - Information string.
 *    -   ids   - Tuple of source ids the data source produces.
 *    -   ring  - URI of the ring from which data will be taken.
 *    -   bodyheaders - bool - True if body headers are required.
 *    -   defaultId - default data source id.
 *    -   textractor - path to timestamp extractor library file.
 *  @param self - Object on which this method is being called.
 *  @param args - method positonal arguments : evbname, dsName
 *  @return PyDict*
 *
 */
static PyObject*
VardbEvb_dsInfo(PyObject* self, PyObject* args)
{
    char* evb;
    char* ds;
    
    if(!PyArg_ParseTuple(args, "ss", &evb, &ds)) {
        return NULL;
    }
    
    CVardbEventBuilder*               pApi = getApi(self);
    CVardbEventBuilder::DsDescription info;
    
    try {
        info = pApi->dsInfo(evb, ds);
    }
    catch(std::exception& e) {
        PyErr_SetString(exception, e.what());
        return NULL;
    }
    
    return dsInfoToDict(info);
}

/**
 * VardbEvb_listDataSources
 *    List all of the data sources for an event builder.  The result
 *    is a tuple whose values are dicts of the sort returned by
 *    dsInfo.
 *
 *  @param self - Object on which this method is being called.
 *  @param args - method positonal arguments : evbname
 *  @return PyTuple* - of info dicts.
 */
static PyObject*
VardbEvb_listDataSources(PyObject* self, PyObject* args)
{
    char* evb;
    
    if (!PyArg_ParseTuple(args, "s", &evb)) {
        return NULL;
    }
    
    CVardbEventBuilder* pApi = getApi(self);
    std::vector<CVardbEventBuilder::DsDescription> vinfo;
    try {
        vinfo =  pApi->listDataSources(evb);
    }
    catch(std::exception& e) {
        PyErr_SetString(exception, e.what());
        return NULL;
    }
    
    PyObject* result = PyTuple_New(vinfo.size());
    for (int i = 0; i < vinfo.size(); i++) {
        PyTuple_SetItem(result, i, dsInfoToDict(vinfo[i]));
    }
    
    return result;
}

/** VardbEvb_rmDataSource
 *    Remove the definition of an existing data source.
 *
 *  @param self - Object on which this method is being called.
 *  @param args - method positonal arguments : evbname
 *  @return Py_NONE
*/
static PyObject*
VardbEvb_rmDataSource(PyObject* self, PyObject* args)
{
    char* evb;
    char* ds;
    
    if (!PyArg_ParseTuple(args, "ss", &evb, &ds)) {
        return NULL;
    }
    
    CVardbEventBuilder* pApi = getApi(self);
    try {
        pApi->rmDataSource(evb, ds);
    }
    catch (std::exception& e) {
        PyErr_SetString(exception, e.what());
        return NULL;
    }
    
    Py_RETURN_NONE;
}
/*---------------------------------------------------------------------------
 * Canonical methods for the VardbEvb class (instantiation/destruction).
 */

/**
 * VardbEvb_new
 *    Create the storage associated with a new VardbEvb object.
 *    Any default initialization is also done here.
 *
 *  @param type - pointer to our type data structure.
 *  @param args - Any positional parameters passed to us (We don't expect any).
 *  @param kargs - Any keyword parameters passed to us (we don't expect any).
 *  @return PyObject* Pointer to the newly created object data structure.
 */

static PyObject*
VardbEvb_new(PyTypeObject* type, PyObject* args, PyObject* kargs)
{
    PyObject* self = type->tp_alloc(type, 0);
    if (!self) {
        PyErr_SetString(exception, "Unable to allocate VarDbEvb object storage");
    } else {
        vardbEvb_Data* pThis = reinterpret_cast<vardbEvb_Data*>(self);
        pThis->m_pApi = 0;
    }
    
    return self;
    
}

/**
 * VardbEvb_init
 *    Our __init__ method for VardbEvb.VardbEvb objects.
 *    
 *   @param self - Pointer to our own data
 *   @param args - Positional parameters - we need one, the database URI.
 *   @param kargs - Keyword args, we don't use this.
 *   @return int - 0 success, -1 failure.
 */
static int
VardbEvb_init(PyObject* self, PyObject* args, PyObject* kargs)
{
    char* uri;
    if (!PyArg_ParseTuple(args, "s", &uri)) {
        PyErr_SetString(exception, "Construction requires a URI");
        return -1;
    }
    vardbEvb_Data* pThis = reinterpret_cast<vardbEvb_Data*>(self);
    try {
        pThis->m_pApi = new CVardbEventBuilder(uri);
    }
    catch(std::exception& e) {
        PyErr_SetString(exception, e.what());
        return -1;
    }
    
    
    return 0;
}
/**
 * VardbEvb_delete
 *     Our __del__ method
 *  @param self - pointer to our storage which wea are reponsible for
 *                releasing.
 */
static void
VardbEvb_delete(PyObject* self)
{
    vardbEvb_Data* pThis = reinterpret_cast<vardbEvb_Data*>(self);
    delete pThis->m_pApi;
    pThis->m_pApi = 0;
    self->ob_type->tp_free(self);
}

/*-----------------------------------------------------------------------------
 * Plumbing needed to define the module and its data types to python:
 */


/*
 *  Module level method definition table.
 *  The only thing that can be done at object level is to instantiate
 *  VardbEventBuilder objects.  Therefore this table is empty but needed
 *  by the initialization code.
 */

static PyMethodDef ModuleMethods[] = {
    {NULL, NULL, 0, NULL}                /* End of method definition marker */   
};

/**
 * object method dispatch table for VardbEvb.VardbEvb objects
 */

static PyMethodDef VarDbEvbMethods[] = {
     {"schemaExists", VardbEvb_schemaExists, METH_VARARGS,
        "Check for schema existence"},
    {"createSchema", VardbEvb_createSchema, METH_VARARGS,
        "Create the event builder variable datbase schema"},
    {"createEventBuilder",
    (PyCFunction)VardbEvb_createEventBuilder, METH_VARARGS |METH_KEYWORDS,
        "Create a new event builder definition"},
    {"setEvbHost", VardbEvb_setEvbHost, METH_VARARGS,
        "Change event builder host"},
    {"setEvbCoincidenceInterval", VardbEvb_setEvbCoincidenceInterval, METH_VARARGS,
        "Change coincidence interval for an eventbuilder"
    },
    {"setEvbRing", VardbEvb_setEvbRing, METH_VARARGS, "Change output ring buffer"}, 
    {"setEvbSourceId", VardbEvb_setEvbSourceId, METH_VARARGS,
        "Change the output source id of a defined event builder."
    },
    {"setEvbServicePrefix", VardbEvb_setEvbServicePrefix, METH_VARARGS,
        "Change the advertised service prefix for a defined event builder"
    },
    {"disableEvbBuild", VardbEvb_disableEvbBuild, METH_VARARGS,
        "Disable production of built events for a defined event builder"
    },
    {"enableEvbBuild", VardbEvb_enableEvbBuild, METH_VARARGS,
        "Enable production of built events  for a defined event builder"
    },
    {"setEvbTimestampPolicy", VardbEvb_setEvbTimestampPolicy, METH_VARARGS,
        "Set a new timestamp policy for an existing event builder"
    },
    {"setEvbServiceSuffix", VardbEvb_setEvbServiceSuffix, METH_VARARGS,
        "Set a new suffix for the advertised service of an existing event builder"
    },
    {"rmEventBuilder", VardbEvb_rmEventBuilder, METH_VARARGS,
        "Remove/destroy an existing event builder definition."
    },
    {"evbInfo", VardbEvb_evbInfo, METH_VARARGS,
        "Get information about an existing event builder definition"
    },
    {"listEventBuilders", VardbEvb_listEventBuilders, METH_VARARGS,
        "Get information about all defined event builders"
    },
    {"addDataSource", (PyCFunction)VardbEvb_addDataSource,
        METH_VARARGS | METH_KEYWORDS,
        "Add a new data source to an event builder"
    },
    {"dsSetHost", VardbEvb_dsSetHost, METH_VARARGS,
        "Set the host for an existingt data source"
    },
    {"dsSetPath", VardbEvb_dsSetPath, METH_VARARGS,
        "Set new path for the program that is a data source"
    },
    {"dsSetRingUri", VardbEvb_dsSetRingUri, METH_VARARGS,
        "Set a new input ring uri for a data source"
    },
    {"dsSetInfo", VardbEvb_dsSetInfo, METH_VARARGS,
        "Change the information string of a data source"
    },
    {"dsSetDefaultId", VardbEvb_dsSetDefaultId, METH_VARARGS,
        "Change the default id of a data source"
    },
    {"dsDontExpectBodyHeaders", VardbEvb_dsDontExpectBodyHeaders, METH_VARARGS,
        "Turn off expectation of body headers."
    },
    {"dsExpectBodyHeaders", VardbEvb_dsExpectBodyHeaders, METH_VARARGS,
        "Turn off expectation of body headers."
    },
    {"dsSetTimestampExtractor", VardbEvb_dsSetTimestampExtractor, METH_VARARGS,
        "Set a new timestamp extraction library"
    },
    {"dsSetIds", VardbEvb_dsSetIds, METH_VARARGS, "Set data source ids"},
    {"dsInfo", VardbEvb_dsInfo, METH_VARARGS, "Get dict describing a source"},
    {"listDataSources", VardbEvb_listDataSources, METH_VARARGS,
        "Describe all of an event builder's data sources."
    },
    {"rmDataSource", VardbEvb_rmDataSource, METH_VARARGS,
        "Destroy a data source definition"
    },
    {NULL, NULL, 0, NULL}                /* End of method definition marker */   
    
};

/**
 *  Type description table for the VardbEvb.VardbEvb type:
 */
static PyTypeObject vardb_VarDbEvbType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "VardbEvb.VardbEvb",       /*tp_name*/
    sizeof(vardbEvb_Data), /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)(VardbEvb_delete), /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    0,                         /*tp_repr*/
    0,                         /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    0,                         /*tp_as_mapping*/
    0,                         /*tp_hash */
    0,                         /*tp_call*/
    0,                         /*tp_str*/
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,        /*tp_flags*/
    "Event builder variable database api objects", /* tp_doc */
    0,		               /* tp_traverse */
    0,		               /* tp_clear */
    0,		               /* tp_richcompare */
    0,		               /* tp_weaklistoffset */
    0,		               /* tp_iter */
    0,		               /* tp_iternext */
    VarDbEvbMethods,           /* tp_methods */
    0,                         /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)VardbEvb_init,      /* tp_init */
    0,                         /* tp_alloc */
    VardbEvb_new,                 /* tp_new */
};




/**
 * Initialize the module, and register all types we contain:
 */

PyMODINIT_FUNC
initVardbEvb(void)
{
    PyObject* module;
    
    // Instantiate register our module.
    
    module = Py_InitModule3(
        "VardbEvb", ModuleMethods,
        "Python bindings to database description of event builders"
    );
    if (module == NULL) {
        return;                // Failed...return begfore doing anything else.
    }
    // Instantiate/register our data type/class.
    
    if (PyType_Ready(&vardb_VarDbEvbType) < 0) {
        return;
    }
    Py_INCREF(&vardb_VarDbEvbType);
    PyModule_AddObject(
        module, "VardbEvb", reinterpret_cast<PyObject*>(&vardb_VarDbEvbType)
    );
    // Create/Register our exception as well:
    
    exception = PyErr_NewException(
        const_cast<char*>("VarDbEvb.exception"), NULL, NULL
    );
    Py_INCREF(exception);
    PyModule_AddObject(module, "exception", exception);
}