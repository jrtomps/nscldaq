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
# @file   pyVarMgr.cpp
# @brief  Python bindings to variable manager API and factory.
# @author <fox@nscl.msu.edu>
*/

#include <Python.h>
#include <CVarMgrApi.h>
#include <CVarMgrApiFactory.h>
#include <CVariableDb.h>
#include <exception>

// Module specific exception:

static PyObject* exception;

// Define the ApiObject storage:

typedef struct {
    PyObject_HEAD
    CVarMgrApi* m_pApi;
} ApiObject;

/**   Utilities **/

/**
 * getApi
 *   Given a pointer to an ApiObject data struct, return the
 *   Api pointer.
 *
 *  @param obj - Actually an ApiObject*
 *  @return CVarMgr*
 */
static CVarMgrApi*
getApi(PyObject* obj)
{
    ApiObject* self = reinterpret_cast<ApiObject*>(obj);
    return self->m_pApi;
}

/**
 * getStringFromTuple
 *    Given a python tuple return the string that is stored
 *    in its n'th element
 *  @param tuple - Pointer to a python tuple object.
 *  @param n     - Index of item to get.
 *  @return const char* string stored in that element.
 *  @throw std::runtime_error -  if the index is out of range or the object
 *                               selected is not a string
 *  @note - caller is responsible for ensuring that tuple is one.
 */
const char*
getStringFromTuple(PyObject* tuple, Py_ssize_t n)
{
    PyObject* item  = PyTuple_GetItem(tuple, n);
    if (!item) {
        throw std::runtime_error("varmgr.getStringFromTuple could not select tuple element");
    }
    
    if (!PyString_Check(item)) {
        throw std::runtime_error("varmgr.getStringFromTuple - tuple element is not a string");
    }
    return PyString_AS_STRING(item);
}

/**
 * makeVarDict
 *   Make a dict that represents a varinfo object
 *
 * @param info - Information about a variable (CVarMgrApi::VarInfo).
 * @return PyObject* dict.
 */
static PyObject*
makeVarDict(CVarMgrApi::VarInfo info)
{
    PyObject* pResult = PyDict_New();
    PyDict_SetItemString(pResult, "name", PyString_FromString(info.s_name.c_str()));
    PyDict_SetItemString(pResult, "type", PyString_FromString(info.s_typeName.c_str()));
    PyDict_SetItemString(pResult, "value", PyString_FromString(info.s_value.c_str()));
    
    
    return pResult;
}

/** Module level functions **/

/**
 * varmgr_create
 *   Create a database given a filename.  This is actually not a CVarMgrApi
 *   function but is provided for convenience since databases have to get
 *   made somehow.
 *
 *  @param self   - Python object that represents the module.
 *  @param args   - Parameters to to the method (the name of the file).
 *  @return PyObject*  Py_None but can also throw exceptions if any come from
 *                  the lower library layers (these get transmuted to
 *                  our exception type)
 */
static PyObject*
varmgr_create(PyObject* self, PyObject* args)
{
    const char* dbPath;
    if (!PyArg_ParseTuple(args, "s", &dbPath)) {
        return NULL;               // Error.
    }
    try {
        CVariableDb::create(dbPath);    
    }
    catch (std::exception& e) {
        PyErr_SetString(exception, e.what());
        return NULL;
    }
    
    Py_INCREF(Py_None);
    return Py_None;

    
}
/**  Object level methods **/

/**
 * Api_delete
 *   Object destructor for Api type
 *   - Delete the API object.
 *   - Delete the ApiObject Struct:
 *
 * @param self - Pointer to the object to destroy
 */
static void
Api_delete(ApiObject* self)
{
    delete self->m_pApi;
    self->ob_type->tp_free(reinterpret_cast<PyObject*>(self));
}

/**
 * Api_new
 *    Allocator for a new API object.
 *
 * @param type - pointer to the type object.
 * @param args - Any creational position sensitive objects.
 * @param kwds - Any keywords parameters.
 * @return Pointer to the newly created object on success, NULL If failed.
 * 
 */
static PyObject*
Api_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
    ApiObject* self = reinterpret_cast<ApiObject*>(type->tp_alloc(type, 0));
    if (self) {
        self->m_pApi = 0;
        return reinterpret_cast<PyObject*>(self);
        
    } else {
        // allocationfailed:
        PyErr_SetString(exception, "Could not allocated ApiObject");
        return NULL;
    }
}
/**
 * Api_init
 *    This is what other languages would call the constructor it corresponds to
 *    the Api class's __init__ method.
 *    - Pull the URI parameter from the objects.
 *    - Use the CVarMgrApiFactory to construct an API object.
 *    - Fill in the m_pApi member with a pointer to that API object.
 * @param self - Pointer to ourself.
 * @param args - Positional parameters.
 * @param kwds - Keyword parameters.
 * @return int  0 - success, -1 failure.
 */
static int
Api_init(PyObject* self, PyObject* args, PyObject* kwds)
{
    const char* pUri;
    ApiObject* pSelf = reinterpret_cast<ApiObject*>(self);
    if (!PyArg_ParseTuple(args, "s", &pUri)) {
        return -1;
    }
    delete pSelf->m_pApi;
    try {
        pSelf->m_pApi = CVarMgrApiFactory::create(pUri);
    }
    catch (std::exception& e) {
        PyErr_SetString(exception, e.what());
        return -1;
    }
    return 0;
}

/**
 * Api_mdkir
 *     Create a new directory.
 *  @param self - Pointer to the api object
 *  @param args - Call parameters, must have a path string.
 *  @return PyObject* Py_None on success, 0 on failure with
 *               error raised.
 */
static PyObject*
Api_mkdir(PyObject* self, PyObject* args)
{
    char* path;
    if (!PyArg_ParseTuple(args, "s", &path)) {
        return 0;
    }
    
   
    CVarMgrApi* pApi = getApi(self);
    
    try {
        pApi->mkdir(path);    
    }
    catch (std::exception& e) {
        PyErr_SetString(exception, e.what());
        return 0;
    }
    
    Py_INCREF(Py_None);
    return Py_None;
}
/**
 * Api_cd
 *    Set the default directory for this api object.
 * @param self - pointer to this object.
 * @param args - Parameters of the method call (the path).
 * @return PyObject* Py_None on success, 0 on error.
 */
static PyObject*
Api_cd(PyObject* self, PyObject* args)
{
    char* path;
    if (!PyArg_ParseTuple(args, "s", &path)) {
        return 0;
    }
    
    CVarMgrApi* pApi = getApi(self);
    
    try {
        pApi->cd(path);
    }
    catch (std::exception& e) {
        PyErr_SetString(exception, e.what());
        return 0;
    }
    
    Py_INCREF(Py_None);
    return Py_None;
    
}
/**
 * Api_getwd
 *    Return the working directory of this object.
 *  @param self - This object.
 *  @param args - Parameters - must be an empty tuple.
 *  @return PyObject* string containing the current working dir.
 * 
 */
static PyObject*
Api_getwd(PyObject* self, PyObject* args)
{
    // Require no parameters:
    
    if (PyTuple_Size(args) != 0) {
        PyErr_SetString(exception, "getwd takes no parameters");
        return 0;
    }
    
    CVarMgrApi*  pApi  = getApi(self);
    
    std::string wd = pApi->getwd();
    PyObject* pResult = PyString_FromString(wd.c_str());
    
    Py_INCREF(pResult);
    return pResult;
    
}
/**
 * Api_rmdir
 *    Remove a directory
 *
 *  @param self - Pointer to self.
 *  @param args - Parameters must contain a path.
 *  @return PyObject* - Py_None on success or  0 on error with error raised.
 */
static PyObject*
Api_rmdir(PyObject* self, PyObject* args)
{
    char* path;
    if (!PyArg_ParseTuple(args, "s", &path)) {
        return 0;
    }
    CVarMgrApi* pApi = getApi(self);
    try {
        pApi->rmdir(path);
    } catch(std::exception& e) {
        PyErr_SetString(exception, e.what());
        return 0;
    }
    
    Py_INCREF(Py_None);
    return Py_None;
    
}
/**
 * Api_declare
 *     Declare a new variable.  Note that this has a variable length parameter
 *     tuple:
 *     -  First parameter is always the variable path (string)
 *     -  Second parameter is always the data type (string)
 *     -  Third parameter is optional and is the initial value of the variable.
 *
 *  @param self - Pointer to object data.
 *  @param args - Pointer to argument tuple.
 *  @return Py_None.
 */
static PyObject*
Api_declare(PyObject* self, PyObject* args)
{
    // Validate parameter count:
    
    Py_ssize_t nargs = PyTuple_Size(args);
    if (nargs < 2) {
        PyErr_SetString(exception, "declare requires at least two parameters");
        return 0;
    }
    if (nargs > 3) {
        PyErr_SetString(exception, "declare takes at most three parameters");
        return 0;
    }
    
    try {
        // Extract the parameters:
        const char* path = getStringFromTuple(args, 0);
        const char* type = getStringFromTuple(args, 1);
        const char* initial(0);
        if (nargs == 3) {
            initial = getStringFromTuple(args, 2);
        }
        
        // Get the API and perform its declare method:
        
        CVarMgrApi* pApi = getApi(self);
        pApi->declare(path, type, initial);
    }
    catch (std::exception& e) {
        PyErr_SetString(exception, e.what());
        return 0;
    }
    
    Py_INCREF(Py_None);
    return Py_None;
}

/**
 * Api_set
 *    Set a new value for a variable.  Parameters are the path to the variable
 *    and the new value.
 *
 * @param self - Pointer to the python api object.
 * @param args - Pointer to the method parameter ntuple.
 * @return Py_None
 */
static PyObject*
Api_set(PyObject* self, PyObject* args)
{
    char* path;
    char* value;
    
    if (!PyArg_ParseTuple(args, "ss", &path, &value)) {
        return 0;
    }
    CVarMgrApi* pApi = getApi(self);
    
    try {
        pApi->set(path, value);
    }
    catch (std::exception& e) {
        PyErr_SetString(exception, e.what());
        return 0;
    }
    
    Py_INCREF(Py_None);
    return Py_None;
}
/**
* Api_get
*     Get the value of a variable.
*
* @param self - Pointer to the object whose method this is.
* @param args - Pointer to the python argument list (tuple).
* @return PyObject* - string - the variable's value.
*/
static PyObject*
Api_get(PyObject* self, PyObject* args)
{
    char* path;
    if (!PyArg_ParseTuple(args, "s", &path)) {
        return 0;
    }
    
    CVarMgrApi* pApi = getApi(self);
    
    std::string value;
    try {
        value = pApi->get(path);        
    }
    catch (std::exception& e) {
        PyErr_SetString(exception, e.what());
        return 0;
    }
    // make the string value:
    
    PyObject*  pValue;
    pValue = PyString_FromString(value.c_str());
    
    Py_INCREF(pValue);
    return pValue;
}

/**
* Api_defEnum
*     Define an enumerated variable type.
*
* @param self - Pointer to the object whose method this is.
* @param args - Pointer to the python argument list (tuple).
* @return PyObject* - Py_None
*/
static PyObject*
Api_defEnum(PyObject* self, PyObject* args)
{
    char*     typeName;
    PyObject* values;
    
    if (!PyArg_ParseTuple(args, "sO", &typeName, &values)) {
        return 0;
    }
    CVarMgrApi* pApi = getApi(self);
    try {
        // We need to iterate through values pulling out our
        // values:
        
        PyObject* p = PyObject_GetIter(values);
        if (!p) {
            throw std::runtime_error("Value parameter must be iterable");
        }
        CVarMgrApi::EnumValues vs;
        PyObject* value;
        while(value = PyIter_Next(p)) {
            if(!PyString_Check(value)) {
                Py_DECREF(p);
                throw std::runtime_error("An enumerator value is not a string");
            }
            vs.push_back(std::string(PyString_AsString(value)));
            Py_DECREF(value);
        }
        Py_DECREF(p);
        pApi->defineEnum(typeName, vs);
        
    }
    catch (std::exception& e) {
        PyErr_SetString(exception, e.what());
        return 0;
    }
    
    Py_INCREF(Py_None);
    return Py_None;
}

/**
* Api_defStateMachine
*     Define a state machine data type.  The parameters are
*     - The name of the new type.
*     - A dict whose keys are state name strings (must be strings)
*       and whose values are iterables containing the strings that are
*       valid target states from this state.
*
* @param self - Pointer to the object whose method this is.
* @param args - Pointer to the python argument list (tuple).
* @return PyObject* - Py_None
*/
static PyObject*
Api_defStateMachine(PyObject* self, PyObject* args)
{
    char*                typeName;
    PyObject*            pTransitionDict;
    CVarMgrApi::StateMap transitions;
    
    if (!PyArg_ParseTuple(args, "sO", &typeName, &pTransitionDict)) {
        return 0;
    }
    CVarMgrApi* pApi = getApi(self);
    try {
        // Transition map must be a dict:
        
        if (!PyDict_Check(pTransitionDict)) {
            throw std::runtime_error("Transition map (second param) must be a dict");
        }
        PyObject* keys = PyDict_Keys(pTransitionDict);
        PyObject* pKeyIterator = PyObject_GetIter(keys); // Keys must have an iterator.
        PyObject* pKey;
        while(pKey = PyIter_Next(pKeyIterator)) {
            if (!PyString_Check(pKey)) {
                Py_DECREF(pKeyIterator);
                Py_DECREF(pKey);
                throw std::runtime_error("Transition map keys must be strings");
            }
            char* key = PyString_AsString(pKey);
            PyObject* targetList = PyDict_GetItem(pTransitionDict, pKey);
            
            // Target list must be an iterable too:
            
            PyObject* pTargetIter = PyObject_GetIter(targetList);
            if (!pTargetIter) {
                Py_DECREF(pKeyIterator);
                Py_DECREF(pKey);
                throw std::runtime_error("Transition map values must be iterable");
            }
            
            PyObject* target;
            while (target = PyIter_Next(pTargetIter)) {
                if (!PyString_Check(target)) {
                    Py_DECREF(target);
                    Py_DECREF(pKey);
                    Py_DECREF(pKeyIterator);
                    Py_DECREF(pTargetIter);
                    
                    throw std::runtime_error("Transition map value list elements must be strings");
                }
                pApi->addTransition(
                    transitions, std::string(key), std::string(PyString_AsString(target)));
                Py_DECREF(target);
            }
                
                
                Py_DECREF(pTargetIter);
                 Py_DECREF(pKey);
        }
            
        Py_DECREF(pKeyIterator);
        
        pApi->defineStateMachine(typeName, transitions);
        
    } catch (std::exception& e) {
        PyErr_SetString(exception, e.what());
        return 0;
    }
    
    Py_INCREF(Py_None);
    return Py_None;
}

/**
* Api_ls
*     List subdirectories of the currentworking dir or some path
*     (absolute or relative to the cwd).
*
* @param self - Pointer to the object whose method this is.
* @param args - Pointer to the python argument list (tuple).
* @return PyObject* - tuple of strings that are the directory names.
*/
static PyObject*
Api_ls(PyObject* self, PyObject* args)
{
    char* path(0);
    std::vector<std::string> subdirs;
    try {
        if (PyTuple_Size(args) == 1) {
            PyObject* pPath = PyTuple_GetItem(args, 0);
            if (!PyString_Check(pPath)) {
                throw std::runtime_error("The path parameter must be a string.");
            }
            path = PyString_AsString(pPath);
        }
        if (PyTuple_Size(args) > 1) {
            throw std::runtime_error( "At most one parameters (path) is allowed for ls");
        }
        
        CVarMgrApi* pApi = getApi(self);
        subdirs = pApi->ls(path);
    }
    catch (std::exception& e) {
        PyErr_SetString(exception, e.what());
        return 0;
    }
    // Marshall subdirs into the return value:
    
    PyObject* result = PyTuple_New(subdirs.size());
    for (int i =0; i < subdirs.size(); i++) {
        PyTuple_SetItem(result, i, PyString_FromString(subdirs[i].c_str()));
    }
    Py_INCREF(result);
    return result;

}

/**
* Api_lsvar
*     produce a list of the variables in a specified directory.
*
* @param self - Pointer to the object whose method this is.
* @param args - Pointer to the python argument list (tuple).  This is empty or a path.
* @return PyObject* - This will be a tuple with one element per variable.
*                     The tuple values will be dicts with the following keys:
*                     - 'name' - Name of the variable.
*                     - 'type' - Type name of the variable
*                     - 'value' - Current variable value.
*/
static PyObject*
Api_lsvar(PyObject* self, PyObject* args)
{
    char* path(0);
    std::vector<CVarMgrApi::VarInfo> vars;
    try {
        if (PyTuple_Size(args) == 1) {
            PyObject* pPath = PyTuple_GetItem(args, 0);
            if (PyString_Check(pPath)) {
                path = PyString_AsString(pPath);
            } else {
                throw std::runtime_error("Path parameter must be a string");
            }
        } else if (PyTuple_Size(args) > 1) {
            throw std::runtime_error("lsvar only takes at most one parameter");
        }
        
        CVarMgrApi* pApi = getApi(self);
        vars = pApi->lsvar(path);
    }    
    catch (std::exception& e) {
        PyErr_SetString(exception, e.what());
        return 0;
    }
    // marshall the result into a tuple of dicts
    
    PyObject* pResult = PyTuple_New(vars.size());
    for (int i = 0; i < vars.size(); i++) {
        PyTuple_SetItem(pResult, i, makeVarDict(vars[i]));
    }
    Py_INCREF(pResult);
    return pResult;
}


/**
* Api_rmvar
*     Removes a variable.
*
* @param self - Pointer to the object whose method this is.
* @param args - Pointer to the python argument list (tuple).
*               This should be a single string that is the path to the var
*               to delete
* @return PyObject* - PyNone
*/
static PyObject*
Api_rmvar(PyObject* self, PyObject* args)
{
    char* path;
    if (!PyArg_ParseTuple(args, "s", &path)) {
        return 0;
    }
    
    try {
        CVarMgrApi* pApi = getApi(self);
        pApi->rmvar(path);
    }
    catch (std::exception& e) {
        PyErr_SetString(exception, e.what());
        return 0;
    }
    
    Py_INCREF(Py_None);
    return Py_None;
}
// Module level dispatch table:

static PyMethodDef VarMgrClassMethods[] = {
    {"create", varmgr_create, METH_VARARGS, "Create a new variable database"},
    {NULL, NULL, 0, NULL} 
};

// Tables for the Api object:

static PyMethodDef ApiMethods[] {
    {"mkdir",   Api_mkdir,   METH_VARARGS, "Create directory"},
    {"cd",      Api_cd,      METH_VARARGS, "Set default directory"},
    {"getwd",   Api_getwd,   METH_VARARGS, "Get working directory"},
    {"rmdir",   Api_rmdir,   METH_VARARGS, "Remove a directory"},
    {"declare", Api_declare, METH_VARARGS, "Declare a new variables"},
    {"set",     Api_set,     METH_VARARGS, "Set a new value for a variable"},
    {"get",     Api_get,     METH_VARARGS, "Get the value of a variable"},
    {"defineEnum",
                Api_defEnum, METH_VARARGS, "Define an enumerated type"},
    {"defineStateMachine",
                Api_defStateMachine,
                             METH_VARARGS,  "Define a statemachine type"},
    {"ls",      Api_ls,      METH_VARARGS,  "List subdirectories"},
    {"lsvar",   Api_lsvar,   METH_VARARGS,  "List variables in directory"},
    {"rmvar",   Api_rmvar,   METH_VARARGS,  "Remove a variable"},
    {NULL, NULL, 0, NULL}
};



static PyTypeObject ApiType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "varmgr.Api",             /*tp_name*/
    sizeof(ApiObject), /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)(Api_delete), /*tp_dealloc*/
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
    "Variable database objects", /* tp_doc */
    0,		               /* tp_traverse */
    0,		               /* tp_clear */
    0,		               /* tp_richcompare */
    0,		               /* tp_weaklistoffset */
    0,		               /* tp_iter */
    0,		               /* tp_iternext */
    ApiMethods,        /* tp_methods */
    0,                         /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)Api_init,      /* tp_init */
    0,                         /* tp_alloc */
    Api_new,                 /* tp_new */
    
};

/**
 * VarMgrInit
 *    Initialize the module object:
 */
PyMODINIT_FUNC
initvarmgr(void)
{
    PyObject* module;
    
    // Create and initialize the module object:
    
    module = Py_InitModule3(
        "varmgr", VarMgrClassMethods, "CVarMgrApi encapsulation"
    );
    if (module == NULL) {
        return;                           // Failed.
    }
    
    // Create and register the module's exception type:
    
    exception = PyErr_NewException(const_cast<char*>("varmgr.error"), NULL, NULL);
    Py_INCREF(exception);
    PyModule_AddObject(module, "error", exception);
    
    // Register the Api type for this module:
    
    if (PyType_Ready(&ApiType) < 0) {
        return;
    }
    Py_INCREF(&ApiType);
    PyModule_AddObject(module, "Api", reinterpret_cast<PyObject*>(&ApiType));
}