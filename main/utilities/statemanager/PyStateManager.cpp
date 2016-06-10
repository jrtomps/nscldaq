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
#include "CStateProgram.h"
#include "PyStateManager.h"

/* Module specific error: */

static PyObject* error;
/* data types:  */

typedef struct {
    PyObject*   s_stateManager;
    PyObject*   s_callback;
    PyObject*   s_calldata;
} CallbackInfo, *pCallbackInfo;



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
 * getProgramApi
 *    Get the api associated with program management:
 * @param self  - object pointer as a PyObject*
 */
static CStateProgram*
getProgramApi(PyObject* self)
{
     statemanager_ApiObject* pThis =
        reinterpret_cast<statemanager_ApiObject*>(self);
    
    return pThis->m_pPrograms;
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
    
    return result;    
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
 * relayTransitionCallbacks
 *    Callback for waitTransitions which relays to a Python
 *    callable:
 *
 * @param mgr  - CStateManager* pointer ignored.
 * @param program - name of the program that transitioned.
 * @param state   - New state of that program.
 * @param cd      - Actually a pCallbackInfo with additional
 *                  information about the callback.
 * @note if the CalllbackInfo struct has Py_None for a s_callback
 *       no callback is actually performed.
 */
static void
relayTransitionCallbacks(
    CStateManager& mgr, std::string program, std::string state, void* cd
)
{
    pCallbackInfo pInfo = static_cast<pCallbackInfo>(cd);
    if (pInfo->s_callback != Py_None) {
        // Marshall the callable's parameters into a tuple:
        
        PyObject* args = PyTuple_Pack(
            4, pInfo->s_stateManager, PyString_FromString(program.c_str()),
            PyString_FromString(state.c_str()), pInfo->s_calldata
        );
        
        PyObject_CallObject(pInfo->s_callback, args);
        
    }
}

/**
 * relayNotificationCallbacks
 *    Relay the notification callbacks from processMessages
 *    The callable is given the parameters:
 *    - api          - Api object reference.
 *    - notification - dict-ized version of the notification message. this has
 *                     the following keys:
 *                    - type - Type string.
 *                    - state - if appropriate the new state.
 *                    - program - if appropriate the program name.
 *                    
 *    - cd           - The callback data object.
 *    
 * @param mgr          - Reference to the underlying state manager.
 * @param notification - Notification contains information about the event.
 * @param cd           - Callback data which is actually a pCallbackInfo.
 */
static void
relayNotificationCallbacks(
    CStateManager& mgr, CStateTransitionMonitor::Notification notification,
    void* cd
)
{
    pCallbackInfo pInfo = static_cast<pCallbackInfo>(cd);
    
    /* Marshall the notification msg into a dict. */
    
    PyObject* notDict = PyDict_New();
    
    // What we put depends on the message type:
    
    switch (notification.s_type) {
        case CStateTransitionMonitor::GlobalStateChange:
            PyDict_SetItemString(
                notDict, "type", PyString_FromString("GlobalStateChange")
            );
            PyDict_SetItemString(
                notDict, "state",
                PyString_FromString(notification.s_state.c_str())
            );
            break;
        case CStateTransitionMonitor::ProgramStateChange:
            PyDict_SetItemString(
                notDict, "type", PyString_FromString("ProgramStateChange")
            );
            PyDict_SetItemString(
                notDict, "program",
                PyString_FromString(notification.s_program.c_str())
            );
            PyDict_SetItemString(
                notDict, "state",
                PyString_FromString(notification.s_state.c_str())
            );
            break;
        case CStateTransitionMonitor::ProgramJoins:
            PyDict_SetItemString(
                notDict, "type", PyString_FromString("ProgramJoins")
            );
            PyDict_SetItemString(
                notDict, "program",
                PyString_FromString(notification.s_program.c_str())
            );
            break;
        case CStateTransitionMonitor::ProgramLeaves:
            PyDict_SetItemString(
                notDict, "type", PyString_FromString("ProgramLeaves")
            );
            PyDict_SetItemString(
                notDict, "program",
                PyString_FromString(notification.s_program.c_str())
            );
            break;
        case CStateTransitionMonitor::VarChanged:
            PyDict_SetItemString(
                notDict, "type", PyString_FromString("VarChanged")
            );
            PyDict_SetItemString(
                notDict, "path",
                PyString_FromString(notification.s_state.c_str())
            );
            PyDict_SetItemString(
                notDict, "value",
                PyString_FromString(notification.s_program.c_str())
            );
            break;
        default:
            /* TODO:  Figure out what to do here?  */
            break;
    }
    /* Build the parameter list for the callback: */
    
    PyObject* args = PyTuple_Pack(
        3, pInfo->s_stateManager, notDict, pInfo->s_calldata
    );
    
    /* Call the callback */
    
    PyObject_CallObject(pInfo->s_callback, args);
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
Api_init(statemanager_ApiObject* self, PyObject* args, PyObject* kwds)
{
    const char* reqUri;
    const char* subUri(0);
    
    // Could be one or two URIs:
    
    if (PyTuple_Size(args) == 1)  {
        if (!PyArg_ParseTuple(args, "s", &reqUri)) {
            PyErr_SetString(error, "Failed to extract URI from parameter list");
            return -1;
        }
    } else {
        if (!PyArg_ParseTuple(args, "ss", &reqUri, &subUri)) {
            PyErr_SetString(error, "need requri and suburi");
            return -1;
        }        
    }
    

    /* Destroy any pre-existing m_pApi and create a new one. */
    
    if (self->m_pApi) {
        delete self->m_pApi;
    } else {
        delete self->m_pPrograms;
    }

    try {
        if (subUri) {
            self->m_pApi = new CStateManager(reqUri, subUri);
            self->m_pPrograms = self->m_pApi->getProgramApi();
        } else {
            self->m_pPrograms = new CStateProgram(reqUri);
        }
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
    if (self->m_pApi) {
        delete self->m_pApi;
    } else {
        delete self->m_pPrograms;
    }
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
    CStateProgram* pPrograms = getProgramApi(self);
    std::string dir;
    try {
        if (pApi) {
            dir = pApi->getProgramParentDir();   // Has special caching logic.
        } else {
            dir = pPrograms->getProgramParentDir();
        }
    }
    catch(std::exception& e) {
        return raise(e.what());
    }
    
    PyObject* result = PyString_FromString(dir.c_str());
    return result;

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
    
    CStateManager* pApi      = getApi(self);
    CStateProgram* pPrograms = getProgramApi(self);
    try {
        if (pApi) {
            pApi->setProgramParentDir(newDir);   // Actions with monitor thread needed.
        } else {
            pPrograms->setProgramParentDir(newDir);
        }
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
    
    
    CStateProgram* pApi = getProgramApi(self);
    
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
    
    CStateProgram* pApi = getProgramApi(self);
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
    
    return result;
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
        CStateProgram* pApi = getProgramApi(self);
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
    
    CStateProgram* pApi = getProgramApi(self);
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

    CStateProgram* pApi = getProgramApi(self);
    
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
    
    CStateProgram* pApi = getProgramApi(self);
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
    
    CStateProgram* pApi = getProgramApi(self);

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
    
    CStateProgram* pApi = getProgramApi(self);

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
    
    CStateProgram* pApi = getProgramApi(self);
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
    
    CStateProgram* pApi = getProgramApi(self);
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
    
    CStateProgram* pApi = getProgramApi(self);
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
    
    CStateProgram* pApi = getProgramApi(self);
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
    
    CStateProgram* pApi = getProgramApi(self);
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
    
    CStateProgram* pApi = getProgramApi(self);
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
    
    CStateProgram* pApi = getProgramApi(self);
    
    try {
        pApi->deleteProgram(programName);
    }
    catch (std::exception& e) {
        return raise(e.what());
    }
    
    Py_RETURN_NONE;
}

/**
 * setEditorPosition
 *     Set the editor position for an object.
 *
* @param self - Pointer to the object whose method this is.
* @param args - Pointer to the python argument list (tuple) : program name.
* @return PyObject* - Py_None
*/  
static PyObject*
setEditorPosition(PyObject* self, PyObject* args)
{
    char* name;
    int   x;
    int   y;
    
    if (!PyArg_ParseTuple(args, "sii", &name, &x, &y)) {
        return NULL;
    }
    CStateProgram* pApi = getProgramApi(self);
    try {
        pApi->setEditorPosition(name, x, y);
    }
    catch(std::exception& e) {
        return raise(e.what());
    }
    
    Py_RETURN_NONE;
}
/**
 * getEditorXPosition
 *    Get the x coordinate of the objects position in the editor.
 *
 *
* @param self - Pointer to the object whose method this is.
* @param args - Pointer to the python argument list (tuple) : program name.
* @return PyIntObject* - x coordinate of the objects's position.
*/  
static PyObject*
getEditorXPosition(PyObject* self, PyObject* args)
{
    char* name;
    int   x;
    
    if (!PyArg_ParseTuple(args, "s", &name)) {
        return NULL;
    }
    
    CStateProgram* pApi = getProgramApi(self);
    try {
        x = pApi->getEditorXPosition(name);
    }
    catch (std::exception& e) {
        return raise(e.what());
    }
    
    return PyInt_FromLong(x);
}

/**
 * getEditorYPosition
 *    Get the x coordinate of the objects position in the editor.
 *
 *
* @param self - Pointer to the object whose method this is.
* @param args - Pointer to the python argument list (tuple) : program name.
* @return PyIntObject* - x coordinate of the objects's position.
*/  
static PyObject*
getEditorYPosition(PyObject* self, PyObject* args)
{
    char* name;
    int   y;
    
    if (!PyArg_ParseTuple(args, "s", &name)) {
        return NULL;
    }
    
    CStateProgram* pApi = getProgramApi(self);
    try {
        y = pApi->getEditorYPosition(name);
    }
    catch (std::exception& e) {
        return raise(e.what());
    }
    
    return PyInt_FromLong(y);
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
    if (!pApi) {
        return raise("This method is only allowed when a subscription URI was passed at construction time");
    }
    try {
        pApi->setGlobalState(newState);
    }
    catch(std::exception& e) {
        return raise(e.what());
    }
    
    Py_RETURN_NONE;
}

/**
* getGlobalState
*     Return the current global state variable.
*
* @param self - Pointer to the object whose method this is.
* @param args - Pointer to the python argument list (tuple) (empty)
* @return PyObject* - PyString - current global state.
*/
static PyObject*
getGlobalState(PyObject* self, PyObject* args)
{
    if (PyTuple_Size(args) > 0) {
        return raise("getGlobalState does not require argumetns");
    }
    
    CStateManager* pApi = getApi(self);
    if (!pApi) {
        return raise("This method is only allowed when a subscription URI was passed at construction time");
    }

    std::string result;
    try {
        result = pApi->getGlobalState();
    }
    catch (std::exception& e) {
        return raise(e.what());
    }
    
    return PyString_FromString(result.c_str());
}
/**
 * getSystemStatus
 *     returns the system status variable value.  This tells the user if a state
 *     transition is in progress.
 *
 *  @param self - pointer to the object on which this is called.
 *  @param args - pointer to the positional args which must be empty.
 *  @return PyObject*  - Newly created string object containing the result.
 */
static PyObject*
getSystemStatus(PyObject* self, PyObject* args)
{
    if (PyTuple_Size(args) > 0) {
        return raise("getSystemStatus does not require parameters");
    }
    CStateManager* pApi = getApi(self);
    if (!pApi) {
        return raise("This method is only allowed when a subscription URI was passed at construction time");
    }
    std::string result;
    try {
        result = pApi->getSystemStatus();
    }
    catch (std::exception& e) {
        return raise(e.what());
    }
    return PyString_FromString(result.c_str());
}

/**
* getParticipantStates
*     Return a map of programs and their states.
*
* @param self - Pointer to the object whose method this is.
* @param args - Pointer to the python argument list (tuple) (empty)
* @return PyObject* - PyDict keys are the names of programs, values are their
*                   states.
*/
static PyObject*
getParticipantStates(PyObject* self, PyObject* args)
{
    if (PyTuple_Size(args) > 0) {
        return raise("getGlobalState does not require argumetns");
    }
    
    CStateManager* pApi = getApi(self);
    if (!pApi) {
        return raise("This method is only allowed when a subscription URI was passed at construction time");
    }
    if (!pApi) {
        return raise("This method is only allowed when a subscription URI was passed at construction time");
    }

    std::vector<std::pair<std::string, std::string> > states;
    try  {
        states = pApi->getParticipantStates();
    }
    catch (std::exception& e) {
        return raise(e.what());
    }
    
    // Marshall the resulting vector into a dict:
    
    PyObject* result = PyDict_New();
    for (int i =0; i < states.size(); i++) {
        PyDict_SetItemString(
            result, states[i].first.c_str(),
            PyString_FromString(states[i].second.c_str())
        );
    }
    return result;
}

/**
* getTitle
*     Return the current title string.
*
* @param self - Pointer to the object whose method this is.
* @param args - Pointer to the python argument list (tuple) (empty)
* @return PyObject* - PyString - current title string.
*/
static PyObject*
getTitle(PyObject* self, PyObject* args)
{
    if (PyTuple_Size(args) > 0) {
        return raise("getGlobalState does not require argumetns");
    }
    
    CStateManager* pApi = getApi(self);
    if (!pApi) {
        return raise("This method is only allowed when a subscription URI was passed at construction time");
    }

    std::string title;
    try {
        title = pApi->title();
    }
    catch (std::exception& e) {
        return raise(e.what());
    }
    
    return PyString_FromString(title.c_str());
}

/**
* setTitle
*     Set the current title.
*
* @param self - Pointer to the object whose method this is.
* @param args - Pointer to the python argument list (tuple) - title string
* @return PyObject* - PyNone
*/
static PyObject*
setTitle(PyObject* self, PyObject* args)
{
    char* title;
    if(!PyArg_ParseTuple(args, "s", &title)) {
        return raise("setTitle needs a title string parameter (only)");
    }
    
    CStateManager* pApi = getApi(self);
    if (!pApi) {
        return raise("This method is only allowed when a subscription URI was passed at construction time");
    }

    try {
        pApi->title(title);
    }
    catch (std::exception& e) {
        return raise(e.what());
    }
    
    Py_RETURN_NONE;
}

/**
* getTimeout
*     Return the current timeout value.
*
* @param self - Pointer to the object whose method this is.
* @param args - Pointer to the python argument list (tuple) (empty)
* @return PyObject* - PyLong - containing the timeout.
*/
static PyObject*
getTimeout(PyObject* self, PyObject* args)
{
    if(PyTuple_Size(args) > 0) {
        return raise("getTimeout requires no parameters");
    }
    
    CStateManager* pApi = getApi(self);
    if (!pApi) {
        return raise("This method is only allowed when a subscription URI was passed at construction time");
    }

    unsigned timeout;
    
    try {
        timeout = pApi->timeout();
    }
    catch (std::exception& e) {
        return raise(e.what());
    }
    
    return PyLong_FromUnsignedLong(timeout);
}

/**
* setTimeout
*     Set the state transition timeout value.
*
* @param self - Pointer to the object whose method this is.
* @param args - Pointer to the python argument list (tuple) (unsigned).
* @return PyObject* - Py_None
*/
static PyObject*
setTimeout(PyObject* self, PyObject* args)
{
    int timeout;
    if (!PyArg_ParseTuple(args, "i", &timeout)) {
        return raise("setTimeout needs a timeout parameter");
    }
    if (timeout <= 0) {
        return raise("Timeout value must be at least one second");
    }
    
    CStateManager* pApi = getApi(self);
    if (!pApi) {
        return raise("This method is only allowed when a subscription URI was passed at construction time");
    }

    try {
        pApi->timeout(timeout);
    }
    catch (std::exception& e) {
        return raise(e.what());
    }
    
    Py_RETURN_NONE;
}

/**
* isRecording
*     Return recording state.
*
* @param self - Pointer to the object whose method this is.
* @param args - Pointer to the python argument list (tuple) (empty)
* @return PyObject* - Py_True if recording or Py_False if not.
*/
static PyObject*
isRecording(PyObject* self, PyObject* args)
{
    if(PyTuple_Size(args) > 0) {
        return raise("isRecording takes no parameters");
    }
    
    CStateManager* pApi = getApi(self);
    if (!pApi) {
        return raise("This method is only allowed when a subscription URI was passed at construction time");
    }

    bool recording;
    try {
        recording = pApi->recording();
    }
    catch(std::exception& e) {
        return raise(e.what());
    }
    
    if (recording) {
        Py_RETURN_TRUE;
    } else {
        Py_RETURN_FALSE;
    }
}

/**
* setRecording
*     Set the recording state.
*
* @param self - Pointer to the object whose method this is.
* @param args - Pointer to the python argument list (tuple) (PyBool)
* @return PyObject* - PyNone.
*/
static PyObject*
setRecording(PyObject* self, PyObject* args)
{
    PyObject* newState;
    
    if (!PyArg_ParseTuple(args, "O", &newState)) {
        return raise("setRecording needs a new state value");
    }
    if (!PyBool_Check(newState)) {
        return raise("setRecording new state must be a boolean");
    }
    
    CStateManager* pApi = getApi(self);
    if (!pApi) {
        return raise("This method is only allowed when a subscription URI was passed at construction time");
    }

    bool state = (newState == Py_True) ? true : false;
    
    try {
        pApi->recording(state);
    }
    catch (std::exception& e) {
        return raise(e.what());
    }
    
    Py_RETURN_NONE;
}

/**
* getRunNumber
*     Return the current run number.
*
* @param self - Pointer to the object whose method this is.
* @param args - Pointer to the python argument list (tuple). (empty)
* @return PyObject* - PyLong
*/
static PyObject*
getRunNumber(PyObject* self, PyObject* args)
{
    if (PyTuple_Size(args) > 0) {
        return raise("getRunNumber needs no parameters");
    }
    
    CStateManager* pApi = getApi(self);
    if (!pApi) {
        return raise("This method is only allowed when a subscription URI was passed at construction time");
    }

    unsigned       runNum;
    
    try {
        runNum = pApi->runNumber();
    }
    catch (std::exception & e) {
        return raise(e.what());
    }
    
    return PyLong_FromUnsignedLong(runNum);
}
/**
* setRunNumber
*     Change the run number
*
* @param self - Pointer to the object whose method this is.
* @param args - Pointer to the python argument list (tuple) (unsigned).
* @return PyObject* - PyNone
*/
static PyObject*
setRunNumber(PyObject* self, PyObject* args)
{
    int newRun;
    if (!PyArg_ParseTuple(args, "i", &newRun)) {
        return raise("setRunNumber needs a run number parameter (only)");
    }
    if (newRun < 0) {
        return raise("Run Numbers must be positive");
    }
    CStateManager* pApi = getApi(self);
    if (!pApi) {
        return raise("This method is only allowed when a subscription URI was passed at construction time");
    }

    
    try {
        pApi->runNumber(newRun);
    }
    catch (std::exception &e) {
        return raise(e.what());
    }
    
    Py_RETURN_NONE;
    
}

/**
* waitTransition
*     Wait for a state transition to complete or timeout.
*
* @param self - Pointer to the object whose method this is.
* @param args - Pointer to the python argument list (tuple).
*               This can be 0,1, or two parameters as follows:
*               - If provided the first parameter is a callback
*                 for each program state transition that is detected
*                 it is passed:
*                 > This object (self)
*                 > The name of the program transitioning.
*                 > The new state of the program.
*                 > Any client data (see below).
*               - If provided, the second parameter is passed
*                 without interpretation.  If not provided,
*                 Py_None is passed.
* @note if parameter 1 is missing, no callbacks will be invoked.
* 
* @return PyObject* - PyBool - Py_True on completed transition or
*                              Py_False if failed transition.
*/
static PyObject*
waitTransition(PyObject* self, PyObject* args)
{
    CallbackInfo info = {
        self, Py_None, Py_None
    };
    // Fill in the supplied bits of the info struct:
    
    PyObject* item;
    if (PyTuple_Size(args) > 2) {
        return raise("waitTransition takes at most two parameters");
    }
    if (item = PyTuple_GetItem(args, 0)) {
        if (!PyCallable_Check(item)) {    /* Must be a callable: */
            return raise("waitTransition - first parameter must be callable");
        }
        info.s_callback = item;
    }
    if (item = PyTuple_GetItem(args, 1)) {
        info.s_calldata = item;
    }
    PyErr_Clear();       /* Clear any get_item index errors. */
    
    CStateManager* pApi = getApi(self);
    if (!pApi) {
        return raise("This method is only allowed when a subscription URI was passed at construction time");
    }

    bool success;
    try {
        pApi->waitTransition(
            relayTransitionCallbacks, reinterpret_cast<void*>(&info)
        ); 
        success = true;
    }
    catch (std::runtime_error& e) {
        success = false;
    }
    
    if (success) {
        Py_RETURN_TRUE;
    } else {
        Py_RETURN_FALSE;
    }
    
}

/**
* processMessages
*     Process messages published by the varmgr server that have
*     been cooked via a StateTransitionMonitor.
*
* @param self - Pointer to the object whose method this is.
* @param args - Pointer to the python argument list (tuple)
*              - Callable that is invoked for each message.
*              - Optional data to be passed to the callable.
*                If not suppled, Py_None is passed in its stead.
*                
* @return PyObject* - Py_None
*/
static PyObject*
processMessages(PyObject* self, PyObject* args)
{
    CallbackInfo info = {self, Py_None, Py_None};
    PyObject* item;
    
    /* there must be at least one parameter: */
    
    if (PyTuple_Size(args) == 0) {
        return raise("processMessages needs at least a callable");
    }
    
    /* Marshall the callback struct:  */
    
    info.s_callback = PyTuple_GetItem(args, 0);
    if (!PyCallable_Check(info.s_callback)) {
        return raise("processMessages first parameter must be a callable");
    }
    
    if(item = PyTuple_GetItem(args, 1)) {
        info.s_calldata = item;
    }
    PyErr_Clear();
    if(PyTuple_Size(args) > 2) {
        return raise("processMessages takes at most two parameters");
    }
    
    CStateManager* pApi = getApi(self);
    if (!pApi) {
        return raise("This method is only allowed when a subscription URI was passed at construction time");
    }

    try {
        pApi->processMessages(relayNotificationCallbacks, &info);
    }
    catch(std::exception& e) {
        return raise(e.what());
    }
    
    Py_RETURN_NONE;
    
}

/**
* isActive
*     Check that a program is ative.
*
* @param self - Pointer to the object whose method this is.
* @param args - Pointer to the python argument list (tuple) : program name..
* @return PyObject* - PyBoolean Py_True if active, Py_False if not.
*/
static PyObject*
isActive(PyObject* self, PyObject* args)
{
    char* program;
    if (!PyArg_ParseTuple(args, "s", &program)) {
        return raise("isActive needs (only) a program name.");
    }
    
    CStateManager* pApi = getApi(self);
    if (!pApi) {
        return raise("This method is only allowed when a subscription URI was passed at construction time");
    }

    bool           isActive;
    try {
        isActive = pApi->isActive(program);
    }
    catch(std::exception& e) {
        return raise(e.what());
    }
    
    if (isActive) {
        Py_RETURN_TRUE;
    } else {
        Py_RETURN_FALSE;
    }

}

/**
* setProgramState
*     Set the program state for a specific program.
*
* @param self - Pointer to the object whose method this is.
* @param args - Pointer to the python argument list (tuple) : program , state
* @return PyObject* - Py_None
*/
static PyObject*
setProgramState(PyObject* self, PyObject* args)
{
    char* program;
    char* state;
    
    if (!PyArg_ParseTuple(args, "ss", &program, &state)) {
        return raise("setProgramState needs program and state (only)");
    }
    
    CStateManager* pApi = getApi(self);
    if (!pApi) {
        return raise("This method is only allowed when a subscription URI was passed at construction time");
    }

    try {
        pApi->setProgramState(program, state);
    }
    catch (std::exception& e) {
        return raise(e.what());
    }
    
    Py_RETURN_NONE;
}

/**
* getProgramState
*     Get the state of a program.
*
* @param self - Pointer to the object whose method this is.
* @param args - Pointer to the python argument list (tuple) : Program name
* @return PyObject* - PyString - program's state.
*/
static PyObject*
getProgramState(PyObject* self, PyObject* args)
{
    char* program;
    if (!PyArg_ParseTuple(args, "s", &program)) {
        return raise("getProgramState needs a programname (only).");
    }
    
    CStateManager* pApi = getApi(self);
    if (!pApi) {
        return raise("This method is only allowed when a subscription URI was passed at construction time");
    }

    std::string state;
    try {
        state = pApi->getProgramState(program);
    }
    catch (std::exception& e) {
        return raise(e.what());
    }
    
    return PyString_FromString(state.c_str());
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
    {"setEditorPosition", setEditorPosition, METH_VARARGS, "set editor position of object"},
    {"getEditorXPosition", getEditorXPosition, METH_VARARGS, "Get x position of an object"},
    {"getEditorYPosition", getEditorYPosition, METH_VARARGS, "Get y position of an object"},
    {"setGlobalState", setGlobalState, METH_VARARGS, "Start a global state transition"},
    {"getGlobalState", getGlobalState, METH_VARARGS, "Get The global state"},
    {"getSystemStatus", getSystemStatus, METH_VARARGS, "Get the state manager system status"},
    {"getParticipantStates", getParticipantStates, METH_VARARGS,
        "Get the states of participants"},
    {"getTitle", getTitle, METH_VARARGS, "Get the current title"},
    {"setTitle", setTitle, METH_VARARGS, "Set the title"},
    {"getTimeout", getTimeout, METH_VARARGS,
        "Get the state transition timeout value"},
    {"setTimeout", setTimeout, METH_VARARGS,
        "Set the state transition timeout value"},
    {"isRecording", isRecording, METH_VARARGS, "Return recording state"},
    {"setRecording", setRecording, METH_VARARGS, "Set recording states"},
    {"getRunNumber", getRunNumber, METH_VARARGS, "Get current run number"},
    {"setRunNumber", setRunNumber, METH_VARARGS, "Set new run number"},
    {"waitTransition", waitTransition, METH_VARARGS, "Wait for state transitions"},
    {"processMessages", processMessages, METH_VARARGS, "Process messages"},
    {"isActive", isActive, METH_VARARGS, "Check for program active"},
    {"setProgramState", setProgramState, METH_VARARGS, "Set state of a specific program"},
    {"getProgramState", getProgramState, METH_VARARGS, "Get the state of a program"},
    {NULL, NULL, 0, NULL}                /* End of method definition marker */       
};

// For external consumption

PyMethodDef* StateManagerMethods=ApiObjectMethods;

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