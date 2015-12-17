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

static CVardbEventBuilder* getApi(PyObject* self)
{
    vardbEvb_Data* pThis = reinterpret_cast<vardbEvb_Data*>(self);
    return pThis->m_pApi;
}

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
        "name", "host", "dt",                  // Positional actually.
        "sourceId", "servicePrefix", "build", "tsPolicy", "serviceSuffix",
        NULL
    };
    // Parameters and their defaults:

    //      Positional params:
    
    char* name;
    char* host;
    unsigned dt;
    
    // keyword params need defaults:
    
    int sourceId(0);
    const char* servicePrefix = "ORDERER";
    bool        build         = true;
    PyObject*   oBuild        = Py_True;
    const char* tsPolicy      = "earliest";
    const char* serviceSuffix = "";
    
    // Process the parameters.
    
    if (! PyArg_ParseTupleAndKeywords(
        args, kwArgs, "ssi|isOss", const_cast<char**>(keywords),
        &name, &host, &dt, &sourceId, &servicePrefix,
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
            name, host, dt, sourceId, servicePrefix, (build != 0),
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
 * setEvbCoincidenceInterval
 *    Set a new coincidence interval for event building in an event builder.
 *
 *  @param self - Object on which this method is being called.
 *  @param args - method positonal arguments : evbname, newInterval
 *  @return Py_None
 */
static PyObject*
setEvbCoincidenceInterval(PyObject* self, PyObject* args)
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
    {"setEvbCoincidenceInterval", setEvbCoincidenceInterval, METH_VARARGS,
        "Change coincidence interval for an eventbuilder"
    },
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
    Py_TPFLAGS_DEFAULT,        /*tp_flags*/
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