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
# @file   Pystatemanager.cpp
# @brief  Python bindings to state client.
# @author <fox@nscl.msu.edu>
*/

#include <Python.h>
#include <stdexcept>
#include <string.h>
#include <string>

#include "CStateManager.h"


/* Module specific error: */

static PyObject* error;


/*
 * Api Object storage:
 */
typedef struct {
    PyObject_HEAD
    
    CStateManager*  m_pApi;
} statemanager_ApiObject;


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
static CStateManager*
getApi(PyObject* self)
{
    statemanager_ApiObject* pThis =
        reinterpret_cast<statemanager_ApiObject*>(self);
    return pThis->m_pApi;
}

/**
 * getBool
 *   Convert PyObject -> bool.
 * @param obj - the object.
 * @return bool
 * @throw std::invalid_argument the obj is not a bool.
 */
static bool
getBool(PyObject* obj)
{
    if (!PyBool_Check(obj)) {
        throw std::invalid_argument("Parameter must be a boolean");
    }
    return obj == Py_True ? true : false;
}
/**
 * getString
 *   Convert PyObject -> std::string
 *
 * @param obj - object to convert.
 * @return std::string - converted obj.
 * @throw std::invalid_argument if the obj is not a string like thing.
 */
static std::string
getString(PyObject* obj)
{
    if (!PyString_Check(obj)) {
        throw std::invalid_argument("parameter must be a string");
    }
    return std::string(PyString_AsString(obj));
}

/**
 * incref
 *    Increment an object reference and return it (sure wish
 *    Py_INCREF did that)
 * @param obj
 * @return PyObject - same object with reference count incremented.
 */
static PyObject*
incref(PyObject* obj)
{
    Py_INCREF(obj);
    return obj;
}

/**
 * stringVecToList
 *    Turn an std::vector<std::string> into a Python list obj,
 *    increment its reference and return it.
 *
 * @param vec - the string vector
 * @return PyObject* list with incremented refcount.
 */
static PyObject*
stringVecToList(const std::vector<std::string>& vec)
{
    PyObject* result = PyList_New(vec.size());
    

    
    for (int i = 0; i < vec.size(); i++) {
        PyList_SetItem(result, i, PyString_FromString(vec[i].c_str()));
    }
    
    return incref(result);    
}

/**
 * marshallDefinitionDict
 *   Marshalls a program definition dict into a ProgramDefinition struct.
 *
 *  @param programDef - The definition dict.
 *  @return CStateManager::ProgramDefinition - corresponding definition.
 *  @throw std::invalid_argument - bad dict values or missing mandatories.
 */
CStateManager::ProgramDefinition
marshallDefinitionDict(PyObject* programDef)
{
    CStateManager::ProgramDefinition p = { true, false, "", "", "", ""};
    PyObject* item;
    
    if (item = PyDict_GetItemString(programDef, "enabled")) { 
        p.s_enabled = getBool(item);
    }
    if (item = PyDict_GetItemString(programDef, "standalone")) {
        p.s_standalone = getBool(item);
    }
    if (item = PyDict_GetItemString(programDef, "path")) {
        p.s_path = getString(item);
    }
    if (item = PyDict_GetItemString(programDef,"host")) {
        p.s_host = getString(item);
    }
    if (item = PyDict_GetItemString(programDef, "outring")) {
        p.s_outRing = getString(item);
    }
    if (item = PyDict_GetItemString(programDef, "inring")) {
        p.s_inRing = getString(item);
    }
    // Host and path must not be empty.
    
    if ((p.s_host == "") || (p.s_path == "")) {
        throw std::invalid_argument(
            "'path' and 'host' are required keys to the program definition"
        );
    }
    return p;
}
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
    statemanager_ApiObject* self =
        reinterpret_cast<statemanager_ApiObject*>(type->tp_alloc(type, 0));
    if (self) {
        self->m_pApi = NULL;
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
Api_init(statemanager_ApiObject* self, PyObject* args, PyObject* kwds)
{
    const char* reqUri;
    const char* subUri;
    
    if (!PyArg_ParseTuple(args, "ss", &reqUri, &subUri)) {
        PyErr_SetString(error, "need requri and suburi");
        return -1;
    }
    /* Destroy any pre-existing m_pApi and create a new one. */
    
    delete self->m_pApi;
    try {
        self->m_pApi = new CStateManager(reqUri, subUri);
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
Api_delete(statemanager_ApiObject* self)
{
    delete self->m_pApi;
    self->ob_type->tp_free(reinterpret_cast<PyObject*>(self));
}

/**
* getProgramParentDir
*     Return the name of the directory that holds the state sensitive
*     programs managed by the system.
*
* @param self - Pointer to the object whose method this is.
* @param args - Pointer to the python argument list (empty tuple).
* @return PyObject* - string containing the directory name.
*/
static PyObject*
getProgramParentDir(PyObject* self, PyObject* args)
{
    if (PyTuple_Size(args) != 0) {
        return raise("getProgramParentDir has no parameters");
    }
    
    CStateManager* pApi = getApi(self);
    std::string dir;
    try {
        dir = pApi->getProgramParentDir();
    }
    catch(std::exception& e) {
        return raise(e.what());
    }
    
    PyObject* result = PyString_FromString(dir.c_str());
    return incref(result);

}
/**
* setProgramParentDir
*     Change the directory that holds the programs managed by the system.
*
* @param self - Pointer to the object whose method this is.
* @param args - Pointer to the python argument list (new directory name.).
* @return PyObject* - Py_None
*/
static PyObject*
setProgramParentDir(PyObject* self, PyObject* args)
{
    char* newDir;
    if(!PyArg_ParseTuple(args, "s", &newDir)) {
        return raise("setProgramParentDir needs a directory name parameter");
    }
    
    CStateManager* pApi = getApi(self);
    try {
        pApi->setProgramParentDir(newDir);
    }
    catch (std::exception& e) {
        return raise(e.what());
    }
    
    Py_RETURN_NONE;
}

/**
* addProgram
*     Adds a program to the system.
*
* @param self - Pointer to the object whose method this is.
* @param args - Pointer to the python argument list 
*               contains a program name and a dict with the following keys:
*               - enabled (optional) enable state of program.
*               - standalone (optional) standalone state.
*               - path (mandatory) Path to the program.
*               - host (mandatory) Host on which program runs.
*               - outring (optional) Output ring.
*               - inring (optional) URI of input ring.
* @return PyObject* - Py_None
*/
static PyObject*
addProgram(PyObject* self, PyObject* args)
{
    PyObject* programDef;
    char*    programName;
    if (!PyArg_ParseTuple(args, "sO", &programName, &programDef)) {
        return raise("addProgram requires  a program name and a definition dict");
    }
    if (!PyDict_Check(programDef)) {
        return raise("addProgram program def must be a dict.");
    }
    
    CStateManager* pApi = getApi(self);
    
    // Marshall the dict into the CStateManager::ProgramDefinition.
    
    CStateManager::ProgramDefinition p;
    
    try {
        p  = marshallDefinitionDict(programDef);
        
        // Now we can try to create the program:
        
        pApi->addProgram(programName, &p);

    }
    catch (std::exception& e) {
        return raise(e.what());
    }

    
    Py_RETURN_NONE;
}

/**
* getProgramDefinition
*     Returns a dict that describes a program.
*
* @param self - Pointer to the object whose method this is.
* @param args - Pointer to the python argument list (tuple - program name).
* @return PyObject* - dict which describes a program.  See
*               addProgram for the keys this dict will have.
*/
static PyObject*
getProgramDefinition(PyObject* self, PyObject* args)
{
    char* name;
    
    if (!PyArg_ParseTuple(args, "s", &name)) {
        return raise("getProgramDefinition needs the name of a program (only).");
    }
    
    CStateManager* pApi = getApi(self);
    CStateManager::ProgramDefinition def;
    
    try {
        def = pApi->getProgramDefinition(name);
    }
    catch (std::exception& e) {
        return raise(e.what());
    }
    PyObject* result = PyDict_New();
    
    // Fill in the dict from the struct.
    
    PyDict_SetItemString(
        result, "enabled", def.s_enabled ? Py_True : Py_False
    );
    PyDict_SetItemString(
        result, "standalone", def.s_standalone ? Py_True : Py_False
    );
    PyDict_SetItemString(
        result, "path", PyString_FromString(def.s_path.c_str())
    );
    PyDict_SetItemString(
        result, "host", PyString_FromString(def.s_host.c_str())
    );
    PyDict_SetItemString(
        result, "outring", PyString_FromString(def.s_outRing.c_str())
    );
    PyDict_SetItemString(
        result, "inring", PyString_FromString(def.s_inRing.c_str())
    );
    
    return incref(result);
}

/**
* modifyProgram
*     Modify the defintion of a program.
*
* @param self - Pointer to the object whose method this is.
* @param args - Pointer to the python argument list (tuple).
*               program name, definition dict.
* @return PyObject* - Py_None
*/
static PyObject*
modifyProgram(PyObject* self, PyObject* args)
{
    char* name;
    PyObject* def;
    
    if (!PyArg_ParseTuple(args, "sO", &name, &def)) {
        return raise(
            "modifyProgram needs program name and definition dict"
        );
    }
    
    try {
        CStateManager::ProgramDefinition progDef = marshallDefinitionDict(def);
        CStateManager* pApi = getApi(self);
        pApi->modifyProgram(name, &progDef);
    }
    catch (std::exception& e) {
        return raise(e.what());
    }
    
    Py_RETURN_NONE;
}

/**
* enableProgram
*     Set the enable flag of a program to true.
*
* @param self - Pointer to the object whose method this is.
* @param args - Pointer to the python argument list (tuple); program name.
* @return PyObject* - Py_None.
*/
static PyObject*
enableProgram(PyObject* self, PyObject* args)
{
    char* progName;
    if (!PyArg_ParseTuple(args, "s", &progName)) {
        return raise("enableProgram needs (only) a program name.");
    }
    
    CStateManager* pApi = getApi(self);
    try {
        pApi->enableProgram(progName);
    }
    catch (std::exception& e) {
        return raise(e.what());
    }
    
    Py_RETURN_NONE;
}

/**
* disableProgram
*     Set a program's enable flag to 'false'.
*
* @param self - Pointer to the object whose method this is.
* @param args - Pointer to the python argument list (tuple): Program name.
* @return PyObject* - Py_None
*/
static PyObject*
disableProgram(PyObject* self, PyObject* args)
{
    char* progName;
    if (!PyArg_ParseTuple(args, "s", &progName)) {
        return raise("enableProgram needs (only) a program name.");
    }

    CStateManager* pApi = getApi(self);
    
    try {
        pApi->disableProgram(progName);
    }
    catch (std::exception& e) {
        return raise(e.what());
    }
    
    Py_RETURN_NONE;
}

/**
* isProgramEnabled
*     check the enable state of a program.
*
* @param self - Pointer to the object whose method this is.
* @param args - Pointer to the python argument list (tuple) : program name.
* @return PyObject* - bool - current enable state.
*/
static PyObject*
isProgramEnabled(PyObject* self, PyObject* args)
{
    char* progName;
    
    if (! PyArg_ParseTuple(args, "s", &progName)) {
        return raise("isProgEnabled needs (only) a program name");
    }
    
    CStateManager* pApi = getApi(self);
    bool enabled;
    try {
        enabled = pApi->isProgramEnabled(progName);
        
    }
    catch (std::exception& e) {
        return raise(e.what());
    }
    
    if (enabled) {
        Py_RETURN_TRUE;
    } else {
        Py_RETURN_FALSE;
    }
}

/**
* setProgramStandalone
*     Set the standalone flag of a program.
*
* @param self - Pointer to the object whose method this is.
* @param args - Pointer to the python argument list (tuple): program name.
* @return PyObject* - Py_None
*/
static PyObject*
setProgramStandalone(PyObject* self, PyObject* args)
{
    char* progName;
    
    if (! PyArg_ParseTuple(args, "s", &progName)) {
        return raise("setProgramNoStandalone needs (only) a program name");
    }
    
    CStateManager* pApi = getApi(self);

    try {
        pApi->setProgramStandalone(progName);
        
    } catch (std::exception& e) {
        return raise(e.what());
    }
    
    Py_RETURN_NONE;    
}

/**
* setProgramNoStandalone
*     turn off the standalone flag of a program.
*
* @param self - Pointer to the object whose method this is.
* @param args - Pointer to the python argument list (tuple) : program name.
* @return PyObject* - Py_None.
*/
static PyObject*
setProgramNoStandalone(PyObject* self, PyObject* args)
{
    char* progName;
    
    if (! PyArg_ParseTuple(args, "s", &progName)) {
        return raise("setProgramNoStandalone needs (only) a program name");
    }
    
    CStateManager* pApi = getApi(self);

    try {
        pApi->setProgramNoStandalone(progName);
        
    } catch (std::exception& e) {
        return raise(e.what());
    }
    
    Py_RETURN_NONE;    

}

/**
* isProgramStandalone
*     Check program's standalone flag.
*
* @param self - Pointer to the object whose method this is.
* @param args - Pointer to the python argument list (tuple) : Program name.
* @return PyObject* - PyBoolean object; state of that flag.
*/
static PyObject*
isProgramStandalone(PyObject* self, PyObject* args)
{
    char* progName;
    
    if (! PyArg_ParseTuple(args, "s", &progName)) {
        return raise("isProgramStandalone needs (only) a program name");
    }
    
    CStateManager* pApi = getApi(self);
    bool enabled;
    try {
        enabled = pApi->isProgramStandalone(progName);
        
    }
    catch (std::exception& e) {
        return raise(e.what());
    }
    
    if (enabled) {
        Py_RETURN_TRUE;
    } else {
        Py_RETURN_FALSE;
    }

}

/**
* listPrograms
*     List the programs known to the system.
*
* @param self - Pointer to the object whose method this is.
* @param args - Pointer to the python argument list (tuple) : empty
* @return PyObject* - list sorted by program name containing the names of
*                      the programs
*/
static PyObject*
listPrograms(PyObject* self, PyObject* args)
{
    if (PyTuple_Size(args) > 0) {
        return raise("listPrograms  does not have any parameters");
    }
    
    CStateManager* pApi = getApi(self);
    std::vector<std::string> programs;
    try {
        programs = pApi->listPrograms();
    }
    catch (std::exception& e) {
        return raise(e.what());
    }

    return stringVecToList(programs);

}

/**
* listEnabledPrograms
*     List all programs with the enable flag set
*
* @param self - Pointer to the object whose method this is.
* @param args - Pointer to the python argument list (tuple).
* @return PyObject* - [description]
*/
static PyObject*
listEnabledPrograms(PyObject* self, PyObject* args)
{
    if (PyTuple_Size(args) > 0) {
        return raise("listPrograms  does not have any parameters");
    }
    
    CStateManager* pApi = getApi(self);
    std::vector<std::string> programs;
    try {
        programs = pApi->listEnabledPrograms();
    }
    catch (std::exception& e) {
        return raise(e.what());
    }

    return stringVecToList(programs);
}

/**
* listStandalonePrograms
*     Lists programs that are set standalone
*
* @param self - Pointer to the object whose method this is.
* @param args - Pointer to the python argument list (tuple) : empty
* @return PyObject* - List of standalone programs
*/
static PyObject*
listStandalonePrograms(PyObject* self, PyObject* args)
{
    if (PyTuple_Size(args) > 0) {
        return raise("listPrograms  does not have any parameters");
    }
    
    CStateManager* pApi = getApi(self);
    std::vector<std::string> programs;
    try {
        programs = pApi->listStandalonePrograms();
    }
    catch (std::exception& e) {
        return raise(e.what());
    }

    return stringVecToList(programs);

}

/**
* listInactivePrograms
*     List programs that don't participate in global state transitions.
*     These are those that are either disabled or standalone.
*
* @param self - Pointer to the object whose method this is.
* @param args - Pointer to the python argument list (tuple) : empty
* @return PyObject* - list of inactive programs.
*/
static PyObject*
listInactivePrograms(PyObject* self, PyObject* args)
{
    if (PyTuple_Size(args) > 0) {
        return raise("listPrograms  does not have any parameters");
    }
    
    CStateManager* pApi = getApi(self);
    std::vector<std::string> programs;
    try {
        programs = pApi->listInactivePrograms();
    }
    catch (std::exception& e) {
        return raise(e.what());
    }

    return stringVecToList(programs);

}
/**
* listActivePrograms
*     Return list of programs that are active.
*
* @param self - Pointer to the object whose method this is.
* @param args - Pointer to the python argument list (tuple):Empty
* @return PyObject* - List of active programs.
*/
static PyObject*
listActivePrograms(PyObject* self, PyObject* args)
{
    if (PyTuple_Size(args) > 0) {
        return raise("listPrograms  does not have any parameters");
    }
    
    CStateManager* pApi = getApi(self);
    std::vector<std::string> programs;
    try {
        programs = pApi->listActivePrograms();
    }
    catch (std::exception& e) {
        return raise(e.what());
    }

    return stringVecToList(programs);
}

/**
* deleteProgram
*     Remove a program definition.
*
* @param self - Pointer to the object whose method this is.
* @param args - Pointer to the python argument list (tuple) : program name.
* @return PyObject* - Py_None
*/
static PyObject*
deleteProgram(PyObject* self, PyObject* args)
{
    char* programName;
    
    if (!PyArg_ParseTuple(args, "s", &programName)) {
        return raise("deleteProgram needs a program name (only)");
    }
    
    CStateManager* pApi = getApi(self);
    
    try {
        pApi->deleteProgram(programName);
    }
    catch (std::exception& e) {
        return raise(e.what());
    }
    
    Py_RETURN_NONE;
}

/**
* setGlobalState
*     Force a state transition.
*
* @param self - Pointer to the object whose method this is.
* @param args - Pointer to the python argument list (tuple) : New state.
* @return PyObject* - Py_None
*/
static PyObject*
setGlobalState(PyObject* self, PyObject* args)
{
    char* newState;
    
    if (!PyArg_ParseTuple(args, "s", &newState)) {
        return raise("setGlobalState requires a new state (only).");
    }
    
    CStateManager* pApi = getApi(self);
    
    try {
        pApi->setGlobalState(newState);
    }
    catch(std::exception& e) {
        return raise(e.what());
    }
    
    Py_RETURN_NONE;
}

/* Api definitions:  */

static PyMethodDef ApiObjectMethods[] = {
    {"getProgramParentDir", getProgramParentDir, METH_VARARGS,
        "Name of directory holding programs"},
    {"setProgramParentDir", setProgramParentDir, METH_VARARGS,
        "Set a new program parent directory"},
    {"addProgram", addProgram, METH_VARARGS, "Add a new program"},
    {"getProgramDefinition", getProgramDefinition, METH_VARARGS,
        "Get a program's definition" },
    {"modifyProgram", modifyProgram, METH_VARARGS, "Modify a program definition"},
    {"enableProgram", enableProgram, METH_VARARGS, "Enable a program"},
    {"disableProgram", disableProgram, METH_VARARGS, "Disable a program"},
    {"isProgramEnabled", isProgramEnabled, METH_VARARGS,
        "Check enable state of program"},
    {"setProgramStandalone", setProgramStandalone, METH_VARARGS,
        "Set program stand alone flag"},
    {"setProgramNoStandalone", setProgramNoStandalone, METH_VARARGS,
        "Clear program standalone flag"},
    {"isProgramStandalone", isProgramStandalone, METH_VARARGS,
        "Check program standalone flag"},
    {"listPrograms", listPrograms, METH_VARARGS, "List defined programs"},
    {"listEnabledPrograms", listEnabledPrograms, METH_VARARGS,
         "List the programs that are enabled"},
    {"listStandalonePrograms", listStandalonePrograms, METH_VARARGS,
         "List programs with the standalone flag set"},
    {"listInactivePrograms", listInactivePrograms, METH_VARARGS,
         "List programs that are either standalone or disabled"},
    {"listActivePrograms", listActivePrograms, METH_VARARGS,
         "List programs that are enabled and not standalone"},
    {"deleteProgram", deleteProgram, METH_VARARGS, "delete a program"},
    {"setGlobalState", setGlobalState, METH_VARARGS, "Start a global state transition"},
    {NULL, NULL, 0, NULL}                /* End of method definition marker */       
};



/* Module level method dispatch table */

static PyMethodDef StateManagerClassMethods[] = {
  {NULL, NULL, 0, NULL}                /* End of methods sentinell. */  
};

/* Api type descriptions */

static PyTypeObject stateManager_ApiType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "statemanager.Api",             /*tp_name*/
    sizeof(statemanager_ApiObject), /*tp_basicsize*/
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

/**
 * initstatemanager
 *    Initialize the class.
 */
PyMODINIT_FUNC
initstatemanager(void)
{
    PyObject* module;
    
    /* Create the module object.  that's the minimum. */
    
    module = Py_InitModule3(
        "statemanager", StateManagerClassMethods, "Cstatemanager encapsulation"
    );
    if (module == NULL) {
        return;                    // For future expansion.
    }

    /* Create our exception */
    
    error = PyErr_NewException(
        const_cast<char*>("statemanager.error"),       
        NULL, NULL
    );

    Py_INCREF(error);
    PyModule_AddObject(module, "error", error);
 
/* Register our new type */
    
    if (PyType_Ready(&stateManager_ApiType) < 0) {
        return;
    }
    Py_INCREF(&stateManager_ApiType);
    PyModule_AddObject(
        module, "Api", reinterpret_cast<PyObject*>(&stateManager_ApiType)
    );    
    
}

void* gpTCLApplication(0);