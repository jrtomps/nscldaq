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
# @file   PyEnum.cpp
# @brief  Python module to support enumerated types.
# @author <fox@nscl.msu.edu>
*/

#include <Python.h>
#include "PyVarDb.h"
#include <CVariableDb.h>
#include <exception>
#include <vector>
#include <CEnumeration.h>

static PyObject* exception;


/*--------------------------------------------------------------------------
 * Utlities that factor out duplicate code:
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
 * stringVectorToSet
 *   Create a Python set from a vector of strings.
 *   @param values - std::vector<std::string> of values
 *   @return PyObject* - set whose members are the members of the vector.
 */
static PyObject*
stringVectorToSet(std::vector<std::string> values)
{
    PyObject* result = PySet_New(NULL);
    for (int i =0; i < values.size(); i++) {
        PyObject* element = PyString_FromString(values[i].c_str());
        PySet_Add(result, element);
    }
    return result;
}

/*--------------------------------------------------------------------------
 * module level methods
 */

/**
 * enum_create
 *    Create a new enumerated type.
 *  @param self - the module object
 *  @param args - Argument tuple object.  This should have a database, a typename
 *                and a tuple of valid values.
 *  @return PyObject* - Integer object containing the new type id.
 */
static PyObject*
enum_create(PyObject* self, PyObject* args)
{
    PyObject* db;
    PyObject* values;
    const char* typeName;
    
    if (!PyArg_ParseTuple(args, "OsO", &db, &typeName, &values)) {
        return raise("enum.create - could not parse parameters");
    }
    // db must be a database

    CVariableDb* pDb = variableDbFromObj(db);
    if (!pDb) return NULL;

    // values object must be iterable
    
    PyObject* valueIterator = PyObject_GetIter(values);
    if (!valueIterator) {
        return raise( "enum.create - last parameter must be iterable");

    }
    
    // Marshall the values tuple into a vector:
    
    std::vector<std::string> vValues;
    PyObject* v;
    
    while (v = PyIter_Next(valueIterator)) {
        vValues.push_back(std::string(PyString_AsString(v)));
        Py_DECREF(v);
    }
    Py_DECREF(valueIterator);


    // Create the new type mapping exceptions -> raises.
    
    int id;
    try {
        id = CEnumeration::create(*pDb, typeName, vValues);
    }
    catch(std::exception& e) {
        return raise(e.what());
    }

   
    
    return PyInt_FromLong(id);
    
}
/**
 * enum_id
 *    Determines and returns the id of an enumerated type.
 * @param self - Module object.
 * @param args - Tuple containing the arguments to the method; database and
 *               type name.
 * @return PyObject* - integer object containing the id of the type.
 */
static PyObject*
enum_id(PyObject* self, PyObject* args)
{
    PyObject* dbObj;
    const char* typeName;
    
    if (!PyArg_ParseTuple(args, "Os", &dbObj, &typeName)) {
        return raise("Could not parse parameters");
    }
    // dbObj must be a database:
    
    CVariableDb* pDb = variableDbFromObj(dbObj);
    if (!pDb) return NULL;
    
    
    // Run the id method in a try/catch to convert C++ exceptions to raises
    
    int id;
    try {
        id = CEnumeration::id(*pDb, typeName);
    }
    catch (std::exception& e) {
        return raise( e.what());
    }
    return PyInt_FromLong(id);
    
}
/**
* enum_listValues
*   Return a set with the legal values for an enumerated type.
* @param self - module (this is a module level method).
* @param args - Parameters.  These must be a database and a type name that is
*               an enum.
* @return PyObject* - a set whose members are the legal values.
*/
static PyObject*
enum_listValues(PyObject* self, PyObject* args)
{
    PyObject*   dbObj;
    const char* typeName;
    if (!PyArg_ParseTuple(args, "Os", &dbObj, &typeName)) {
        return raise("Unable to parse arguments");
    }
    
    // Require dbObj be a data base and extract the CVariableDb* from it:

    CVariableDb* pDb = variableDbFromObj(dbObj);
    if (!pDb) return NULL;    
    
    // Get the values in side a try/catch to convert exceptions to python raises
    
    std::vector<std::string> resultSet;
    try {
        resultSet = CEnumeration::listValues(*pDb, typeName);
    }
    catch (std::exception& e) {
        return raise(e.what());
        return NULL;
    }
    
    // Marshall the set values into the final python result set object:
    
    PyObject* result = stringVectorToSet(resultSet);
    
    return result;
}
/**
 * enum_listEnums
 *   Returns a list of the enumerated types.
 * @param self - Module object (this is a module level method).
 * @param args - Parameters - just a database object.
 * @return PyObject* set of strings that are the enumerated names.
 */
static PyObject*
enum_listEnums(PyObject* self, PyObject* args)
{
    PyObject* dbObject;
    if(!PyArg_ParseTuple(args, "O", &dbObject)) {
        return raise("Could not parse parameters");
        return NULL;
    }
    // Check that this is a database object and marshall the db from it if so:
    
    if (!isDb(dbObject)) {
        return raise("Parameter must be a database object");
        return NULL;
    }
    vardb_VarDbObject* pDbObject =
        reinterpret_cast<vardb_VarDbObject*>(dbObject);
    CVariableDb* pDb = pDbObject->m_db;
    
    // Get the enum values mapping exceptions -> raises
    
    std::vector<std::string> enums;
    try {
        enums = CEnumeration::listEnums(*pDb);
    }
    catch (std::exception& e) {
        PyErr_SetString(exception, e.what());
        return NULL;
    }
    // Marshall the vector to a set:
    
    PyObject* result = stringVectorToSet(enums);
    
    return result;
}

/*---------------------------------------------------------------------------
 * Glue data structures:
 */

static PyMethodDef EnumClassMethods[] = {
    {"create", enum_create, METH_VARARGS, "Create an enum data type"},
    {"id",     enum_id,     METH_VARARGS, "Get id of an enum"},
    {"listValues",enum_listValues, METH_VARARGS,
        "List legal values for an enum type"},
    {"listEnums", enum_listEnums, METH_VARARGS,
        "List the enumerator types"},
    {NULL, NULL, 0, NULL}                           /* End sentinal */    
};

/**
 * initenum
 *    Initialize the package
 */
PyMODINIT_FUNC
initenum(void)
{
    PyObject* module;
    
    module = Py_InitModule3(
        "enum", EnumClassMethods, "VariableDB enum encapsulation"
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