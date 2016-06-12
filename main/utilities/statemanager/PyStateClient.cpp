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
# @file   PyStateClient.cpp
# @brief  Python bindings to state client.
# @author <fox@nscl.msu.edu>
*/

#include <Python.h>
#include <stdexcept>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <iostream>

#include "CStateClientApi.h"
#include "PyStateManager.h"


/* Module specific error: */

static PyObject* error;

/*
 * Api Object storage:
 */
typedef statemanager_ApiObject stateclient_ApiObject;

/*
 *  Utility functions
 */

/**
 * raise
 *    Raise a local exception.
 *  @param message - the exception message.
 *  @return null.
 */
static PyObject*
raise(std::string msg)
{
    PyErr_SetString(error, msg.c_str());
    return NULL;
}

/**
 * getApi
 *   Get the Api from object local storage.
 * @param self - the object pointer as a PyObject.
 */
static CStateClientApi*
getApi(PyObject* self)
{
    stateclient_ApiObject* pThis = reinterpret_cast<stateclient_ApiObject*>(self);
    return dynamic_cast<CStateClientApi*>(pThis->m_pApi);
}

/*
 *    API object canonicals.
 */


/**
 * Api_new
 *    Allocate storage for a new Api object.
 *
 * @param type - Pointer to the type data structurwe.
 * @param args - Argument list passed.
 * @param kwargs - Keworded parameters passed.
 *
 * @return PyObject*  - Pointer to the created object.
 * @retval NULL       - If unable to comply.
 */
static PyObject*
Api_new(PyTypeObject* type, PyObject* args, PyObject* kwargs)
{
    stateclient_ApiObject* self =
        reinterpret_cast<stateclient_ApiObject*>(type->tp_alloc(type, 0));
    if (self) {
        self->m_pApi = NULL;
        self->m_pPrograms = NULL;
        return reinterpret_cast<PyObject*>(self);
    } else {
        PyErr_SetString(error,"Could not allocate an api object");
        return NULL;
    }
}
/**
 * Api_init
 *    Initialize the contents of the object.
 *    We need the following parameteres:
 *    *  URI for the request port.
 *    *  URI for the subscription port.
 *    *  name of the program we represent.
 *
 * @param self   - ponter to this object.
 * @param args   - The argument list.
 * @param kwargs - Keyword arguments to init.
 * @return int   - -1 for failure, 0 for success.
 */
static int
Api_init(stateclient_ApiObject* self, PyObject* args, PyObject* kwds)
{
    const char* reqUri;
    const char* subUri;
    const char* program;
    
    if (!PyArg_ParseTuple(args, "sss", &reqUri, &subUri, &program)) {
        PyErr_SetString(error, "need requri suburi, program");
        return -1;
    }
    /* Destroy any pre-existing m_pApi and create a new one. */
    
    delete self->m_pApi;
    try {
        self->m_pApi = new CStateClientApi(reqUri, subUri, program);
        self->m_pPrograms = self->m_pApi->getProgramApi();
    }
    catch(std::exception& e) {
        PyErr_SetString(error, e.what());
        return -1;
    }
    
    return 0;
}

/**
 * Api_delete
 *   Destructor of Api objects.
 *
 *  @param self - Pointer to the object to destroy.
 */
static void
Api_delete(stateclient_ApiObject* self)
{
    delete self->m_pApi;
    self->ob_type->tp_free(reinterpret_cast<PyObject*>(self));
}


/**
 * getstate
 *   Return the program's current state.
 *
 * @param self - the object on which this method was called.
 * @param args - tuple of method parameters.
 * @return PyObject* - string that contains the current state.
 */
static PyObject*
getstate(PyObject* self, PyObject* args)
{
    if (PySequence_Size(args) != 0) {
        return raise("getstate should not be passed parametrs");
    }
    
    CStateClientApi* pApi = getApi(self);
    std::string state;
    try {
        state = pApi->getState();
    }
    catch(std::exception& e) {
        return raise(e.what());
    }
    
    PyObject* result = PyString_FromString(state.c_str());
    Py_INCREF(result);
    return result;
}

/**
 * setstate
 *    Set the program's state.
 *
 * @param self - Object on which the method is executed.
 * @param args - Additional  parameters (one string - the new state).
 * @return PyNone
 */
static PyObject*
setstate(PyObject* self, PyObject* args)
{
    char* newState;
    
    if (!PyArg_ParseTuple(args, "s", &newState)) {
        return raise("sestate - needs a new state parameter");
    }
    
    CStateClientApi* pApi = getApi(self);
    try {
        pApi->setState(newState);
    }
    catch(std::exception& e) {
        return raise(e.what());
    }
    Py_RETURN_NONE;
}

/**
 * isenabled
 *    Check the program's enable flag.
 *  @param self - Pointer to the object on which this method is being called.
 *  @param args - Argument list (must be empty).
 *  @return PyObject* - Boolean indicating the state.
 */
static PyObject*
isenabled(PyObject* self, PyObject* args)
{
    if (PySequence_Size(args) != 0) {
        return raise("isenabled - takes no parametesr");
    }
    
    CStateClientApi* pApi = getApi(self);
    bool result;
    try {
        result = pApi->isEnabled();
    }
    catch (std::exception& e) {
        return raise(e.what());
    }
    
    if (result) {
        Py_RETURN_TRUE;
    } else {
        Py_RETURN_FALSE;
    }
}

/**
 * isstandalone
 *    Check the program's standalone flag.
 *
 *  @param self - Pointer to the object on which this method is being called.
 *  @param args - Argument list (must be empty).
 *  @return PyObject* - Boolean indicating the state.
 */
static PyObject*
isstandalone(PyObject* self, PyObject* args)
{
    if (PySequence_Size(args) != 0) {
        return raise("isenabled - takes no parametesr");
    }
    
    CStateClientApi* pApi = getApi(self);
    bool result;
    try {
        result = pApi->isStandalone();
    }
    catch (std::exception& e) {
        return raise(e.what());
    }
    
    if (result) {
        Py_RETURN_TRUE;
    } else {
        Py_RETURN_FALSE;
    }
    
}
/**
 * getttitle
 *    Return the current title string.
 *
 *  @param self - Pointer to the object on which this method is being called.
 *  @param args - Argument list (must be empty).
 *  @return PyObject* - Boolean indicating the state.
 */
static PyObject*
gettitle(PyObject* self, PyObject* args)
{
    if (PySequence_Size(args) != 0) {
        return raise("isenabled - takes no parametesr");
    }
    
    CStateClientApi* pApi = getApi(self);
    std::string titleString;
    try {
        titleString = pApi->title();
    }
    catch (std::exception& e) {
        return raise(e.what());
    }
    PyObject* result = PyString_FromString(titleString.c_str());
    Py_INCREF(result);
    return result;
    
}

/**
* runnumber
*     Return the current run number.
*
* @param self - Pointer to the object whose method this is.
* @param args - Pointer to the python argument list (tuple).
* @return PyObject* - long -run number.
*/
static PyObject*
runnumber(PyObject* self, PyObject* args)
{
    if (PySequence_Size(args) != 0) {
        return raise("runnumber - takes no parameters");
    }
    
    CStateClientApi* pApi = getApi(self);
    int runNumber;
    try {
        runNumber = pApi->runNumber();
    }
    catch (std::exception& e) {
        return raise(e.what());
    }
    
    PyObject* result = PyLong_FromUnsignedLong(runNumber);
    Py_INCREF(result);
    return result;
}

/**
* recording
*     Returns state of recording bool.
*
* @param self - Pointer to the object whose method this is.
* @param args - Pointer to the python argument list (tuple).
* @return PyObject* - Py_TRUE or Py_FALSE.
*/
static PyObject*
recording(PyObject* self, PyObject* args)
{
    if (PySequence_Size(args) != 0) {
        return raise("runnumber - takes no parameters");
    }
    
    CStateClientApi* pApi = getApi(self);
    bool recording;
    try {
        recording = pApi->recording();
    }
    catch (std::exception& e) {
        return raise(e.what());
    }
    if (recording) {
        Py_RETURN_TRUE;        
    } else {
        Py_RETURN_FALSE;
    }
}

/**
* outring
*     Return string containing the output ring.
*
* @param self - Pointer to the object whose method this is.
* @param args - Pointer to the python argument list (tuple).
* @return PyObject* - string with ringname.
*/
static PyObject*
outring(PyObject* self, PyObject* args)
{
    // No parameters:
    
    if (PySequence_Size(args) != 0) {
        return raise("runnumber - takes no parameters");
    }
    
    CStateClientApi* pApi = getApi(self);
    std::string ring;
    try {
        ring = pApi->outring();
    }
    catch (std::exception& e) {
        return raise(e.what());
    }
    
    PyObject* result = PyString_FromString(ring.c_str());
    Py_INCREF(result);
    return result;
}

/**
 * inring
 *   Get input ring name.
* @param self - Pointer to the object whose method this is.
* @param args - Pointer to the python argument list (tuple).
* @return PyObject* - string with ringname.
*/
static PyObject*
inring(PyObject* self, PyObject* args)
{
    if (PySequence_Size(args) != 0) {
        return raise("runnumber - takes no parameters");
    }
    
    CStateClientApi* pApi = getApi(self);
    std::string ring;
    try {
        ring = pApi->inring();
    }
    catch (std::exception& e) {
        return raise(e.what());
    }
    
    PyObject* result = PyString_FromString(ring.c_str());
    Py_INCREF(result);
    return result;    
}
/**
* waitTransition
*     Wait for a transition or timeout.
*
* @param self - Pointer to the object whose method this is.
* @param args - Pointer to the python argument list (tuple).
* @return PyObject* - Map with keys:
*                     * 'changed' true if there was a transition false if tiemout.
*                     * 'state'   if 'changed' is true, the new state.
* @note If there is a parameter it is the time out in milliseconds where the value
*       -1 means never timeout and is what happens when no parameter is present.
*/
static PyObject*
waitTransition(PyObject* self, PyObject* args)
{
    int timeout;
    if (PySequence_Size(args) == 0) {
        timeout = -1;
    } else if(!PyArg_ParseTuple(args, "i", &timeout)) {
        return raise("waitTransition can have at most one parameter, a timeout");
    }
    if (timeout < -1) {
        return raise("waitTransition timeouts must be -1 or >= 0");
    }
    
    CStateClientApi* pApi = getApi(self);
    
    std::string newState;
    bool        transitioned;
    try {
        transitioned = pApi->waitTransition(newState, timeout);
    }
    catch (std::exception& e) {
        return raise(e.what());
    }
    
    /* Make the dict - it alays has a 'changed' item:
       It has a 'state' if transitioned is true.
    */
    
    PyObject* result = PyDict_New();
    if(transitioned) {
        PyDict_SetItemString(result, "changed", Py_True);
        
        PyObject* state = PyString_FromString(newState.c_str());
        PyDict_SetItemString(result, "state", state);
    } else {
        PyDict_SetItemString(result, "changed", Py_False);
    }
    
    Py_INCREF(result);
    return result;
}

/* Api object dispatch table:  */

static PyMethodDef ApiObjectMethods[] = {
     {"getstate",  getstate, METH_VARARGS, "Get program's state."},
     {"setstate",  setstate, METH_VARARGS, "Set program's state"},
     {"isenabled", isenabled, METH_VARARGS, "Check program enable flag"},
     {"isstandalone", isstandalone, METH_VARARGS, "Check program standalone flag"},
     {"gettitle",  gettitle, METH_VARARGS, "Get the current title"},
     {"runnumber", runnumber, METH_VARARGS, "Get current run number"},
     {"recording", recording, METH_VARARGS, "Reg recording state"},
     {"outring",   outring,   METH_VARARGS, "Get output ring"},
     {"inring",    inring,    METH_VARARGS, "Get input ring"},
     {"waitTransition", waitTransition, METH_VARARGS, "Wait/get transition"},
     {NULL, NULL, 0, NULL}                /* End of method definition marker */       
};

/* Api type descriptions */

static PyTypeObject stateClient_ApiType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "statecient.Api",             /*tp_name*/
    sizeof(stateclient_ApiObject), /*tp_basicsize*/
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
    "State client API wrapper objects", /* tp_doc */
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
    (initproc)Api_init,      /* tp_init */
    0,                         /* tp_alloc */
    Api_new,                 /* tp_new */
};

/* Module level method dispatch table. */

static PyMethodDef StateClientClassMethods[] = {
  {NULL, NULL, 0, NULL}                /* End of methods sentinell. */  
};

/**
 * countMethods
 *    Given a pointer to a method dispatch table, counts the number
 *    of methods defined in it:
 *
 *   @param tbl - pointer to the first entry of the table.
 *   @return size_t - number of methods in the table.
 */
static size_t
countMethods(PyMethodDef* tbl)
{
    size_t result(0);
    while(tbl->ml_name) {
        result++;
        tbl++;
    }
    return result;
}

/**
 * addMethods
 *   Given a pointer to a set of method slots fills them in from a source
 *   table.
 *   -   There must be sufficient slots in the destination for the method
 *       defs in the source table.
 *   -   The copy is shallow.
 *
 *   @param p    - Pointer to the first slot to fill.
 *   @param m    - Pointer to the methods to fil
 *   @return PyMethodDef*  - Pointer to the next free table slot.
 */
static PyMethodDef*
addMethods(PyMethodDef* p, PyMethodDef* m)
{
    while (m->ml_name) {
        memcpy(p, m, sizeof(PyMethodDef));
        p++;
        m++;
    }
    return p;
}
/**
 *  Build a method dispatch table from our methods and those of
 *  our base class.
 *
 *  @param mymethods - points to the dispatch table for our methods.
 *  @param baseMethods - Points to the dispatch table for our base class methods.
 *  @return PyMethoDef* - Dynamically allocated dispatch table with methods from
 *                        both classes.
 * (Yeah if I really understood how to do this in the API I'm sure it could be
 *  simple  filling in tp_base and then what?)
 */
static PyMethodDef*
buildDispatchTable(PyMethodDef* mymethods, PyMethodDef* baseMethods)
{
    // Total up the methods..
    
    size_t numMyMethods = countMethods(mymethods);
    size_t numBaseMethods = countMethods(baseMethods);
    
    // Allocate the result table.
    
    PyMethodDef* result =
        reinterpret_cast<PyMethodDef*>(calloc(
            (numMyMethods + numBaseMethods + 1), sizeof(PyMethodDef)
        ));
    PyMethodDef* p      = result;
    
    // Fill in the table with methods from both classes
    
    p = addMethods(p, mymethods);
    addMethods(p, baseMethods);
    
    return result;
}

/**
 * initpystateclient
 *    Initialize the class.
 */

PyMODINIT_FUNC
initstateclient(void)
{
    PyObject* module;
    
    /* Create the module object.  that's the minimum. */
    
    module = Py_InitModule3(
        "stateclient", StateClientClassMethods, "CStateClient encapsulation"
    );
    if (module == NULL) {
        return;                    // For future expansion.
    }

    /* Create our exception */
    
    error = PyErr_NewException(
        const_cast<char*>("stateclient.error"),       
        NULL, NULL
    );

    Py_INCREF(error);
    PyModule_AddObject(module, "error", error);
  
 /* Register our new type */
 
    // First we need to build the combined method dispatch table and
    // plug it into the api type tp_methods
    
    stateClient_ApiType.tp_methods =
        buildDispatchTable(ApiObjectMethods, StateManagerMethods);
    
    if (PyType_Ready(&stateClient_ApiType) < 0) {
        return;
    }
    Py_INCREF(&stateClient_ApiType);
    PyModule_AddObject(
        module, "Api", reinterpret_cast<PyObject*>(&stateClient_ApiType)
    );    


}

void* gpTCLApplication(0);