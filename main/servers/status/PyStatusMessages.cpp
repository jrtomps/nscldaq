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
# @file   PyStatusMessages.cpp
# @brief  Python bindings to the status message generation API.
# @author <fox@nscl.msu.edu>
*/
#include <Python.h>
#include <CStatusMessage.h>
#include <exception>
#include <string>


static PyObject*  exception;

static bool       testMode(false);                  // if true, sockets are ZMQ_PUSH,
zmq::context_t*   pContext(new zmq::context_t(1));  

typedef struct _ringstatistics_Data {
    PyObject_HEAD
    CStatusDefinitions::RingStatistics* m_pObject;
    zmq::socket_t*                      m_pSocket;
} ringStatisticsData, *pRingStatisticsData;

/**
 * common utilities:
 */

/**
 * iterableToStringVector
 *    Takes a python interable and returns a vector of strings containing its
 *    values.
 *
 *  @param obj  - The python interable.
 *  @return std::vector<std::string>L
 */
static std::vector<std::string>
iterableToStringVector(PyObject* objv) {
    std::vector<std::string> result;
    PyObject* iter = PyObject_GetIter(objv);
    if (!iter) return result;                  // exception in progress though.
    
    while (PyObject* item = PyIter_Next(iter)) {
        char* word = PyString_AsString(item);
        if (!word) {
            Py_DECREF(item);
            PyErr_SetString(exception, "iterable must contain only strings");
            return result;
        }
        result.push_back(std::string(word));
        Py_DECREF(item);
    }
    
    Py_DECREF(iter);                          // Release the iterator.
    return result;
}

/*----------------------------------------------------------------------------
 * The RingStatistics type/class.
 */


/**
 *  Canonical methods:
 */

/**
 * ringstatistics_new
 *   Allocate the data needed by the object.
 *   @param type - pointer to the type table (ringstatistics_Type)
 *   @param args - Positional args (none expected).
 *   @param kwargs - Keyword argument dict (empty is fine).
 *   @return PyObject* - Pointer to the storage we newly allocated for the object.
 */
static PyObject*
ringstatistics_new(PyTypeObject* type, PyObject* args, PyObject* kwargs)
{
    PyObject* self = type->tp_alloc(type, 0);         // allocate storage.
    if (!self) {
        /// Allocation failed.
        PyErr_SetString(exception, "Unable to allocate object storage");
        
    } else {
        
        // Initialize  the object data so that the components have not yet
        // been created.
        
        
        pRingStatisticsData  pThis = reinterpret_cast<pRingStatisticsData>(self);
        pThis->m_pObject = 0;
        pThis->m_pSocket = 0;
    }
    return self;
}


/**
 * ringstatistics_init
 *    Initializes the contents of a ringstatistics object.  This means:
 *    - Creating/connecting the socket.
 *    - Creating the API object.
 *    - Saving the two objects in object storage.
 *  @note the type of socket created depends on the testMode flag.  If true
 *        a push socket is created else a PUB socket is created.
 *  @param self - pointer to object storage.
 *  @param args - Positional arguments - need URI and optional appname.
 *  @param kwargs - Keywords parameters.
 *  @return status of the initialization:
 *  @retval  0    - success.
 *  @retval -1    - failure.
 */
static int
ringstatistics_init(PyObject* self, PyObject* args, PyObject* kwargs)
{
    const char* uri(0);
    const char* app("RingStatDaemon");
    int         parseStatus;
    
    size_t nParams = PyTuple_Size(args);
    if (nParams == 1) {
        parseStatus = PyArg_ParseTuple(args, "s", &uri);    
    } else if (nParams == 2) {
        parseStatus = PyArg_ParseTuple(args, "ss", &uri, &app);
    } else {
        parseStatus = -0;
    }
    if (!parseStatus) {
        PyErr_SetString(exception, "Only a URI and an optional appname can be supplied");
        return -1;
    }
    // This block turns any exceptions into python raises.
    
     bool raise(false);
     std::string msg;
     try {
        pRingStatisticsData  pThis = reinterpret_cast<pRingStatisticsData>(self);
        pThis->m_pSocket =
            new zmq::socket_t(*pContext, testMode ? ZMQ_PUSH : ZMQ_PUB);
        pThis->m_pSocket->connect(uri);
        pThis->m_pObject =
            new CStatusDefinitions::RingStatistics(*pThis->m_pSocket, app);
     }
     catch (std::exception& e) {
        msg = e.what();
        raise = true;
     }
    catch (...) {
        msg = "Unanticipated C++ exception type caught";
        raise = true;
    }
     if (raise) {
        PyErr_SetString(exception, msg.c_str());
        return -1;
     }
     
     return 0;
     
}
/**
 * ringstatistics_delete
 *    Dispose of object dynamic storage (to whit the wrapped object and the
 *    transport socket).
 *
 *  @param self - pointer to our object storage
 *  @note we also delete the object storage itself.
 */
static void
ringstatistics_delete(PyObject* self)
{
    pRingStatisticsData  pThis = reinterpret_cast<pRingStatisticsData>(self);
    delete pThis->m_pObject;
    delete pThis->m_pSocket;
    pThis->m_pSocket = nullptr;
    pThis->m_pObject = nullptr;
    
    self->ob_type->tp_free(self);
}

/**
 * Methods on instances of a RingStatistics object.

/**
 *  Tables needed by Python to connect data/code.
 */

/**
 * ringstatistics_startMessage
 *    Wrapper for CStatusDefinitions::RingStatistics::startMessage
 *    A message is opened and the ringbuffer name is provided so that
 *    the id message segment can be formatted.
 *
 * @param PyObject* self - Pointer to our object storage.
 * @param PyObject* args - Tuple containing one element  - the ringbuffer name.
 * @return PyObject* (Py_None on success, NULL if not).
 */
static PyObject*
ringstatistics_startMessage(PyObject* self, PyObject* args)
{
    const char* ring;
    pRingStatisticsData pThis = reinterpret_cast<pRingStatisticsData>(self);
    
    if(!PyArg_ParseTuple(args, "s", &ring)) {
        return NULL;
    }
    
    // The actual method is invoked inside a try/catch block that
    // maps C++ exceptions into raises of our exception type.
    
    try {
        CStatusDefinitions::RingStatistics* pStats = pThis->m_pObject;
        pStats->startMessage(std::string(ring));
    }
    catch(std::exception& e) {
        PyErr_SetString(exception, e.what());
        return NULL;
    }
    catch (...) {
        PyErr_SetString(exception, "Unanticipated C++ type exception returned");
        return NULL;
    }
    
    Py_RETURN_NONE;
}
/**
 * ringstatistics_endMessage
 *    Wraps CStatusDefinitions::RingStatistics::endMessage.  This indicates
 *    all the required chunks of the statistics message have been accumulated
 *    and hsould be dumped out.
 * @param self    - Pointer to ou object storage.
 * @param args    - Positional parameters... cannot be andy.
 * @return PyObject* (Py_None).
 */
static PyObject*
ringstatistics_endMessage(PyObject* self, PyObject* args)
{
    pRingStatisticsData pThis = reinterpret_cast<pRingStatisticsData>(self);
    
    if (PyTuple_Size(args) > 0) {
        PyErr_SetString(exception, "endMessage does not accept any parameters");
        return NULL;
    }
    
    try {
        CStatusDefinitions::RingStatistics* pStats = pThis->m_pObject;
        pStats->endMessage();
    }
    catch(std::exception& e) {
        PyErr_SetString(exception, e.what());
        return NULL;
    }
    catch (...) {
        PyErr_SetString(exception, "Unanticipated C++ type exception returned");
        return NULL;
    }
    
    Py_RETURN_NONE;
}
/**
 * ringstatistics_addProducer
 *    Adds a producder to the message being built up.  This wraps
 *    CStatusDefinitions::RingStatistics::addProducer
 *
 * @param self - pointer to object's data.
 * @param args - Positional parameters which must be a tuple containing in order:
 *               -  An iterable containing the command words.
 *               -  The number of operations (puts) the producer performed.
 *               -  The number of bytes the producer put.
 * @return PyObject* - Py_None;
 */
static PyObject*
ringstatistics_addProducer(PyObject* self, PyObject* args)
{
    pRingStatisticsData pThis = reinterpret_cast<pRingStatisticsData>(self);
    PyObject*           command;
    uint64_t            ops;
    uint64_t            bytes;
    const char*         format;
    if(sizeof(uint64_t) == sizeof(unsigned long)) format = "Okk";
    else if (sizeof(uint64_t) == sizeof(unsigned long long)) format = "OKK";
    else {
        PyErr_SetString(exception, "Cant' figure out format string to decode uint64_t");
    }
    
    if (!PyArg_ParseTuple(args, format, &command, &ops, &bytes)) {
        return NULL;
    }
    std::vector<std::string> cmdWords = iterableToStringVector(command);
    if(PyErr_Occurred()) return NULL;
    
    try {
        pThis->m_pObject->addProducer(cmdWords, ops, bytes);
    }
    catch (std::exception& e) {
        PyErr_SetString(exception, e.what());
        return NULL;
    }
    catch (...) {
        PyErr_SetString(exception, "Unanticipated C++ exception type caught.");
    }
    
    Py_RETURN_NONE;
}

/*  Method dispatch table for RingStatistics objects: */

static PyMethodDef RingStatistics_Methods[] {
    {"startMessage", ringstatistics_startMessage, METH_VARARGS,
     "Start building up the information needed to send a message"},
    {"endMessage", ringstatistics_endMessage, METH_VARARGS,
     "Complete and send a ring statistics message"},
     {"addProducer", ringstatistics_addProducer, METH_VARARGS,
      "Add a producer record to the message"},
    {NULL, NULL, 0, NULL}
};

static PyTypeObject ringstatistics_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "statusmessages.RingStatistics",       /*tp_name*/
    sizeof(ringStatisticsData), /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)(ringstatistics_delete), /*tp_dealloc*/
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
    "Encapsulation of CRingStatisticsMessage class.", /* tp_doc */
    0,                         /* tp_traverse */
    0,                         /* tp_clear */
    0,                         /* tp_richcompare */
    0,                         /* tp_weaklistoffset */
    0,                         /* tp_iter */
    0,                         /* tp_iternext */
    RingStatistics_Methods,           /* tp_methods */
    0,                         /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)ringstatistics_init,      /* tp_init */
    0,                         /* tp_alloc */
    ringstatistics_new,                 /* tp_new */
};

/*----------------------------------------------------------------------------
 * Module level methods (see ModuleMethods for the dispatch table).
 */

/**
 * enableTest
 *    This turns on the testMode flag for the module.  When that flag is true,
 *    zmq::sockets are created as ZMQ_PUSH sockets which are much easier
 *    to deal with in a test environment than ZMQ_PUB sockets.
 *    This method is only intended to be called by test programs/methods.
 *
 * @param PyObject* self  - pointer to the module object.
 * @param PyObject* args  - Tuple must be empty.
 * 
 * @return PyObject* (Py_None).
 */
static PyObject*
enableTest(PyObject* self, PyObject* args)
{
    if(PyTuple_Size(args) > 0) {
        PyErr_SetString(exception, "enableTest does not accept any parameters");
        return NULL;
    }
    testMode = true;
    
    Py_RETURN_NONE;
}
/**
 * disableTest
 *   Turns off the testMode flag for the module.  See enableTest for more
 *   information.
 *
 *  @param self - Pointer to the object's storage.
 *  @param args - Positional parameters (must be an empty tuple).
 *  @return PyObject* (Py_None).
 */
static PyObject*
disableTest(PyObject* self, PyObject* args)
{
    if (PyTuple_Size(args) > 0) {
        PyErr_SetString(exception, "disableTest takes no parameters");
    }
    testMode = false;
    
    Py_RETURN_NONE;
}
// Method dispatch table for module level methods:

static PyMethodDef ModuleMethods[] = {
    {"enableTest", enableTest,   METH_VARARGS, "Run Sockets in test mode"},
    {"disableTest", disableTest, METH_VARARGS, "Run sockets in normal mode"},
    {NULL, NULL, 0, NULL}                        // End of methods sentinell.
};

/**
 *  Python module initialization:
 *  -  Create the module.
 *  -  Define the exception
 *  -  Define the types (classes) inside the module.
 */
PyMODINIT_FUNC
initstatusmessages(void)
{
    PyObject* module;
    
    // Initialize the module.
    
    module = Py_InitModule3(
        "statusmessages", ModuleMethods,
        "Python bindings to the status message emitters."
    );
    if(module == nullptr) {
        return;                         // Though there's no way to say we failed.
    }
    
    // Initialize our module specific exception
    
    exception = PyErr_NewException(
        const_cast<char*>("statusmessages.exception"), nullptr, nullptr
    );
    Py_INCREF(exception);                 // Type object immune from grbg collector.
    PyModule_AddObject(module, "exception", exception);  // Add object to module.
    
    // Add the RingStatistics type:
    
    if (PyType_Ready(&ringstatistics_Type) < 0) {
        return;
    }
    Py_INCREF(&ringstatistics_Type);
    PyModule_AddObject(
        module, "RingStatistics", reinterpret_cast<PyObject*>(&ringstatistics_Type)
    );
}
