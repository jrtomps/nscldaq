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
# @file   PyRingMaster
# @brief  Provide bindings to the CRingMaster Class that might be useful to python.
# @author <fox@nscl.msu.edu>
*/

#include <Python.h>
#include <CRingMaster.h>
#include <CConnectivity.h>
#include <Exception.h>
#include <TCLInterpreter.h>
#include <TCLObject.h>
#include <string>
#include <vector>
#include <stdexcept>


static PyObject* exception;             // For exceptions raised by us.
void* gpTCLApplication(0);   // libtcl++ may need this.


typedef struct {
    PyObject_HEAD
    
    CRingMaster* m_pRingMaster;
} RingMasterObject;

typedef struct {
    PyObject_HEAD
    
    CConnectivity* m_pConnectivity;
} ConnectivityObject;

// class methods:

static PyMethodDef RingMasterClassMethods[] {
    {NULL, NULL, 0, NULL}    
};

/**--------------------------------------------------------------------------
 * utilities:
 */

/**
 * getRingMaster
 *    Given an object pointer, return the ring master from the object.
 *
 *   @param self - pointer to the object.
 *   @return CRingMaster*
 */
static CRingMaster*
getRingMaster(PyObject* self)
{
    RingMasterObject* pSelf = reinterpret_cast<RingMasterObject*>(self);
    return pSelf->m_pRingMaster;
}
/**
 * describeConsumers
 *   Returns a tuple containing the dicts that describe ring consumers.
 *
 *   @param interp - interpreter used to pull apart the lists.
 *   @param cinfo  - Basic consumer information list (pid, backlog).
 *   @param cstats - Consumer statistics (pid, ngets, bytes).
 *
 *   @return PyObject* - tuple of consumer dicts.
 */
static PyObject*
describeConsumers(CTCLInterpreter& interp, CTCLObject& cinfo, CTCLObject& cstats)
{
    PyObject* result = PyTuple_New(cinfo.llength());
    // Note that cinfo and cstats are in the same order and have the same length.
    
    for (auto i = 0; i < cinfo.llength(); i++) {
        PyObject* consumer = PyDict_New();
        CTCLObject info    = cinfo.lindex(i);      // PID and backlog.
        info.Bind(interp);
        CTCLObject stats   = cstats.lindex(i);      // gets and bytes.
        stats.Bind(interp);
        PyDict_SetItemString(consumer, "pid", PyInt_FromLong((int)(info.lindex(0))));
        PyDict_SetItemString(consumer, "backlog", PyInt_FromLong((int)(info.lindex(1))));
        PyDict_SetItemString(consumer, "gets", PyFloat_FromDouble((double)(stats.lindex(1))));
        PyDict_SetItemString(consumer, "bytes", PyFloat_FromDouble((double)(stats.lindex(2))));
        
        PyTuple_SetItem(result, i, consumer);
    }
    return result;
}
/**
 * addRingDescription
 *    Adds the dict that describes a ring to the overall dict that describes
 *    all rings.
 *            -  size  - ring buffer size in bytes.
 *            -  free  - ring buffer free space in bytes.
 *            -  maxconsumers - Maximum consumers the ring can have.
 *            -  producer - PID of the producer.
 *            -  maxavail - Maximum  amount of space available by all consumers.
 *            -  minavail - Minimum amount of space avaialble by all consumers.
 *            -  puts     - Number of puts done by the producer.
 *            -  bytes    - Number of bytes put by the producer.
 *            - consumers - Contains a tuple whose elements are maps contining:
 *                 - pid     - Pid of the consumer.
 *                 - backlog - Number of bytes in the backlog.
 *                 - gets    - Number of gets done by this consumer.
 *                 - bytes   - Number of bytes gotten by this consumer.
 *    @param result - The result dict we're assembling.
 *    @param interp - references the interpreter we're using to do the parse.
 *    @param ring   - 2 element list describing the ring.
 */
static void
addRingDescription(PyObject* result, CTCLInterpreter& interp, CTCLObject& ring)
{
    std::string ringname    = ring.lindex(0);
    CTCLObject  description = ring.lindex(1);  description.Bind(interp);
    
    // Pull out the chunks of the description:
    
    int size         = description.lindex(0);
    int nFree        = description.lindex(1);
    int maxConsumers = description.lindex(2);
    int producer     = description.lindex(3);
    int maxavail     = description.lindex(4);
    int minavail     = description.lindex(5);
    CTCLObject consumers = description.lindex(6); consumers.Bind(interp);   
    CTCLObject pstats    = description.lindex(7); pstats.Bind(interp);
    double     nputs      = pstats.lindex(0);
    double     pbytes    = pstats.lindex(1);
    CTCLObject cstats    = description.lindex(8); cstats.Bind(interp);
    
    // Fill in the dict for the ring:
    
    PyObject* ringDict = PyDict_New();
    PyDict_SetItemString(ringDict, "size", PyInt_FromLong(size));
    PyDict_SetItemString(ringDict, "free", PyInt_FromLong(nFree));
    PyDict_SetItemString(ringDict, "maxconsumers", PyInt_FromLong(maxConsumers));
    PyDict_SetItemString(ringDict, "producer",     PyInt_FromLong(producer));
    PyDict_SetItemString(ringDict, "maxavail",  PyInt_FromLong(maxavail));
    PyDict_SetItemString(ringDict, "minavail",  PyInt_FromLong(minavail));
    PyDict_SetItemString(ringDict, "puts",  PyFloat_FromDouble(nputs));
    PyDict_SetItemString(ringDict, "bytes", PyFloat_FromDouble(pbytes));
    

    PyDict_SetItemString(ringDict, "consumers", describeConsumers(interp, consumers, cstats));
    
    
    PyDict_SetItemString(result, ringname.c_str(), ringDict);
}
/**
 *  usageToDict
 *     Given the usage string, converts it to the dict that is returned by
 *     ringmaster_usage (see that function for a description).
 *
 *  @param usageString - std::string from CRingMaster::requestUsage()
 *  @return PyObject*  - Pointer to the dict created.
 *  @note - this is a bit weird because the simplest way to parse the lists
 *          in requestUsage is to ask Tcl to do it for us.
 */
static PyObject*
usageToDict(std::string usageString)
{
    CTCLInterpreter interp;                         // Captive interpreter.
    CTCLObject      usage;
    usage.Bind(interp);
    usage = usageString;
    
    PyObject* result = PyDict_New();
    
    // usage is a list, one per ring:
    
    for (int i = 0; i < usage.llength(); i++) {
        CTCLObject ringInfo = usage.lindex(i);
        ringInfo.Bind(interp);
        addRingDescription(result, interp, ringInfo);
    }
    
    return result;
}
/**
 * describeClient
 *   Returns the description of a single client command string.
 *
 * @param command - A single client's command words.
 * @return PyObject* - A tuple containing the command words.
 */
static PyObject*
describeClient(CRingMaster::commandWords& command)
{
    PyObject* result = PyTuple_New(command.size());
    for (auto i = 0; i < command.size(); i++) {
        PyTuple_SetItem(result, i, PyString_FromString(command[i].c_str()) );
    }
    return result;
}
/**
 * describeClients
 *    Describes the clients in a Python object.  See ringmaster_clients
 *    for information about what's returned.
 *
 *  @param clients - information from CRingMaster::clients.
 *  @return PyObject* - Dict as described by ringmaster_clients.
 */
static PyObject*
describeClients(CRingMaster::ClientCommands& info)
{
    PyObject* result = PyDict_New();
    
    // Producer is simple.
    
    PyDict_SetItemString(result, "producer", describeClient(info.s_producer));
    
    // Consumer is a tuple of tuples.
    
    PyObject* consumers = PyTuple_New(info.s_consumers.size());
    for (auto i = 0; i < info.s_consumers.size(); i++) {
        PyTuple_SetItem(consumers, i, describeClient(info.s_consumers[i]));
    }
    
    PyDict_SetItemString(result, "consumers", consumers);
    return result;
    
}

/**
 * getConnectivity
 *    Return the connectivity object pointer for a python connectivity object.
 *
 *  @param self - PyObject* that really points to a ConnectivityObject struct.
 *  @return CConnectivity
 */
static CConnectivity*
getConnectivity(PyObject* self)
{
    ConnectivityObject* c = reinterpret_cast<ConnectivityObject*>(self);
    return c->m_pConnectivity;
}
/**
 * stringVectorToTuple
 *    Converts a string vector into a Python Tuple containing one element
 *    of the vector in each tuple element.
 *
 *   @param vec - the string vector.
 *   @return PyObject* - the tuple.
 */
static PyObject*
stringVectorToTuple(const std::vector<std::string>& vec)
{
    PyObject* result = PyTuple_New(vec.size());
    
    for (size_t i = 0; i < vec.size(); i++) {
        PyTuple_SetItem(result, i, PyString_FromString(vec[i].c_str()));
    }
    return result;
}
/**--------------------------------------------------------------------------
 * Object cannonicals
 */

/**
 * RingMaster_new
 *    Allocates storage for the ringmaster object.  This does no initialization.
 *
 *  @param type - pointer to the type object.
 *  @param args - Positional parameters.
 *  @param kwargs - keyword parameters
 *  @return PyObject* - pointer to the object's storage.
 */
static PyObject*
RingMaster_new(PyTypeObject* type, PyObject* args, PyObject* kwargs)
{
    // Allocate the object:
    
        RingMasterObject* self = reinterpret_cast<RingMasterObject*>(
            type->tp_alloc(type, 0)
        );
        if (self) {
            self->m_pRingMaster = 0;
            return reinterpret_cast<PyObject*>(self);
        } else {
            PyErr_SetString(exception, "Unable to allocate RingMaster object");
            return NULL;
        }
}

/**
 * RingMaster_init
 *    The __init__ method for the ring master object.
 *    This requires a host to which the ring master object proxy will be
 *    connected.
 *
 *  @param self   - pointer to the object being initialized.
 *  @param args   - Positional parameters - must contain a host string.
 *  @param kwds   - Keyword parameters.
 *  @return int   - -1 for failure.
 */
static int
RingMaster_init(RingMasterObject* self, PyObject* args, PyObject* kwargs)
{
    const char* pHost;
    if (! PyArg_ParseTuple(args, "s", &pHost)) {
        return -1;                  // Parsetuple raises its own exception.
    }
    
    // remove any existing CRingMaster object:
    
    delete self->m_pRingMaster;
    try {
        self->m_pRingMaster = new CRingMaster(pHost);
    }
    catch(CException& e) {
        PyErr_SetString(exception, e.ReasonText());
        return -1;
    }
    catch (std::string msg) {
        PyErr_SetString(exception, msg.c_str());
        return -1;
    }
    catch (const char* msg) {
        PyErr_SetString(exception, msg);
        return -1;
    }
    catch (...) {
        PyErr_SetString(exception, "Unanticipated exception type");
        return -1;
    }
    
    return 0;                        // Success.
}

/**
 * RingMaster_delete
 *    Destroys a ringmaster object.
 *
 *  @param self - pointer to the object storage.
 */
static void
RingMaster_delete (RingMasterObject* self)
{
    delete self->m_pRingMaster;
    self->ob_type->tp_free(reinterpret_cast<PyObject*>(self));
}


/*-----------------------------------------------------------------------------
 * Public object methods for the RingMaster type:
 */

/**
 * ringmaster_usage
 *    Return usage information for all rings in the ringbuffer.
 *  
 *   @param self  - Pointer to the ringbuffer object.
 *   @param args  - Positional arguments.
 *
 *  @return - returns a PyObject* that is really a dict. containing the
 *            one key for each ringbuffer (the ringname).  Each of those
 *            key contains another dict that has the following keys:
 *            -  size  - ring buffer size in bytes.
 *            -  free  - ring buffer free space in bytes.
 *            -  maxconsumers - Maximum consumers the ring can have.
 *            -  producer - PID of the producer.
 *            -  maxavail - Maximum  amount of space available by all consumers.
 *            -  minavail - Minimum amount of space avaialble by all consumers.
 *            -  puts     - Number of puts done by the producer.
 *            -  bytes    - Number of bytes put by the producer.
 *            - consumers - Contains a tuple whose elements are maps contining:
 *                 - pid     - Pid of the consumer.
 *                 - backlog - Number of bytes in the backlog.
 *                 - gets    - Number of gets done by this consumer.
 *                 - bytes   - Number of bytes gotten by this consumer.
 */
static PyObject*
ringmaster_usage(PyObject* self, PyObject* args)
{
    // should not have any args:
    
    if (PyTuple_Size(args) != 0) {
        PyErr_SetString(exception, "the usage method does not take parameters");
        return NULL;
    }
    
    // Get our CRingMaster Object:
    
    
    CRingMaster* rmaster = getRingMaster(self);
    try {
        std::string info = rmaster->requestUsage();
        return usageToDict(info);
        
    }
    catch (CException& e) {
        PyErr_SetString(exception, e.ReasonText());
        return NULL;
    }
    catch (std::string msg) {
        PyErr_SetString(exception, msg.c_str());
        return NULL;
    }
    catch (const char* msg) {
        PyErr_SetString(exception, msg);
        return NULL;
    }
    catch (...) {
        PyErr_SetString(exception, "Unanticipated C++ exception type");
        return NULL;
    }
    
}
/**
 * ringmaster_clients
 *    Returns an object that describes the clients of a ringbuffer.
 *
 *   @param self - pointer to the object on which this method is being invoked.
 *   @param args - The positional arguments.  There must be exactly 1, the name
 *                  of the ringbuffer.
 *   @return PyObject*  - a Dict with the following keys:
 *              - 'producer' - Producer specification.
 *              - 'consumers' - tuple of consumer specifications.
 *              Each specification (producer, or consumer) is a tuple of command
 *              words that describe the full command that is the client.
 */
static PyObject*
ringmaster_clients(PyObject* self, PyObject* args)
{
    const char* ringName;
    if (!PyArg_ParseTuple(args, "s", &ringName)) {
        PyErr_SetString(exception, "ringmaster.client method requires a ringname");
        return NULL;
    }
    // Get our ring master object:
    
    CRingMaster* rMaster = getRingMaster(self);
    
    // The remainder is done in a try/catch block to turn C++ exceptions into
    // Python exceptions:
    
    try {
        
        CRingMaster::ClientCommands clients =rMaster->listClients(std::string(ringName));
        return describeClients(clients);
    }
    catch(CException& e) {
        PyErr_SetString(exception, e.ReasonText());
        return NULL;
    }
    catch(std::exception& e) {
        PyErr_SetString(exception, e.what());
        return NULL;
    }
    catch (std::string msg) {
        PyErr_SetString(exception, msg.c_str());
        return NULL;
    }
    catch (const char* msg) {
        PyErr_SetString(exception, msg);
        return NULL;
    }
    catch (...) {
        PyErr_SetString(exception, "Unanticipated C++ exception type");
        return NULL;
    }
    // Should not get here:
    
    Py_RETURN_NONE;
}

// Type Dispatch table for the ringmaster type:

static PyMethodDef RingMasterObjectMethods [] {
    {"usage", ringmaster_usage, METH_VARARGS,
        "Obtain ring buffer usage"
    },
    {"clients", ringmaster_clients, METH_VARARGS,
        "List the clients of a ringbuffer."
    },
    {NULL, NULL, 0, NULL}
};
// Type table.

static PyTypeObject RingMasterType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "ringmaster.ringmaster",             /*tp_name*/
    sizeof(RingMasterObject), /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)(RingMaster_delete), /*tp_dealloc*/
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
    "RingMaster object", /* tp_doc */
    0,		               /* tp_traverse */
    0,		               /* tp_clear */
    0,		               /* tp_richcompare */
    0,		               /* tp_weaklistoffset */
    0,		               /* tp_iter */
    0,		               /* tp_iternext */
    RingMasterObjectMethods,        /* tp_methods */
    0,                         /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)RingMaster_init,      /* tp_init */
    0,                         /* tp_alloc */
    RingMaster_new,                 /* tp_new */
    
};

/*---------------------------------------------------------------------------
 * Canonical methods for the connectivity type.
 */
/**
 * Connectivity_new
 *    Allocates storage for the connectivity object.  This does no initialization.
 *
 *  @param type - pointer to the type object.
 *  @param args - Positional parameters.
 *  @param kwargs - keyword parameters
 *  @return PyObject* - pointer to the object's storage.
 */
static PyObject*
Connectivity_new(PyTypeObject* type, PyObject* args, PyObject* kwargs)
{
    // Allocate the object:
    
        ConnectivityObject* self = reinterpret_cast<ConnectivityObject*>(
            type->tp_alloc(type, 0)
        );
        if (self) {
            self->m_pConnectivity = 0;
            return reinterpret_cast<PyObject*>(self);
        } else {
            PyErr_SetString(exception, "Unable to allocate Connectivity object");
            return NULL;
        }
}


/**
 * RingMaster_init
 *    The __init__ method for the connectivity object.
 *    This requires a host to which the connectivity object proxy will be
 *    connected.
 *
 *  @param self   - pointer to the object being initialized.
 *  @param args   - Positional parameters - must contain a host string.
 *  @param kwds   - Keyword parameters.
 *  @return int   - -1 for failure.
 */
static int
Connectivity_init(ConnectivityObject* self, PyObject* args, PyObject* kwargs)
{
    const char* pHost;
    if (! PyArg_ParseTuple(args, "s", &pHost)) {
        return -1;                  // Parsetuple raises its own exception.
    }
    
    // remove any existing CRingMaster object:
    
    delete self->m_pConnectivity;
    try {
        self->m_pConnectivity = new CConnectivity(pHost);
    }
    catch(CException& e) {
        PyErr_SetString(exception, e.ReasonText());
        return -1;
    }
    catch (std::string msg) {
        PyErr_SetString(exception, msg.c_str());
        return -1;
    }
    catch (const char* msg) {
        PyErr_SetString(exception, msg);
        return -1;
    }
    catch (...) {
        PyErr_SetString(exception, "Unanticipated exception type");
        return -1;
    }
    
    return 0;                        // Success.
}

/**
 * Connectivity_delete
 *    Destroys a connectivity object.
 *
 *  @param self - pointer to the object storage.
 */
static void
Connectivity_delete (ConnectivityObject* self)
{
    delete self->m_pConnectivity;
    self->ob_type->tp_free(reinterpret_cast<PyObject*>(self));
}

/*----------------------------------------------------------------------------
 *  public methods of the connectibity object:
 */

/**
 * connectivity_producers
 *    Returns a tuple (possibly empty) containing the set of hosts that are
 *    producing into proxy rings in the host represented by the connectivity
 *    object.
 *
 *  @param self  - Pointer to the object storage (ConnectivityObject*).
 *  @param args  - Positional parameters (should not be any).
 *  @return PyObject* Tuple we created.
 */
static PyObject*
connectivity_producers(PyObject* self, PyObject* args)
{
    // There can be no parameters:
    
    if(PyTuple_Size(args) > 0) {
        PyErr_SetString(exception, "producers method takes no parameters");
        return NULL;
    }
    
    CConnectivity* pConn = getConnectivity(self);
    try {
        std::vector<std::string> producers = pConn->getProducers();
        return stringVectorToTuple(producers);
    }
    catch (CException& e) {
        PyErr_SetString(exception, e.ReasonText());
        return NULL;
    }
    catch (std::exception& e) {
        PyErr_SetString(exception, e.what());
        return NULL;
    }
    catch (std::string msg) {
        PyErr_SetString(exception, msg.c_str());
        return NULL;
    }
    catch (const char* msg) {
        PyErr_SetString(exception, msg);
        return NULL;
    }
    catch (...) {
        PyErr_SetString(exception, "Unanticipated C++ exception type caught");
        return NULL;
    }
    
    return NULL;                      // Should not get here.
}
/**
 * connectivity_consumers
 *   Return a (possibly empty ) tuple containing the hosts that consume data
 *   from our rings.
 *
 * @param self   - Pointer to our object storage.
 * @param args   - Positional argument tuple -- must be empty.
 * @return PyObject* - actually a tuple.
 */

static PyObject*
connectivity_consumers(PyObject* self, PyObject* args)
{
    // Must not have params:
    
    if (PyTuple_Size(args) > 0) {
        PyErr_SetString(exception, "Method has no parameters");
        return NULL;
    }
    
    CConnectivity* pConn = getConnectivity(self);
    try {
        std::vector<std::string> consumers = pConn->getConsumers();
        return stringVectorToTuple(consumers);
    }
    catch (CException& e) {
        PyErr_SetString(exception, e.ReasonText());
        return NULL;
    }
    catch (std::exception& e) {
        PyErr_SetString(exception, e.what());
        return NULL;
    }
    catch (std::string msg) {
        PyErr_SetString(exception, msg.c_str());
        return NULL;
    }
    catch (const char* msg) {
        PyErr_SetString(exception, msg);
        return NULL;
    }
    catch (...) {
        PyErr_SetString(exception, "Unanticipated C++ exception type caught");
        return NULL;
    }
    // Should not get here:
    
    return NULL;
}
/**
 * connectivity_participants
 *  Returns a tuple containing the fully qualified domain names of all hosts
 *  that participate in the dataflow givne a single host that participates.
 *
 *  @param self  -  Pointer to the object struct for the object being invoked.
 *  @param args  -  Pointer to a tuple containing the method's positional parameters.
 *                  There must not be any parameters.  The host on which the
 *                  object was constructed is the starting point.
 *  @return PyObject* - An  n-tuple that contains at least one element (the
 *                      starting point host),
 */
static PyObject*
connectivity_participants(PyObject* self, PyObject* args)
{
    if (PyTuple_Size(args) > 0) {
        PyErr_SetString(exception, "Method takes no parameters");
        return NULL;
    }
    
    CConnectivity* c = getConnectivity(self);
    try {
        std::vector<std::string> participants = c->getAllParticipants();
        return stringVectorToTuple(participants);
    }
    catch (CException& e) {
        PyErr_SetString(exception, e.ReasonText());
        return NULL;
    }
    catch (std::exception& e) {
        PyErr_SetString(exception, e.what());
        return NULL;
    }
    catch (std::string msg) {
        PyErr_SetString(exception, msg.c_str());
        return NULL;
    }
    catch (const char* msg) {
        PyErr_SetString(exception, msg);
        return NULL;
    }
    catch (...) {
        PyErr_SetString(exception, "An unanticpated C++ exception type was caught.");
        return NULL;
    }
    
    // Control should not pass here:
    
    return NULL;

}
// Type dispatch table for the connectivity type:

static PyMethodDef ConnectivityObjectMethods [] {
    {"producers", connectivity_producers, METH_VARARGS,
        "List remote hosts that produce data into proxy rings"},
    {"consumers", connectivity_consumers, METH_VARARGS,
        "List remote hosts that consume data from our rings"},
    {"allParticipants", connectivity_participants, METH_VARARGS,
        "Lists all hosts involved in the data flow from a starting point"},
    {NULL, NULL, 0, NULL}
};

// Type table.

static PyTypeObject ConnectivityType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "ringmaster.connectivity",             /*tp_name*/
    sizeof(ConnectivityObject), /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)(Connectivity_delete), /*tp_dealloc*/
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
    "Connectivity object", /* tp_doc */
    0,		               /* tp_traverse */
    0,		               /* tp_clear */
    0,		               /* tp_richcompare */
    0,		               /* tp_weaklistoffset */
    0,		               /* tp_iter */
    0,		               /* tp_iternext */
    ConnectivityObjectMethods,        /* tp_methods */
    0,                         /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)Connectivity_init,      /* tp_init */
    0,                         /* tp_alloc */
    Connectivity_new,                 /* tp_new */
    
};


/*----------------------------------------------------------------------------
 *  initringmaster
 *    Module initialization that is searched for by Python.
 */
PyMODINIT_FUNC
initringmaster(void)
{
    PyObject* module;
    
    // Initialize the module:
    
    module = Py_InitModule3(
        "ringmaster", RingMasterClassMethods, "CRingMaster encapsulation"
    );
    if (module == NULL) {
            return;               // Failure but not much we can do to signal it.
    }
    // Register our ringmaster type:
    
    if (PyType_Ready(&RingMasterType) < 0) {
        return;
    }
    Py_INCREF(&RingMasterType);
    PyModule_AddObject(module, "ringmaster", reinterpret_cast<PyObject*>(&RingMasterType));
    
    // Register our connectivity type:
    
    if (PyType_Ready(&ConnectivityType) < 0) {
        return;
    }
    Py_INCREF(&ConnectivityType);
    PyModule_AddObject(module, "connectivity", reinterpret_cast<PyObject*>(&ConnectivityType));
    
    // Add our exception type:
    
    exception = PyErr_NewException(const_cast<char*>("ringmaster.error"), NULL, NULL);
    PyModule_AddObject(module, "error", exception);
}