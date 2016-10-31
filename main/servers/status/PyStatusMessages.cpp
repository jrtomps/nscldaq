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

typedef struct _readoutStatisticsData {
    PyObject_HEAD
    CStatusDefinitions::ReadoutStatistics* m_pObject;
    zmq::socket_t*                         m_pSocket;

} readoutStatisticsData, *pReadoutStatisticsData;

typedef struct _logmessage_Data {
    PyObject_HEAD
    CStatusDefinitions::LogMessage*   m_pObject;
    zmq::socket_t*                    m_pSocket;
} logMessageData, *pLogMessageData;

typedef struct _statechange_Data {
    PyObject_HEAD
    CStatusDefinitions::StateChange*   m_pObject;
    zmq::socket_t*                    m_pSocket;
} stateChangeData, *pStateChangeData;

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
/**
*   The StateChange class.
*/

/**
 *  Canonicalmethods.
 */
/**
 * statechange_new
 *   Allocate the data needed by the object.
 *   @param type - pointer to the type table (statechange_Type)
 *   @param args - Positional args (none expected).
 *   @param kwargs - Keyword argument dict (empty is fine).
 *   @return PyObject* - Pointer to the storage we newly allocated for the object.
 */
static PyObject*
statechange_new(PyTypeObject* type, PyObject* args, PyObject* kwargs)
{
    PyObject* self = type->tp_alloc(type, 0);         // allocate storage.
    if (!self) {
        /// Allocation failed.
        PyErr_SetString(exception, "Unable to allocate object storage");
        
    } else {
        
        // Initialize  the object data so that the components have not yet
        // been created.
        
        
        pStateChangeData  pThis = reinterpret_cast<pStateChangeData>(self);
        pThis->m_pObject = 0;
        pThis->m_pSocket = 0;
    }
    return self;
}


/**
 * statechange_init
 *    Initializes the contents of a statechange object.  This means:
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
statechange_init(PyObject* self, PyObject* args, PyObject* kwargs)
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
        pStateChangeData  pThis = reinterpret_cast<pStateChangeData>(self);
        
        // testMode means we're testing publication, so we et a PUB/bind socket.
        // otherwise we get a PUSH connect socket.
        
        if (testMode) {
            pThis->m_pSocket = new zmq::socket_t(*pContext,  ZMQ_PUB);
            pThis->m_pSocket->bind(uri);
        } else {
            pThis->m_pSocket = new zmq::socket_t(*pContext, ZMQ_PUSH );
            pThis->m_pSocket->connect(uri);
        }
        pThis->m_pObject =
            new CStatusDefinitions::StateChange(*pThis->m_pSocket, app);
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
 * statechange_delete
 *    Dispose of object dynamic storage (to whit the wrapped object and the
 *    transport socket).
 *
 *  @param self - pointer to our object storage
 *  @note we also delete the object storage itself.
 */
static void
statechange_delete(PyObject* self)
{
    pStateChangeData  pThis = reinterpret_cast<pStateChangeData>(self);
    delete pThis->m_pObject;
    delete pThis->m_pSocket;
    pThis->m_pSocket = nullptr;
    pThis->m_pObject = nullptr;
    
    self->ob_type->tp_free(self);
}

/**
 * Methods that can be called on constructed object.
 */

/**
 * statechange_logChange
 *    Log a change of state.
 *
 *  @param self   - pointer to object data.
 *  @param args   - positional parameters which are the state being left and the
 *                  state being entered.
 *  @return PyObject* Py_None
 */
static PyObject*
statechange_logChange(PyObject* self, PyObject* args)
{
    const char* from;
    const char* to;
    
    if (!PyArg_ParseTuple(args, "ss", &from, &to)) {
        return NULL;
    }
    
    pStateChangeData  pThis = reinterpret_cast<pStateChangeData>(self);
    
    try {
        pThis->m_pObject->logChange(std::string(from), std::string(to));
    }
    catch (std::exception& e) {
        PyErr_SetString(exception, e.what());
        return NULL;
    }
    catch (...) {
        PyErr_SetString(exception, "Unexpected C++ exception type caught");
        return NULL;
    }
    
    Py_RETURN_NONE;
}

/**
 * Tables needed to build the type/class
 */

static PyMethodDef statechange_Methods[] = {
    {"logChange", statechange_logChange, METH_VARARGS,
    "Log a state change"},
    {NULL, NULL, 0, NULL}
};

static PyTypeObject statechange_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "statusmessages.statechange",       /*tp_name*/
    sizeof(stateChangeData), /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)(statechange_delete), /*tp_dealloc*/
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
    "Encapsulation of StateChange class.", /* tp_doc */
    0,                         /* tp_traverse */
    0,                         /* tp_clear */
    0,                         /* tp_richcompare */
    0,                         /* tp_weaklistoffset */
    0,                         /* tp_iter */
    0,                         /* tp_iternext */
    statechange_Methods,           /* tp_methods */
    0,                         /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)statechange_init,      /* tp_init */
    0,                         /* tp_alloc */
    statechange_new,                 /* tp_new */
};

/*---------------------------------------------------------------------------
 * The LogMessage class
 */

/**
 * Canonical methods:
 */
/**
 * logmessage_new
 *   Allocate the data needed by the object.
 *   @param type - pointer to the type table (logmessage_Type)
 *   @param args - Positional args (none expected).
 *   @param kwargs - Keyword argument dict (empty is fine).
 *   @return PyObject* - Pointer to the storage we newly allocated for the object.
 */
static PyObject*
logmessage_new(PyTypeObject* type, PyObject* args, PyObject* kwargs)
{
    PyObject* self = type->tp_alloc(type, 0);         // allocate storage.
    if (!self) {
        /// Allocation failed.
        PyErr_SetString(exception, "Unable to allocate object storage");
        
    } else {
        
        // Initialize  the object data so that the components have not yet
        // been created.
        
        
        pLogMessageData  pThis = reinterpret_cast<pLogMessageData>(self);
        pThis->m_pObject = 0;
        pThis->m_pSocket = 0;
    }
    return self;
}


/**
 * logmessage_init
 *    Initializes the contents of a logmessage object.  This means:
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
logmessage_init(PyObject* self, PyObject* args, PyObject* kwargs)
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
        pLogMessageData  pThis = reinterpret_cast<pLogMessageData>(self);
        
        // Normally we push data to a known server.  In testMode, however
        // we publish data and bind so that subscriptions  can be tested without
        // the services of an aggregator process.
        
        if (testMode) {
            pThis->m_pSocket = new zmq::socket_t(*pContext,  ZMQ_PUB);
            pThis->m_pSocket->bind(uri);
        } else {
            pThis->m_pSocket = new zmq::socket_t(*pContext, ZMQ_PUSH );
            pThis->m_pSocket->connect(uri);
        }
        pThis->m_pObject =
            new CStatusDefinitions::LogMessage(*pThis->m_pSocket, app);
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
 * logmessage_delete
 *    Dispose of object dynamic storage (to whit the wrapped object and the
 *    transport socket).
 *
 *  @param self - pointer to our object storage
 *  @note we also delete the object storage itself.
 */
static void
logmessage_delete(PyObject* self)
{
    pLogMessageData  pThis = reinterpret_cast<pLogMessageData>(self);
    delete pThis->m_pObject;
    delete pThis->m_pSocket;
    pThis->m_pSocket = nullptr;
    pThis->m_pObject = nullptr;
    
    self->ob_type->tp_free(self);
}

/**
 * methods that can be called on a constructed object.
 */

/**
 * logmessage_Log
 *    Emit a log message.
 *
 *  @param self   - pointer to our storage,.
 *  @param args   - positional arguments which are:
 *                  - Severity - a severity code.
 *                  - The message string.
 *  @return PyObject* - Py_None
 */
PyObject*
logmessage_Log(PyObject* self, PyObject* args)
{
    uint32_t severity;
    const char* msg;
    
    static_assert(
        sizeof(uint32_t) == sizeof(unsigned int),
        "i format won't work on this platform"
    );
    
    if(!PyArg_ParseTuple(args, "is", &severity, &msg)) {
        return NULL;
    }
    
    pLogMessageData pThis = reinterpret_cast<pLogMessageData>(self);
    
    try {
        pThis->m_pObject->Log(severity, std::string(msg));
    }
    catch (std::exception& e) {
        PyErr_SetString(exception, e.what());
        return NULL;
    }
    catch(...) {
        PyErr_SetString(
            exception,
            "An unanticipated C++ exception was caught."
        );
        return NULL;
    }
    
    Py_RETURN_NONE;
}

/**
 * Tables needed by Python to construct this type.
 */

static PyMethodDef logmessage_Methods[] = {
    {"Log", logmessage_Log, METH_VARARGS,
    "Emit a log message"},
    {NULL, NULL, 0, NULL}
};
static PyTypeObject logmessage_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "statusmessages.logmessage",       /*tp_name*/
    sizeof(logMessageData), /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)(logmessage_delete), /*tp_dealloc*/
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
    "Encapsulation of ClogmessageMessage class.", /* tp_doc */
    0,                         /* tp_traverse */
    0,                         /* tp_clear */
    0,                         /* tp_richcompare */
    0,                         /* tp_weaklistoffset */
    0,                         /* tp_iter */
    0,                         /* tp_iternext */
    logmessage_Methods,           /* tp_methods */
    0,                         /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)logmessage_init,      /* tp_init */
    0,                         /* tp_alloc */
    logmessage_new,                 /* tp_new */
};

/*---------------------------------------------------------------------------
 *  The ReadoutStatistics type/class.
 */

/**
* Canonical methods
*/

/**
 * readoutstatistics_new
 *    Allocate object storage for ReadoutStatistics class instance.
 *
 * @param    type - Pointer to the type object.
 * @param    args - Positional args.
 * @param    kwargs - Keword parameters.
 * @return PyObject* - the allocated storage.
 */
static PyObject*
readoutstatistics_new(PyTypeObject* type, PyObject* args, PyObject* kwargs)
{
    // Attempt to allocate the object.
    
    PyObject* self = type->tp_alloc(type, 0);
    if(!self) {
        PyErr_SetString(exception, "Unable to allocate object storage");
    } else {
        // Initialize the fields.
        
        pReadoutStatisticsData pThis
            = reinterpret_cast<pReadoutStatisticsData>(self);
        pThis->m_pObject = nullptr;
        pThis->m_pSocket = nullptr;
    }
    
    // Return the result (win or lose)
    
    return self;
}
/**
 * readoutstatistics_init
 *    Initialize a readout statistics object.
 *
 * @param self - pointer to our object storage.
 * @param args - positional args, must have a URI.  There can also be an optional
 *               application name.  Note that the socket created will either be
 *               a PUSH (testMode true), or a PUB socket (testMode false).
 * @return int - 0  success, -1 failure.
 * 
 */
static int
readoutstatistics_init(PyObject* self, PyObject*args, PyObject* kwargs)
{
    const char* uri(0);
    const char* app("Readout");
    size_t nParams = PyTuple_Size(args);
    int parseStatus;
    
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
    // Actually create the componet objects in try block so the catchs can change
    // C++ exceptions into Python:
    
     bool raise(false);
     std::string msg;
     try {
        pReadoutStatisticsData  pThis = reinterpret_cast<pReadoutStatisticsData>(self);
        
        // Normally the socket pushes to a pull server that aggregates data
        // and relays it on a pub socket.  testMode, however, allows SUB
        // to be tested without requiring an aggregator so it binds a PUB socket
        // to the requested URI:
        
        if (testMode) {
            pThis->m_pSocket = new zmq::socket_t(*pContext,  ZMQ_PUB);
            pThis->m_pSocket->bind(uri);
        } else {
            
            pThis->m_pSocket = new zmq::socket_t(*pContext, ZMQ_PUSH);
            pThis->m_pSocket->connect(uri);
        }
        pThis->m_pObject =
            new CStatusDefinitions::ReadoutStatistics(*pThis->m_pSocket, app);
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
 * readoutstatistics_delete
 *    Free the storage associated with a ReadoutStatistics object.
 *    @param self - pointer to the object storage.
 */
static void
readoutstatistics_delete(PyObject* self)
{
    pReadoutStatisticsData  pThis = reinterpret_cast<pReadoutStatisticsData>(self);
    delete pThis->m_pObject;
    delete pThis->m_pSocket;
    pThis->m_pSocket = nullptr;
    pThis->m_pObject = nullptr;
    
    self->ob_type->tp_free(self);
}
/**
 *  Methods on constructed objects:
 */

/**
 * readoutstatistics_beginRun
 *   Logs the beginning of a run.  This establishes the contents of the
 *   ReadoutStatRunInfo message segments until the next call to this method.
 *   The underlying object will also emit a two part message consisting of a header
 *   and a RunStatRunInfo message segment.
 *
 * @param self - pointer to the object's storage.
 * @param args - Positional parameters.  A tuple that must have:
 *               *   The run number.
 *               *   The run title.
 * @return PyObject* - Py_None.
 */
static PyObject*
readoutstatistics_beginRun(PyObject* self, PyObject* args)
{
    uint32_t    runNumber;
    static_assert(sizeof(uint32_t) == sizeof(int), "Python I conversion won't work");
    const char* title;
    
    if (!PyArg_ParseTuple(args, "Is", &runNumber, &title)) {
        return NULL;                  // Exception has already been raised.
    }
    
    pReadoutStatisticsData pThis = reinterpret_cast<pReadoutStatisticsData>(self);
    try {
        pThis->m_pObject->beginRun(runNumber, std::string(title));
    }
    catch (std::exception& e) {
        PyErr_SetString(exception, e.what());
        return NULL;
    }
    catch (...) {
        PyErr_SetString(exception, "Unexpected C++ exception type caught");
        return NULL;
    }
    Py_RETURN_NONE;
}

/**
 * readoutstatistics_emitStatistics
 *    Emits the statistics concerning triggers, events and bytes from a readout.
 *
 *  @param self - Pointer to the object's storage.
 *  @param args - Tuple of parameters, in order:
 *                - triggers, the number of times the code responded to triggers.
 *                - events, the number of events actually read.  Could be more or
 *                  less than triggers - events can be rejected and triggers could,
 *                  in an appropriate framework, result in more than one event.
 *                - bytes - number of bytes that have been read.
 *  @return PyObject* - Py_None.
 */
static PyObject*
readoutstatistics_emitStatistics(PyObject* self, PyObject* args)
{
    uint64_t triggers;
    uint64_t events;
    uint64_t bytes;
    static_assert(
        sizeof(uint64_t) == sizeof(unsigned long),
        "k conversion won't work on this platform"
    );
    if (!PyArg_ParseTuple(args, "kkk", &triggers, &events, &bytes)) {
        return NULL;
    }
    pReadoutStatisticsData pThis = reinterpret_cast<pReadoutStatisticsData>(self);
    try {
        pThis->m_pObject->emitStatistics(triggers, events, bytes);
    }
    catch (std::exception& e) {
        PyErr_SetString(exception, e.what());
        return NULL;
    }
    catch (...) {
        PyErr_SetString(exception, "Unexpected C++ exception type caught");
        return NULL;
    }
    Py_RETURN_NONE;
    
}
/**
 *  tables that define the class and its methods:
 */

static PyMethodDef readoutStatistics_Methods[] = {
    {"beginRun", readoutstatistics_beginRun, METH_VARARGS,
     "Log the run information (run number and title"},
    {"emitStatistics", readoutstatistics_emitStatistics, METH_VARARGS,
    "Emit readout statistics"},
    {NULL, NULL, 0, NULL}
};
static PyTypeObject readoutstatistics_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "statusmessages.readoutStatistics",       /*tp_name*/
    sizeof(readoutStatisticsData), /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)(readoutstatistics_delete), /*tp_dealloc*/
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
    "Encapsulation of CreadoutStatisticsMessage class.", /* tp_doc */
    0,                         /* tp_traverse */
    0,                         /* tp_clear */
    0,                         /* tp_richcompare */
    0,                         /* tp_weaklistoffset */
    0,                         /* tp_iter */
    0,                         /* tp_iternext */
    readoutStatistics_Methods,           /* tp_methods */
    0,                         /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)readoutstatistics_init,      /* tp_init */
    0,                         /* tp_alloc */
    readoutstatistics_new,                 /* tp_new */
};
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
        // The socket we create is normally pushing to a pull server which
        // aggregates to a single pub interface.   In test mode however
        // we want to test our ability to subscribe so the socket we create is
        // a pub which binds to the URI:
        
        if (testMode) {
            pThis->m_pSocket = new zmq::socket_t(*pContext, ZMQ_PUB);
            pThis->m_pSocket->bind(uri);
        } else {
            pThis->m_pSocket = new zmq::socket_t(*pContext, ZMQ_PUSH);
            pThis->m_pSocket->connect(uri);    
        }
        
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
/**
 * ringstatistics_addConsumer
 *    Add a consumer record to the message.
 *
 *  @param self - pointer to our storage.
 *  @param args - Pointer to the positional parameters which must have:
 *                - iterable containing the command words.
 *                - number of operations (gets) performed.
 *                - Numer of bytes gotten.
 *  @return PyObject* - Py_None.
 */
PyObject*
ringstatistics_addConsumer(PyObject* self, PyObject* args)
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
        pThis->m_pObject->addConsumer(cmdWords, ops, bytes);
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
    {"addConsumer", ringstatistics_addConsumer, METH_VARARGS,
      "Add a consumer record to the message"},
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
    // Add constants from the CStatusDefinitions::MessageTypes:
    
    PyObject* MessageTypes = Py_InitModule3(
        "statusmessages.MessageTypes", ModuleMethods,  // Must change if
        "Message type code"                            // outer modules get methods
    );
    if (MessageTypes == nullptr) {
        return;
    }
    Py_INCREF(MessageTypes);
    PyModule_AddObject(module, "MessageTypes", MessageTypes);

    // Message types:
    
    PyModule_AddIntConstant(
        MessageTypes, "RING_STATISTICS",
        CStatusDefinitions::MessageTypes::RING_STATISTICS
    );
    PyModule_AddIntConstant(
        MessageTypes, "EVENT_BUILDER_STATISTICS",
        CStatusDefinitions::MessageTypes::EVENT_BUILDER_STATISTICS
    );
    PyModule_AddIntConstant(
        MessageTypes, "READOUT_STATISTICS",
        CStatusDefinitions::MessageTypes::READOUT_STATISTICS
    );
    PyModule_AddIntConstant(
        MessageTypes, "LOG_MESSAGE",
        CStatusDefinitions::MessageTypes::LOG_MESSAGE
    );
    PyModule_AddIntConstant(
        MessageTypes, "STATE_CHANGE",
        CStatusDefinitions::MessageTypes::STATE_CHANGE
    );
    PyModule_AddIntConstant(
        MessageTypes, "FIRST_FREE_TYPE",
        CStatusDefinitions::MessageTypes::FIRST_FREE_TYPE
    );
    PyModule_AddIntConstant(
        MessageTypes, "FIRST_USER_TYPE",
        CStatusDefinitions::MessageTypes::FIRST_USER_TYPE
    );
    // The constants from CStatusDefinitions::SeverityLevels:
    
    PyObject* SeverityLevels = Py_InitModule3(
        "statusmessages.SeverityLevels", ModuleMethods,
        "Message severity codes"
    );
    if (SeverityLevels == nullptr) {
        return;
    }
    Py_INCREF(SeverityLevels);
    PyModule_AddObject(module, "SeverityLevels", SeverityLevels);
    
    PyModule_AddIntConstant(
        SeverityLevels, "DEBUG", CStatusDefinitions::SeverityLevels::DEBUG
    );
    PyModule_AddIntConstant(
        SeverityLevels, "INFO", CStatusDefinitions::SeverityLevels::INFO
    );
    PyModule_AddIntConstant(
        SeverityLevels, "WARNING", CStatusDefinitions::SeverityLevels::WARNING
    );
    PyModule_AddIntConstant(
        SeverityLevels, "SEVERE", CStatusDefinitions::SeverityLevels::SEVERE
    );
    PyModule_AddIntConstant(
        SeverityLevels, "DEFECT", CStatusDefinitions::SeverityLevels::DEFECT
    );
    
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
        module, "RingStatistics",
        reinterpret_cast<PyObject*>(&ringstatistics_Type)
    );
    
    // Add the ReadoutStatistics class:
    
    if (PyType_Ready(&readoutstatistics_Type) < 0) {
        return;
    }
    Py_INCREF(&readoutstatistics_Type);
    PyModule_AddObject(
        module, "ReadoutStatistics",
        reinterpret_cast<PyObject*>(&readoutstatistics_Type)
    );
    // add the LogMessage class:
    
    if (PyType_Ready(&logmessage_Type) < 0) {
        return;
    }
    Py_INCREF(&logmessage_Type);
    PyModule_AddObject(
        module, "LogMessage",
        reinterpret_cast<PyObject*>(&logmessage_Type)
    );
    // Add the StateChange class:
    
    if (PyType_Ready(&statechange_Type) < 0) {
        return;
    }
    Py_INCREF(&statechange_Type);
    PyModule_AddObject(
        module, "StateChange",
        reinterpret_cast<PyObject*>(&statechange_Type)
    );
}
