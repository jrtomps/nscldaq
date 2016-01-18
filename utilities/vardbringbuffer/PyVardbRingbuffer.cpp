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
# @file   PyVardbRingBuffer.cpp
# @brief  Python bindings to CVardbRingBuffer class.
# @author <fox@nscl.msu.edu>
*/

#include <Python.h>
#include "CVardbRingBuffer.h"

#include <exception>

/**
 * Instance data struct:
 */

typedef struct _vardbRingbuffer_Data {
    PyObject_HEAD;
    CVardbRingBuffer* m_pApi;
} vardbRingBuffer_Data, *pvardbRingBuffer_Data;


PyObject* exception;                // Module specific exception.


/*----------------------------------------------------------------------------
 * Useful local utilities.
 */

/**
 * getApi
 *    Return the C++ api object from our object.
 *  @param self - Pointer to us.
 *  @return CVardbRingBuffer*
 */
static CVardbRingBuffer*
getApi(PyObject* self)
{
    pvardbRingBuffer_Data pThis = reinterpret_cast<pvardbRingBuffer_Data>(self);
    return pThis->m_pApi;

}
/**
 * setDictValue
 *    Set the value of a dict key
 *
 *  @param dict  - Dict object pointer.
 *  @param key   - keyword string.
 *  @param value - Value to sets.
 *
 *   @note there are two overloads.  The first  has the value an unsigned while
 *   the second has it an std::string.
 */
static void
setDictValue(PyObject* dict, const char* key, unsigned value)
{
    PyObject* valueObj = PyLong_FromUnsignedLong(value);
    PyDict_SetItemString(dict, key, valueObj);
}
static void
setDictValue(PyObject* dict, const char* key, std::string value)
{
    PyObject* valueObj = PyString_FromString(value.c_str());
    PyDict_SetItemString(dict, key, valueObj);
}

/**
 *  Convert a ring info struct to a python dict.  See _ringInfo for information
 *  about the dict keys.
 */
static PyObject*
ringInfoToDict(const CVardbRingBuffer::RingInfo& info)
{
    PyObject* result = PyDict_New();
    setDictValue(result, "name", info.s_name);
    setDictValue(result, "host", info.s_host);
    setDictValue(result, "datasize", info.s_dataSize);
    setDictValue(result, "maxconsumers", info.s_maxConsumers);
    
    return result;
}
/**
 *----------------------------------------------------------------------------
 *  canonical methods for the api object
 */

/**
 * vardbRingbuffer_new
 *    Allocate storage for the object.  In this case we allocate and initialize
 *    a vardbRingBuffer_Data struct and set the m_pApi element to 0.
 *
 *  @param type - pointer to the type data struct.
 *  @param args - any positional parameters to the new operation.
 *  @param kargs - any keywords parameters to the new operation.
 *  @return PyObject* - pointer to the newly created object data struct.
 */
static PyObject*
vardbRingbuffer_new(PyTypeObject* type, PyObject* args, PyObject* kargs)
{
    PyObject* self = type->tp_alloc(type, 0);
    
    if (!self) {
        PyErr_SetString(exception, "Unable to allocated api object");
    } else {
        pvardbRingBuffer_Data pThis =
            reinterpret_cast<pvardbRingBuffer_Data>(self);
        pThis->m_pApi = 0;
    }
    return self;
}
/**
 * vardbRingbuffer_init
 *    __init__ for an api object. our only function is to allocate
 *    the API Object using the URI supplied.
 *
 * @param self   - Pointer to our data.
 * @param args   - Positional args (URI string).
 * @param kargs  - keywords args.
 * @return int 0 success, -1 failure.
 */
static int
vardbRingbuffer_init(PyObject* self, PyObject* args, PyObject* kargs)
{
    const char* uri;
    pvardbRingBuffer_Data pThis = reinterpret_cast<pvardbRingBuffer_Data>(self);
    
    if (!PyArg_ParseTuple(args, "s", &uri)) {
        PyErr_SetString(exception, "Construction requires a URI");
        return -1;
    }
    
    try {
        pThis->m_pApi = new CVardbRingBuffer(uri);
    }
    catch (std::exception& e) {
        PyErr_SetString(exception, e.what());
        return -1;
    }
    
    return 0;
}
/**
 * vardbRingbuffer_delete
 *    Delete storage associated with an api object.
 *
 * @param self - pointer to object storage.
 */
static void
vardbRingbuffer_delete(PyObject* self)
{
    pvardbRingBuffer_Data pThis = reinterpret_cast<pvardbRingBuffer_Data>(self);
    delete pThis->m_pApi;
    pThis->m_pApi = 0;
    self->ob_type->tp_free(self);
}

/*
 *------------------------------------------------------------------------------
 *  object methods for an api object.
 */

/**
 *  _schemaExists
 *     Tests for existence of the schema in the database.
 * @param self - Pointer to this object.
 * @param args - Parameter to position sensitive args (must be none).
 * @return PyBool type - result of schemaExists in the API.
 */
static PyObject*
_schemaExists(PyObject* self, PyObject* args)
{
    // Ensure there are no args:
    
    if (PyTuple_Size(args) != 0) {
        PyErr_SetString(exception,"No args for checkSchema");
        return 0;
    }
    CVardbRingBuffer* pRb = getApi(self);
    bool result;
    try {
        result = pRb->haveSchema();
    }
    catch (std::exception& e) {
        PyErr_SetString(exception, e.what());
    }
    
    if (result) {
        Py_RETURN_TRUE;
    } else {
        Py_RETURN_FALSE;
    }
}

/**
 * _createSchema
 *    Create the schema needed to represent ring buffers in the variable database.
 *
 *  @param self - Pointer to object data.
 *  @param args - Positional args.
 *  @return Py_None
*/
static PyObject*
_createSchema(PyObject* self, PyObject* args)
{
    if(PyTuple_Size(args) != 0) {
        PyErr_SetString(exception, "No parametesr for createSchema method");
        return 0;
    }
    CVardbRingBuffer* pApi = getApi(self);
    
    try {
        pApi->createSchema();
    }
    catch (std::exception& e) {
        PyErr_SetString(exception, e.what());
        return 0;
    }
    Py_RETURN_NONE;
}
/**
 * _create
 *    Create a new ring buffer definition.
 *
 * @param self - pointer to the data for the object invoked on.
 * @param args - Positional args - these are ring name and host.
 * @param kargs - Keyword params - these are datasize, maxconsumers.
 * @return Py_None
 */
static PyObject*
_create(PyObject* self, PyObject* args, PyObject* kargs)
{
    char*  name;
    char*  host;
    unsigned dsize        = 8*1024*1024;
    unsigned maxConsumers = 100;
    
    static const char* keywords[] = {
        "name", "host", "datasize", "maxconsumers", 0
    };
    
    if (!PyArg_ParseTupleAndKeywords(
        args, kargs, "ss|II", const_cast<char**>(keywords), &name, &host, &dsize, &maxConsumers
    )) {
        return NULL;
    }
    
    CVardbRingBuffer* pApi = getApi(self);
    try {
        pApi->create(name, host, dsize, maxConsumers);
    }
    catch (std::exception& e) {
        PyErr_SetString(exception, e.what());
        return NULL;
    }
    
    Py_RETURN_NONE;
}

/**
 * _destroy
 *    Destroy a ring buffer definition.
 *
 * @param self - pointer to the data of the object this is invoked on.
 * @param args - position args (name, host)
 * @return Py_None
 */
static PyObject*
_destroy(PyObject* self, PyObject* args)
{
    char* name;
    char* host;
    
    if (!PyArg_ParseTuple(args, "ss", &name, &host) ) {
        return NULL;
    }
    
    CVardbRingBuffer* pApi = getApi(self);
    
    try {
        pApi->destroy(name, host);
    }
    catch(std::exception& e) {
        PyErr_SetString(exception, e.what());
        return NULL;
    }
    Py_RETURN_NONE;
}
/**
 * _setMaxData
 *    Set the data size for an existing ring.
 *
 * @param self - pointer to the data of the object making the call.
 * @param args - Positional args: ringname, host
 * @return Py_None
 */
static PyObject*
_setMaxData(PyObject* self, PyObject* args)
{
    char* name;
    char* host;
    unsigned newSize;
    
    if (!PyArg_ParseTuple(args, "ssI", &name, &host, &newSize)) {
        return NULL;
    }
    
    CVardbRingBuffer* pApi = getApi(self);
    
    try {
        pApi->setMaxData(name, host, newSize);
    }
    catch(std::exception& e) {
        PyErr_SetString(exception, e.what());
        return NULL;
    }
    Py_RETURN_NONE;
}

/**
 * _setMaxConsumers
 *    Set a new value for the maximum number of simlutaneous consumers for
 *    an existing ring definition.
 *
 *  @param self - Poiner to the object data of an API.
 *  @param args - positional parameters, ring, host, new value.
 *  @return Py_None
 */
static PyObject*
_setMaxConsumers(PyObject* self, PyObject* args)
{
    char* name;
    char* host;
    unsigned newMax;
    
    if (!PyArg_ParseTuple(args, "ssI", &name, &host, &newMax)) {
        return NULL;
    }
    CVardbRingBuffer* pApi = getApi(self);
    
    try {
        pApi->setMaxConsumers(name, host, newMax);
    }
    catch(std::exception& e) {
        PyErr_SetString(exception, e.what());
        return NULL;
    }
    Py_RETURN_NONE;
}
/**
* _ringInfo
*    Returns information about a ringbuffer.
*  @param self - Pointer to our data.
*  @param args - Positional args -- name, host
*  @return PyDict* - dict with info about the ring:
*                  * 'name' - name of the ring.
*                  * 'host' - Host the ring runs in.
*                  *  'datasize' - Bytes of buffering in the ring.
*                  *  'maxconsumers' - Max number of simlutaneously connected
*                                      consumers.
*/
static PyObject*
_ringInfo(PyObject* self, PyObject* args)
{
    char* name;
    char* host;
    
    if (!PyArg_ParseTuple(args, "ss", &name, &host)) {
        return NULL;
    }
    
    CVardbRingBuffer* pApi = getApi(self);
    CVardbRingBuffer::RingInfo result;
    
    try {
        result = pApi->ringInfo(name, host);
    }
    catch(std::exception& e) {
        PyErr_SetString(exception, e.what());
        return NULL;
    }
    return ringInfoToDict(result);
    
}
/**
 * _list
 *    Provide dicts for all rings as an ntuple
 * @param self - pointer to our data.
 * @param args - positional args tuple (should be empty).
 * @return PyTupleObject* - pointer to a tuple of ringInfo dicts.
 */
static PyObject*
_list(PyObject* self, PyObject* args)
{
    if (PyTuple_Size(args) != 0) {
        PyErr_SetString(exception, "list method has no parameters");
        return NULL;
    }
    
    CVardbRingBuffer* pApi  = getApi(self);
    std::vector<CVardbRingBuffer::RingInfo> info;
    try {
        info = pApi->list();
    }
    catch(std::exception& e) {
        PyErr_SetString(exception, e.what());
        return NULL;
    }
    
    PyObject* result = PyTuple_New(info.size());
    for (int i = 0; i < info.size(); i++) {
        PyTuple_SetItem(result, i, ringInfoToDict(info[i]));
    }
    return result;
}
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
 * object method dispatch table for vardbRingbuffer.vardbRingbuffer objects
 */

static PyMethodDef  apiMethods[] = {
    {"haveSchema", _schemaExists, METH_VARARGS, "Check for schema existence"},
    {"createSchema", _createSchema, METH_VARARGS, "Create ring buffer schema"},
    {
        "create", (PyCFunction)_create, METH_VARARGS | METH_KEYWORDS,
        "Create a new ringbuffer definition"
    },
    {"destroy", _destroy, METH_VARARGS, "Destroy a ring buffer definition"},
    {"setMaxData", _setMaxData, METH_VARARGS, "Change ring data size"},
    {
        "setMaxConsumers", _setMaxConsumers, METH_VARARGS,
        "Change max # of consumers"
    },
    {"ringInfo", _ringInfo, METH_VARARGS, "Get information about a ring"},
    {"list", _list, METH_VARARGS, "List information about all rings"},
    
    {NULL, NULL, 0, NULL}                /* End of method definition marker */   
};



/**
 *  Type description table for the vardbRingbuffer.api type:
 */
static PyTypeObject vardb_vardbRingbufferType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "vardbRingbuffer.api",       /*tp_name*/
    sizeof(vardbRingBuffer_Data), /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)(vardbRingbuffer_delete), /*tp_dealloc*/
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
    apiMethods,           /* tp_methods */
    0,                         /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)vardbRingbuffer_init,      /* tp_init */
    0,                         /* tp_alloc */
    vardbRingbuffer_new,                 /* tp_new */
};





/**
 *  - Initialize the module and establish it for import
 */

PyMODINIT_FUNC
initVardbRingbuffer(void)
{
    PyObject* module;
    
    // Instantiate and register the module:
    
    module = Py_InitModule3(
        "VardbRingbuffer", ModuleMethods,
        "Python bindings to CVardbRingBuffer class."
    );
    if (module == NULL) {
        return;
    }
    // Register our api type.
    
    if (PyType_Ready(&vardb_vardbRingbufferType) < 0) {
        return;
    }
    Py_INCREF(&vardb_vardbRingbufferType);
    PyModule_AddObject(
        module, "api", reinterpret_cast<PyObject*>(&vardb_vardbRingbufferType)
    );
    
    // Register our exception object.
    
    exception = PyErr_NewException(
        const_cast<char*>("VardbRingbuffer.exception"), NULL, NULL
        
    );
    Py_INCREF(exception);
    PyModule_AddObject(module, "exception", exception);
}
