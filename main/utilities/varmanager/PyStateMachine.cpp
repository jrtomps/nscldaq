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
# @file   PyStateMachine.cpp
# @brief  Python API for state machine data types.
# @author <fox@nscl.msu.edu>
*/


#include <Python.h>
#include "PyVarDb.h"
#include <CVariableDb.h>
#include <exception>
#include <vector>
#include "CStateMachine.h"


PyObject* exception;


/*-----------------------------------------------------------------------------
 * Utilities:
 */
/**
 * raise
 *    Sets the exception string and returns NULL
 *    thus you can raise our exception by return raise(string);
 * @param msg - const char* - error message string
 * @return NULL
 */
static PyObject*
raise(const char* msg)
{
    PyErr_SetString(exception, msg);
    return NULL;
}

/**
 * variableDbFromObj
 *   @param  obj  - PyObject* for an object that is supposed to be a vardb_VarDbObject*
 *   @return CVariable* Pointer to the CVariableDb* encapsulated in that object.
 *   @retval NULL - If the obj is not a variable database object. In that case
 *                  the exception is already raised and the caller just has
 *                  to return NULL.
 */
static CVariableDb*
variableDbFromObj(PyObject* obj) {
    if (!isDb(obj)) {
        return reinterpret_cast<CVariableDb*>(
            raise( "Parameter must be a database object")
        );
    }
    vardb_VarDbObject* pDbObj = reinterpret_cast<vardb_VarDbObject*>(obj);
    return pDbObj->m_db;
}

/**
 * buildTransitionMap
 *   Given a transition dict, build/return the transition table represented
 *   by that map:
 * @param dict    - The dictionary containing the map.
 * @return CStateMachine::TransitionMap  - The transition table built.
 * @throw  runtime_error - if the object is not a dict and if the dict 
 *                         values are not iterable.
 */
static CStateMachine::TransitionMap
buildTransitionMap(PyObject* dict)
{
    CStateMachine::TransitionMap map;
    
    if(!PyDict_Check(dict)) {
        throw std::runtime_error("State map is not a dict and must be");
    }
    // We get and iterate over the dict keys..  These are the allowed states
    
    PyObject* pKeyList     = PyDict_Keys(dict);
    PyObject* pKeyIterator = PyObject_GetIter(pKeyList);
    if (!pKeyIterator) {
        Py_DECREF(pKeyList);
        throw std::runtime_error("BUG - state map key list cannot be iterated!!");
    }
    // Get the key string and value list...and iterate over that to get the
    // target states:
    
    PyObject* pKey;
    while (pKey = PyIter_Next(pKeyIterator)) {
        
        // Get the key as text and the value list:
        
        const char* state = PyString_AsString(pKey);
        PyObject* pToList = PyDict_GetItem(dict, pKey);
        if (!pToList) {
            Py_DECREF(pKey);
            Py_DECREF(pKeyIterator);
            throw std::runtime_error("BUG - state map key without a value!!!");
        }
        // Get an iterator for the value list:
        
        PyObject* pValueIterator = PyObject_GetIter(pToList);
        if (!pValueIterator) {
            Py_DECREF(pKey);
            Py_DECREF(pKeyList);
            Py_DECREF(pKeyIterator);
            throw std::runtime_error("All state map values must be iterable");
        }
        // Now we have the stuff needed to iterate over the values:
        
        PyObject* pValue;
        while (pValue = PyIter_Next(pValueIterator)) {
            const char* toState = PyString_AsString(pValue);
            CStateMachine::addTransition(map, state, toState);
            Py_DECREF(pValue);
        }
        

        Py_DECREF(pValueIterator);
        Py_DECREF(pKey);
    }
    
    Py_DECREF(pKeyList);
    Py_DECREF(pKeyIterator);
    
    return map;
}
/**
 * transtionsToDict
 *    Converts a transtion map to a dict as described in sm_create below.
 *
 *  @param tMap - the transition map (reference just for performance)
 *  @return PyObject* pointer to a dict object that describes the transtion map.
 */
static PyObject*
transitionsToDict(CStateMachine::TransitionMap& tMap)
{
    PyObject* result = PyDict_New();
    
    // Iterate over the states and the target states each state has:
    
    CStateMachine::TransitionMap::iterator pStates = tMap.begin();
    while (pStates != tMap.end()) {
        std::string fromState = pStates->first;
        
        // Make a tuple of the to states:
        
        PyObject* nextList = PyTuple_New(pStates->second.size());
        std::set<std::string>::iterator pNexts = pStates->second.begin();
        int i = 0;
        while(pNexts != pStates->second.end()) {
            std::string toState = *pNexts;
            PyTuple_SET_ITEM(nextList, i, PyString_FromString(toState.c_str()));
            
            i++;
            pNexts++;
        }
        // Add that tuple into the dict:
        
        PyDict_SetItem(result, PyString_FromString(fromState.c_str()), nextList);
        pStates++;
    }
    
    return result;
}
/*---------------------------------------------------------------------------
 * module level methods:
 */

/**
 * sm_create
 *    Create a statemachine (bindg to the CStateMachine::create method)
 *
 *  @param self   - Not used - pointer to the module object.
 *  @param args   - Parameters to the method call.  These must be in order:
 *                *   A database handle.
 *                *   A type name.
 *                *   A state transition map.  A state transition map is a
 *                    dict whose keys are valid states and whose values are
 *                    iterable.  The elements of the iterables are valid 'to'
 *                    states.  For example:
 *                    {'first' : ('second',), 'second' : ('third',),
 *                     'third' : ('first', 'second') }
 *                    Defines the following state transition table:
 * \verbatim
 *                    |  From state       | To state     |
 *                    +-------------------+--------------+
 *                    | first             | second       |
 *                    | second            | third        |
 *                    | third             | first        |
 *                    | third             | second       |
 *                    +-------------------+--------------+
 * \endverbatim
 * @note in the example above, the extra commas are needed if there is only a
 *       single allowed transition in order to make the value a tuple rather than
 *       a string.  Both are iterable, but in processing a string, what create
 *       will see is e.g. 'first' transitions to s, e,c,o,n,d none of which are
 *       valid states.
 *       
 * @return PyObject* type id of the newly created type.
 */
static PyObject*
sm_create(PyObject* self, PyObject* args)
{
    PyObject*   dbObj;
    const char* typeName;
    PyObject*   transitionDict;
    
    if (!PyArg_ParseTuple(args, "OsO", &dbObj, &typeName, &transitionDict )) {
        return raise("statemachine.create - could not parse parameters");
    }
    
    /* Extract the database handle:  */
    
    CVariableDb* pDb = variableDbFromObj(dbObj);
    if (! pDb) return NULL;
    
    // Both buildTransitionMap and CStateMachine::crete can throw runtime_eror's
    // map those to our exception.
    
    int id;
    try {
        CStateMachine::TransitionMap transitions =
            buildTransitionMap(transitionDict);
            
        id = CStateMachine::create(*pDb, std::string(typeName), transitions);
    }        
    catch (std::runtime_error& err) {
        PyErr_SetString(exception, err.what());
        return NULL;
    }
    
    return PyInt_FromLong(id);
    
}
/**
 * sm_isStateMachine
 *    Jacket for CStateMachine::isStateMachine.
 *
 * @param self  - module object pointer (unused).
 * @param args  - method arguments. These are:
 *               * db   - Database handle.
 *               * id   - an integer type id.
 */
static PyObject*
sm_isStateMachine(PyObject* self, PyObject* args)
{
    PyObject* db;
    int       typeId;
    
    if(!PyArg_ParseTuple(args, "Oi", &db, &typeId)) {
        return raise("Cannot parse arguments");
    }
    // Pull the  CVariableDb from the datbase object.
    
    CVariableDb* pDb = variableDbFromObj(db);
    if (!pDb) return NULL;             // Error already raised.
    
    // try/catch to modify c++ throws to a raise:
    
    bool result;
    try {
        result = CStateMachine::isStateMachine(*pDb, typeId);
    }
    catch (std::runtime_error& err) {
        return raise(err.what());
    }
    // Map the C++ bool to the correct Python object return:
    
    if (result) {
        Py_RETURN_TRUE;
    } else {
        Py_RETURN_FALSE;
    }
    
}
/**
 * sm_validNextStates
 *    Determine the next allowed states for either a type (given a from state)
 *    or for a variable that is a state machine...given its current value.
 * @param self    - Pointer to the module object (unused)
 * @param args    - Parameters.  This method is two-way polymorphic:
 *                  Type:
 *                   *   db     - Database handle.
 *                   *   typeId - Id of a data type.
 *                   *   from   - State in the type for which we want valid next
 *                                states.
 *                   Variable:
 *                   *   db     - Database Handle.
 *                   *   varId  - Id of a variable whose next valid states we
 *                                want.
 */
static PyObject*
sm_validNextStates(PyObject* self, PyObject* args)
{
    PyObject*   db;
    int         id;
    const char* currentState;
    
    // Figure out how many parameters we have as that determines our
    // parse and control flow:
    
    int nArgs = PyTuple_Size(args);
    
    if (nArgs == 2) {
        if (!PyArg_ParseTuple(args, "Oi", &db, &id)) {
            return raise("validNextStates - could not parse parameters");
        }
    } else if (nArgs == 3) {
        if (!PyArg_ParseTuple(args, "Ois", &db, &id, &currentState)) {
            return raise("validNextStates - could not parse parameters");
        }
        
    } else {
        return raise("validNextStates requires two or three parameters");
    }
    
    // Get the database from the db object:
    
    CVariableDb* pDb = variableDbFromObj(db);
    if (!pDb) return NULL;              // Error already raised.
    
    // Get the result vector in a try/catch block depending on the
    // number of params.
    
    std::vector<std::string> nextStates;
    
    try {
        if (nArgs == 2) {
            nextStates = CStateMachine::validNextStates(*pDb, id);
        } else {
            nextStates = CStateMachine::validNextStates(*pDb, id, currentState);
        }
    }
    catch (std::runtime_error& err) {
        return raise(err.what());
    }
    // Marshall the nextStates vector into a tuple.
    
    PyObject* result = PyTuple_New(nextStates.size());
    for (int i = 0; i < nextStates.size(); i++) {
        PyObject* state =  PyString_FromString(nextStates[i].c_str());                                          
        PyTuple_SET_ITEM(result, i, state);
    }
    
    Py_INCREF(result);
    return result;
}

/**
 * sm_getTransitionMap
 *   Reconstruct the transition map dict for a state machine type.
 *
 *  @param self   - Pointer to the module object (unused).
 *  @param args   - Tuple containing the method arguments:
 *                  *   db    - database object.
 *                  *   typeId- Id of the state machine type being queried.
 * @return PyObject* A dict that defines the state machine for the type as
 *                   describedin sm_create.
 */
static PyObject*
sm_getTransitionMap(PyObject* self, PyObject* args)
{
    PyObject* db;
    int       typeId;
    
    if (!PyArg_ParseTuple(args, "Oi", &db, &typeId)) {
        return raise("Unable to parse parameters");
    }
    
    // Get the database C++ object and get the C++ transition map in a try
    // catch block to map C++ exceptions to a raise:
    
    CVariableDb* pDb = variableDbFromObj(db);
    if (!pDb) return NULL;
    
    CStateMachine::TransitionMap tMap;
    try {
        tMap = CStateMachine::getTransitionMap(*pDb, typeId);
    }
    catch (std::runtime_error& e) {
        return raise(e.what());
    }
    
    
    return transitionsToDict(tMap);
}
/*--------------------------------------------------------------------------
 * Module level glue:
 */

// Module level dispatch table:
//
static PyMethodDef StateMachineClassMethods[] = {
    {"create", sm_create, METH_VARARGS, "Create a statemachine type"},
    {"isStateMachine", sm_isStateMachine, METH_VARARGS,
        "Is type id a statemachine?"},
    {"validNextStates", sm_validNextStates, METH_VARARGS,
        "Determine valid next states for type or variable"},
    {"getTransitionMap", sm_getTransitionMap, METH_VARARGS,
        "Return a statemachine's transition dict"},
    {NULL, NULL, 0, NULL}                           /* End sentinal */    
};

// Module initialization/registration:

/**
 * initenum
 *    Initialize the package
 */
PyMODINIT_FUNC
initstatemachine(void)
{
    PyObject* module;
    
    module = Py_InitModule3(
        "statemachine", StateMachineClassMethods, "VariableDB statemachine encapsulation"
    );
    if(module == NULL) {
        return;
    }
    /* Add the vardb exception type.  We'll map internal
     * C++ exceptions into this.
     */
    exception = PyErr_NewException(const_cast<char*>("enum.error"), NULL, NULL);
    Py_INCREF(exception);
    PyModule_AddObject(module, "error", exception);
}