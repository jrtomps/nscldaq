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
# @file   pyNotifier.cpp
# @brief  Implement Python bindings to the variable manager subscription interface.
# @author <fox@nscl.msu.edu>
*/
#include <Python.h>
#include "CVarMgrSubscriptions.h"
#include <CPortManager.h>
#include <URL.h>
#include <string>
#include <stdexcept>
#include <Exception.h>
#include <structmember.h>

#define UNCONST(cstring)  const_cast<char*>(cstring)

/*----------------------------------------------------------------------------
 * Definitions the methods need to be able to see.
 */

static const std::string defaultService("vardb-changes");

PyObject* exception;

typedef struct {
    PyObject_HEAD
    CVarMgrSubscriptions* m_pSubscriptions;
    CVarMgrSubscriptions::FilterType m_accept;
    CVarMgrSubscriptions::FilterType m_reject;
} NotifierObject;

/*----------------------------------------------------------------------------
 * utility methods:
 */

/**
 * lookupService
 *    Translate a service name to a port number ina server.
 * @param host - Name of host in which to do the lookup.
 * @param svc  - Service name.
 * @return int  port number
 * @throw std::runtime_error in case the lookup fails.
 */
static int
lookupService(std::string host, std::string service)
{
    try {
        std::string me = CPortManager::GetUsername();
        CPortManager m(host);
        std::vector<CPortManager::portInfo> ports = m.getPortUsage();
        
        for (int i =0; i < ports.size(); i++) {
            if((me == ports[i].s_User)  && (service == ports[i].s_Application)) {
                return ports[i].s_Port;
            }
        }
        throw std::runtime_error("Failed to lookup port in the port manager");
    }
    catch (CException& e) {
        throw std::runtime_error(e.ReasonText());
    }
}

/**
 * processUri
 *   Given a URI, either returns the host and port it represents or throws
 *   an std::runtime_error if this can't be done.
 *
 * @param uri - URI string.
 * @return std::pair<std::string, int>  (host,port) pair.
 */
static std::pair<std::string, int>
processUri(const char* uri)
{
    try {
        URL url(uri);
        
        // Must be tcp protocol:
        
        if (url.getProto() != "tcp") {
            throw std::runtime_error("URI must use tcp protocol");
        }
        
        // Is the port directly supplied:
        
        std::string host = url.getHostName();
        int         port = url.getPort();
        std::string service = url.getPath();
        if (port > 0) {
            if (service != "/") {
                throw std::runtime_error("Service name and port cannot both be in URI");
            }
            return std::pair<std::string, int>(host, port);
        } else {
            // Need to translate a service name:
            
            if (service == "/") {
                service = defaultService;
            }
            
            port = lookupService(host, service);
            return std::pair<std::string, int>(host, port);
        }
        
    }
    catch(CException& e) {
        throw std::runtime_error(e.ReasonText());    
    }
}

/**
 * getSubscriptions
 *    Pull the subscription attribute out of an object.
 *
 * @param self - actually  NotifierObject*
 * @return CVarMgrSubscriptions*
 */
static CVarMgrSubscriptions*
getSubscriptions(PyObject* self)
{
    NotifierObject* p  = reinterpret_cast<NotifierObject*>(self);
    return p->m_pSubscriptions;
}
/*-----------------------------------------------------------------------------
 *  Object methods
 */

/**
 * Notifier_delete
 *    Release storage associated with an object.
 *    -  Delete m_pSubscriptions
 *    -  Delete the Notifier object that represents us.
 * @param self - Py object that represents 'self' being deleted.
 */
static void
Notifier_delete(NotifierObject* self)
{
    delete self->m_pSubscriptions;
    self->ob_type->tp_free(reinterpret_cast<PyObject*>(self));
}

/**
 * Notifier_new
 *     Create storage for a notifier object.  This does not initialize that
 *     storage, that's done by Notifier_init which represents the
 *     __init__ method of the class.
 *
 *  @param type - pointer to the type object.
 *  @param args - Any creational (positional)
 *  @param kwds - Any keyword args.
 *  @return PyObject*   NULL If allocation failed.
 */
static PyObject*
Notifier_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
    NotifierObject* self =
        reinterpret_cast<NotifierObject*>(type->tp_alloc(type, 0));
    if (self) {
        self->m_pSubscriptions = 0;
        return reinterpret_cast<PyObject*>(self);
    } else {
        PyErr_SetString(
            exception, "Could not allocate NotifierObject storage"
        );
        return NULL;
    }
}
/**
 * Notifier_init
 *    Initialize the object.  This means
 *    - Parsing the URI (which must be present).
 *    - Deleting any existing subscription object in the object storage.
 *    - Creating a new one based on the URI Passed in.
 *    - Handling any exception from the failure to create a new subscription
 *      object
 * @param self - Pointer to object storage.
 * @param args - Position sensitive parameters.
 * @param kwargs - Dict with keyword parameters.
 * @return int - 0 success, -1 failure with exception raised.
 */
static int
Notifier_init(PyObject* self, PyObject* args, PyObject* kwargs)
{
    const char* uri;
    NotifierObject* pObject = reinterpret_cast<NotifierObject*>(self);
    
    if (!PyArg_ParseTuple(args, "s", &uri)) {
        return -1;
    }
    
    delete pObject->m_pSubscriptions;            // Harmless if 0.
    
    try {
        std::pair<std::string, int> connectionInfo = processUri(uri);
        pObject->m_pSubscriptions =
            new CVarMgrSubscriptions(connectionInfo.first.c_str(), connectionInfo.second);
        pObject->m_accept = CVarMgrSubscriptions::accept;
        pObject->m_reject = CVarMgrSubscriptions::reject;
        
    }
    catch (std::exception& e) {
        PyErr_SetString(exception, e.what());
        return -1;
    }
    return 0;
}


/**
* Notifier_subscribe
*    Add a path to the set of subsriptions.
*
* @param self - Pointer to the object whose method this is.
* @param args - Pointer to the python argument list (tuple).
*               This contains only the path to subscribe
* @return PyObject* - Py_None
*/
static PyObject*
Notifier_subscribe(PyObject* self, PyObject* args)
{
    char* path;
    
    if (!PyArg_ParseTuple(args, "s", &path)) {
        return NULL;
    }
    
    CVarMgrSubscriptions* pSub = getSubscriptions(self);
    try {
        pSub->subscribe(path);
    }
    catch (std::exception& e) {
        PyErr_SetString(exception, e.what());
        return NULL;
    }
    
    Py_INCREF(Py_None);
    return Py_None;
}

/**
* Notifier_unsubscribe
*     Remove a subscription from the set of subscriptions
*
* @param self - Pointer to the object whose method this is.
* @param args - Pointer to the python argument list (tuple).
*               Contains only the path to unsub.
* @return PyObject* - Py_None
*/
static PyObject*
Notifier_unsubscribe(PyObject* self, PyObject* args)
{
    char* path;
    
    if (!PyArg_ParseTuple(args, "s", &path)) {
        return NULL;
    }
    
    CVarMgrSubscriptions* pSub = getSubscriptions(self);
    try {
        pSub->unsubscribe(path);
    }
    catch (std::runtime_error& e) {
        PyErr_SetString(exception, e.what());
        return NULL;
    }
    
    Py_INCREF(Py_None);
    return Py_None;
}

/**
* Notifier_waitmsg
*     Wait for an notification message with a timeout:
*     -   Timeouts that are positive integers specify the maximum number of ms
*         to wait.
*     -   Timeouts that are 0 never block.
*     -   Negative timeouts wait until a message or hell freezes whichever is first.
*
* @param self - Pointer to the object whose method this is.
* @param args - Pointer to the python argument list (tuple).
*               Contains the timeout as an integer.
* @return PyObject* - A Boolean that is true if there's a message, false if not.
*/
static PyObject*
Notifier_waitmsg(PyObject* self, PyObject* args)
{
    int timeout;
    if(!PyArg_ParseTuple(args, "i", &timeout)) {
        return NULL;
    }
    CVarMgrSubscriptions* pSub = getSubscriptions(self);
    bool                  result;
    try {
        result = pSub->waitmsg(timeout);
    }
    catch (std::exception& e) {
        PyErr_SetString(exception, e.what());
        return NULL;
    }
    
    PyObject* pResult;
    if (result) {
        pResult = Py_True;
    } else {
        pResult = Py_False;
    }
    Py_INCREF(pResult);
    return pResult;
}

/**
* Notifier_readable
*     Return true if there's a message immediately readable
*     (this is just like waitmsg(0) actually).
*
* @param self - Pointer to the object whose method this is.
* @param args - Pointer to the python argument list (tuple) (emtpy).
* @return PyObject* - bool indicating the result.
*/
static PyObject*
Notifier_readable(PyObject* self, PyObject* args)
{
    // The args tuple must be empty:
    
    if (PyTuple_Size(args) != 0) {
        PyErr_SetString(exception, "readable method takes no parameters");
        return NULL;
    }
    
    CVarMgrSubscriptions* pSub = getSubscriptions(self);
    bool                 result;
    
    try {
        result = pSub->readable();
    }
    catch (std::exception& e) {
        PyErr_SetString(exception, e.what());
        return NULL;
    }
    
    PyObject* pResult;
    if (result) {
        pResult = Py_True;
    } else {
        pResult = Py_False;
    }
    Py_INCREF(pResult);
    return pResult;
}

/**
* Notifier_read
*     Read the next notification method blocking until one arrives.
*
* @param self - Pointer to the object whose method this is.
* @param args - Pointer to the python argument list (tuple) (empty)
* @return PyObject* - Dict with the keys:
*             - path - Path of the item modified.
*             - op   - The actual modification operation performe.d
*             - data - Additional data about the modification.
*/
static PyObject*
Notifier_read(PyObject* self, PyObject* args)
{
    if (PyTuple_Size(args) != 0) {
        PyErr_SetString(exception, "read method takes no parameters");
        return NULL;
    }
    
    CVarMgrSubscriptions* pSub = getSubscriptions(self);
    CVarMgrSubscriptions::Message msg;
    
    try {
        msg = pSub->read();
    }
    catch (std::exception& e) {
        PyErr_SetString(exception, e.what());
        return NULL;
    }
    // Turn the msg into the dict we need to return:
    
    PyObject* result =PyDict_New();
    PyObject* path   = PyString_FromString(msg.s_path.c_str());
    PyObject* op     = PyString_FromString(msg.s_operation.c_str());
    PyObject* data   = PyString_FromString(msg.s_data.c_str());
    
    PyDict_SetItemString(result, "path", path);
    PyDict_SetItemString(result, "op",   op);
    PyDict_SetItemString(result, "data", data);
    
    Py_INCREF(result);
    return result;
    
}
/**
 * Notifier_filter
 *     Checks a pattern against the currently established set of filters.
 *     
 *
 * @param self - Pointer to the object whose method this is.
 * @param args - Pointer to the python argument list (tuple).
 * @return PyObject* - boolean True if the filters pass  else False
 */
 static PyObject*
 Notifier_filter(PyObject* self, PyObject* args)
 {
    const char* pattern;
    
    if (!PyArg_ParseTuple(args, "s", &pattern)) {
        return NULL;
    }
    
    bool result;
    CVarMgrSubscriptions* pSub = getSubscriptions(self);
    try {
        result = pSub->checkFilters(pattern);
    } catch (std::exception& e) {
        PyErr_SetString(exception, e.what());
        return NULL;
    }
    PyObject* pResult = PyBool_FromLong(result ? 1 : 0);
    Py_INCREF(pResult);
    return pResult;
 }

/**
* Notifier_addFilter
*     Adds a filter pattern.
*
* @param self - Pointer to the object whose method this is.
* @param args - Pointer to the python argument list (tuple).
*              The tuple will have a filter type and a filter pattner.
* @return PyObject* - None.
*/
static PyObject*
Notifier_addFilter(PyObject* self, PyObject* args)
{
    CVarMgrSubscriptions::FilterType filterType;
    const char*                      filterPattern;
    
    // pull the parameters from the ntuple
    
    if(!PyArg_ParseTuple(args, "is", &filterType, &filterPattern)) {
        return NULL;
    }
    
    // Validate the filter type.
    
    if (filterType != CVarMgrSubscriptions::accept && filterType !=
        CVarMgrSubscriptions::reject
    ) {
        PyErr_SetString(exception, "Invalid filter type");
        return NULL;
        
    }
    
    
    // Add the filter in a try/catch block.
    
    try {
        CVarMgrSubscriptions* pSub = getSubscriptions(self);
        pSub->addFilter(filterType, filterPattern);
    }
    catch (std::exception& what) {
        PyErr_SetString(exception, what.what());
        return NULL;
    }
    
    Py_INCREF(Py_None);
    return Py_None;
}

// Module level dispatch table...there are no module level methods.

static PyMethodDef NotifierClassMethods[] = {
    {NULL, NULL, 0, NULL} 
};

// object level dispatch table for Notifer objects:

static PyMethodDef NotifierObjectMethods []   = {
    {"subscribe",   Notifier_subscribe,   METH_VARARGS, "Add a subscription"},
    {"unsubscribe", Notifier_unsubscribe, METH_VARARGS, "Remove a subscription"},
    {"waitmsg",     Notifier_waitmsg,     METH_VARARGS, "Wait for a message"},
    {"readable",    Notifier_readable,    METH_VARARGS, "Is there a message now?"},
    {"read",        Notifier_read,        METH_VARARGS, "Read next notification message"},
    {"filter",      Notifier_filter,      METH_VARARGS, "Check filter against pattern"},
    {"addFilter",   Notifier_addFilter,   METH_VARARGS, "Add a filter pattern"},
    {NULL, NULL, 0, NULL}  
};

// We're using this to hold the values for the CVarMgrSubscriptions::FilterType enum:

static PyMemberDef NotifierAttributes[] = {
  {UNCONST("Accept"), T_INT, offsetof(NotifierObject, m_accept), READONLY, UNCONST("Acceptance filter")},
  {UNCONST("Reject"), T_INT, offsetof(NotifierObject, m_reject), READONLY, UNCONST("Rejection filter")}, 
    {NULL, 0, 0, 0, 0}
};
// Storage for the object -- needs to contain a CVarMgrSubscriptions pointer.


// Type definition struct for Notifier:

static PyTypeObject NotifierType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "notifier.Notifier",             /*tp_name*/
    sizeof(NotifierObject), /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)(Notifier_delete), /*tp_dealloc*/
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
    "Server subscription object", /* tp_doc */
    0,		               /* tp_traverse */
    0,		               /* tp_clear */
    0,		               /* tp_richcompare */
    0,		               /* tp_weaklistoffset */
    0,		               /* tp_iter */
    0,		               /* tp_iternext */
    NotifierObjectMethods,     /* tp_methods */
    NotifierAttributes,        /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)Notifier_init,      /* tp_init */
    0,                         /* tp_alloc */
    Notifier_new,                 /* tp_new */
    
};

// Module initialization:

PyMODINIT_FUNC
initnotifier(void)
{
    PyObject* module;
    
    // Create and initialize the module object:
    
    module = Py_InitModule3(
        "notifier", NotifierClassMethods, "CSubscriptions encapsulation"
    );
    if (module == NULL) {
        return;                           // Failed.
    }
    
    // Create and register the module's exception type:
    
    exception = PyErr_NewException(const_cast<char*>("notifier.error"), NULL, NULL);
    Py_INCREF(exception);
    PyModule_AddObject(module, "error", exception);
    
    // Register the new data type:

    if(PyType_Ready(&NotifierType) < 0 ) {
        return;
    }
    Py_INCREF(&NotifierType);
    PyModule_AddObject(module, "Notifier", reinterpret_cast<PyObject*>(&NotifierType));
}
