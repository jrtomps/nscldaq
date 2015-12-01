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
# @file   pyServices.cpp
# @brief  Python bindings to the services  api.
# @author <fox@nscl.msu.edu>
*/

#include <Python.h>
#include "CServiceApi.h"
#include <stdexcept>

static PyObject* exception;

/* API object storage definition */

typedef struct {
    PyObject_HEAD
    
    CServiceApi*   m_pApi;
} ApiObject, *pApiObject;

/*---------------------------------------------------------------------------
 * Utility methods.
 */

/**
 * getApi
 *    Given a pointer to an object that says it's a service api
 *    return the m_pApi pointer.
 */
static CServiceApi*
getApi(PyObject* self)
{
    pApiObject pApi = reinterpret_cast<pApiObject>(self);
    
    return pApi->m_pApi;
}


/*----------------------------------------------------------------------------
 * Object canonical methods
 */
/**
 * _new
 *    Do storage allocation for a new API object.
 *
 * @param type  Pointer to object type struct.
 * @param args  Positional parameters.
 * @param kwargs Kewywords argument dit./
 *
 * @return PyObject* - pointer to the created object.
 * @retval NULL      - If not able to create.
 */
static PyObject*
_new(PyTypeObject* type, PyObject* args, PyObject* kwrgs)
{
    pApiObject self = reinterpret_cast<pApiObject>(type->tp_alloc(type, 0));
    if (self) {
        self->m_pApi = NULL;
        return reinterpret_cast<PyObject*>(self);
    } else {
        PyErr_SetString(exception, "Could not allocate an api object");
        return NULL;
    }
}

/**
 * _init
 *    Performs initialization of an allocated API object.
 *
 * @param self - Pointer to object.
 * @param args - Positional parameters (URI string).
 * @param kwargs - Keyword arguments.
 * @return   int -1 failure 0 success.
 */
static int
_init(PyObject* self, PyObject* args, PyObject* kwds)
{
    const char* pUri;
    
    if (!PyArg_ParseTuple(args, "s", &pUri)) {
        PyErr_SetString(exception, "need db uri");
        return -1;
    }
    
    pApiObject pApi = reinterpret_cast<pApiObject>(self);
    delete     pApi->m_pApi;
    try {
        pApi->m_pApi    = new CServiceApi(pUri);
    }
    catch (std::exception& e) {
        PyErr_SetString(exception, e.what());
        return -1;
    }
    return 0;
}

/**
 * _delete
 *    Get rid of resources associated with an API object
 *
 *  @param self - Pointer to the object that we are getting rid of.
 */
static void
_delete(PyObject* self)
{
    pApiObject pApi = reinterpret_cast<pApiObject>(self);
    delete pApi->m_pApi;
    self->ob_type->tp_free(self);
}

/*--------------------------------------------------------------------------
 * Object functional methods
 */

/**
 * exists
 *    Check that the schema exists.
 *
 *  @param self - Pointer to us.
 *  @param args - Pointer to args tuple (should be empty).
 *  @return PyObject* - boolean result of the query.
 */
static PyObject*
exists(PyObject* self, PyObject* args)
{
    /* args must be empty */
    
    if (PyTuple_Size(args) > 0) {
        PyErr_SetString(exception, "exists - takes no parameters");
        return 0;
    }
    
    /* Get the Api pointer */
    
    CServiceApi* pApi = getApi(self);
    if(!pApi) {
        return 0;                      /* Caller set the exception string */
    }
    
    bool result;
    try {
        result = pApi->exists();
    }
    catch (std::exception& e) {
        PyErr_SetString(exception, e.what());
        return 0;
    }
    
    /* Return the right pyobject: */
    
    if(result) {
        Py_RETURN_TRUE;
    } else {
        Py_RETURN_FALSE;
    }
}

/**
 * create
 *    Create the schema
 *
 *   @param self - Pointer to the api object.
 *   @param args - Positional parameters (empty).
 *   @return PyNone
 */
static PyObject*
create(PyObject* self, PyObject* args)
{
    /* args must be empty */
    
    if (PyTuple_Size(args) > 0) {
        PyErr_SetString(exception, "create - takes no parameters");
        return 0;
    }
    
    /* Get the Api pointer */
    
    CServiceApi* pApi = getApi(self);
    if(!pApi) {
        return 0;                      /* Caller set the exception string */
    }
    try {
       pApi->create();
    }
    catch (std::exception& e) {
        PyErr_SetString(exception, e.what());
        return 0;
    }
    Py_RETURN_NONE;
}

/**
 * createProgram
 *     Create a new program definition.
 *
 *  @param self - Pointer to an Api objet.
 *  @param args - 3tuple containing the program name, path and host.
 *  @return PyNone
 */
static PyObject*
createProgram(PyObject* self, PyObject* args)
{
    char* name;
    char* path;
    char* host;
    
    if(!PyArg_ParseTuple(args, "sss", &name, &path, &host)) {
        PyErr_SetString(exception, "createProgram requires name, path and host");
        return 0;
    }
    
    CServiceApi* pApi = getApi(self);
    if(!pApi) {
        return 0;                      /* Caller set the exception string */
    }
    try {
        pApi->create(name, path, host);
    }
    catch (std::exception& e) {
        PyErr_SetString(exception, e.what());
        return 0;
    }
    Py_RETURN_NONE;
}
/**
 * setHost
 *    Changes the host name for a program.
 *
 *  @param self - API to use.
 *  @param args - 2-tuple with the program name and new host name.
 *  @return PyNone
 */
static PyObject*
setHost(PyObject* self, PyObject* args)
{
    char* name;
    char* host;
    
    if (!PyArg_ParseTuple(args, "ss", &name, &host)) {
        PyErr_SetString(exception, "setHost needs program name and host");
        return 0;
    }
    
    CServiceApi* pApi = getApi(self);
    if(!pApi) {
        return 0;                      /* Caller set the exception string */
    }
    try {
        pApi->setHost(name, host);
    }
    catch (std::exception& e) {
        PyErr_SetString(exception, e.what());
    }
    
    Py_RETURN_NONE;
}
/**
 * setCommand
 *    sets a new program path for a service.
 *
 *   @param self - Pointer to api.
 *   @param args - 2-tuple containing name and new path.
 *   @return PyNone
 */
static PyObject*
setCommand(PyObject* self, PyObject* args)
{
    char* name;
    char* path;
    
    if(!PyArg_ParseTuple(args, "ss", &name, &path)) {
        PyErr_SetString(exception, "setProgram needs name and path");
        return 0;
    }
    CServiceApi* pApi = getApi(self);
    if(!pApi) {
        return 0;                      /* Caller set the exception string */
    }
    try {
        pApi->setCommand(name, path);
    }
    catch (std::exception& e) {
        PyErr_SetString(exception, e.what());
    }
    
    
    Py_RETURN_NONE;
}

/**
 * remove
 *    Remove a program definition,.
 *
 *   @param self - Pointer to api.
 *   @param args - 1-tuple containing name.
 *   @return PyNone
 */
static PyObject*
remove(PyObject* self, PyObject* args)
{
    char* name;
    if(!PyArg_ParseTuple(args, "s", &name)) {
        PyErr_SetString(exception, "remove requires a program name");
        return 0;
    }
    CServiceApi* pApi = getApi(self);
    if(!pApi) {
        return 0;                      /* Caller set the exception string */
    }
    try {
        pApi->remove(name);
    }
    catch (std::exception& e) {
        PyErr_SetString(exception, e.what());
    }
    Py_RETURN_NONE;
    
}

/**
 * list
 *   produce a list of programs.
 *
 *   @param self - Pointer to the api object.
 *   @param args - empty ntuple
 *   @return PyObject*  This will be a dict whose keys are program names and
 *                      whose values are 2-tuples; [command, host]
*/
static PyObject* list(PyObject* self, PyObject* args)
{
    if (PyTuple_Size(args) > 0) {
        PyErr_SetString(exception, "list takes no parameters");
        return 0;
    }
    
    CServiceApi* pApi = getApi(self);
    if(!pApi) {
        return 0;                      /* Caller set the exception string */
    }
    std::map<std::string, std::pair<std::string, std::string> > apiResult;
    try {
        apiResult = pApi->list();
    }
    catch (std::exception& e) {
        PyErr_SetString(exception, e.what());
        return 0;
    }
    
    /* Marshall the api Result to the PyObject result */
    
    PyObject* pResult = PyDict_New();
    
    std::map<std::string, std::pair<std::string, std::string> >::iterator p =
        apiResult.begin();
    while(p != apiResult.end()) {
        std::string key = p->first;
        std::string path    = p->second.first;
        std::string host    = p->second.second;
        
        PyObject* pData = PyTuple_New(2);
        PyTuple_SET_ITEM(pData, 0, PyString_FromString(path.c_str()));
        PyTuple_SET_ITEM(pData, 1, PyString_FromString(host.c_str()));
        
        PyDict_SetItemString(pResult, key.c_str(), pData);
        
        p++;
    }

    return pResult;
}
/**
 * listProgram
 *    List information about a program (command and host).
 *
 *  @param self - pointer to the API python object.
 *  @param args - positional args tuple (name of program).
 *  @return PyObject* - 2-tuple with command path and host.
 */
static PyObject*
listProgram(PyObject* self, PyObject* args)
{
    char* name;
    if (! PyArg_ParseTuple(args, "s", &name)) {
        PyErr_SetString(exception, "listProgram needs a program name");
        return 0;
    }
    CServiceApi* pApi = getApi(self);
    if(!pApi) {
        return 0;                      /* Caller set the exception string */
    }
    std::pair<std::string, std::string> result;
    try {
        result = pApi->list(name);
    }
    catch (std::exception& e) {
        PyErr_SetString(exception, e.what());
        return 0;
    }
    
    PyObject* retval = PyTuple_Pack(
        2,
        PyString_FromString(result.first.c_str()),
        PyString_FromString(result.second.c_str())
    );
    return retval;
    
}
/* Method dispatch table for the API */

static PyMethodDef ApiObjectMethods[] =
{
    {"exists", exists, METH_VARARGS, "Schema exists"},
    {"create", create, METH_VARARGS, "Create schema"},
    {"createProgram", createProgram, METH_VARARGS, "Create new server program"},
    {"setHost", setHost, METH_VARARGS, "Change a program's host name"},
    {"setCommand", setCommand, METH_VARARGS, "Changes a programs runnable"},
    {"remove",     remove,     METH_VARARGS, "Remove a program"},
    {"list",       list,       METH_VARARGS, "List all programs"},
    {"listProgram", listProgram, METH_VARARGS, "List a program"},
    {NULL, NULL, 0, NULL}           // End of table sentinel.
};


/**
 * Type table for the ApI object:
 */
/* Api type descriptions */

static PyTypeObject ApiType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "stateclient.Api",             /*tp_name*/
    sizeof(ApiObject), /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)(_delete), /*tp_dealloc*/
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
    "services API wrapper objects", /* tp_doc */
    0,		               /* tp_traverse */
    0,		               /* tp_clear */
    0,		               /* tp_richcompare */
    0,		               /* tp_weaklistoffset */
    0,		               /* tp_iter */
    0,		               /* tp_iternext */
    ApiObjectMethods,        /* tp_methods */
    0,                         /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)_init,      /* tp_init */
    0,                         /* tp_alloc */
    _new,                 /* tp_new */
};

/* Module dispatch table: */

static PyMethodDef ServicesClassMethods[] = {
    {NULL, NULL, 0, NULL}           // End of table sentinell.
};

/**
 *  initservices
 *     Initialize the class.
 */
PyMODINIT_FUNC
initservices(void)
{
    PyObject* module;
    
    module = Py_InitModule3(
        "services", ServicesClassMethods, "CServicesApi encapsulation"
    );
    
    if (module ==  NULL) {
        return;
    }
    
    /* Create our own exception */
    
    exception = PyErr_NewException(
        const_cast<char*>("services.error"), NULL, NULL
    );
    Py_INCREF(exception);
    PyModule_AddObject(module, "error", exception);
    
    /* Register our type */
    
    if (PyType_Ready(&ApiType) < 0) {
        return;
    }
    Py_INCREF(&ApiType);
    PyModule_AddObject(
        module, "Api", reinterpret_cast<PyObject*>(&ApiType)
    );
}
