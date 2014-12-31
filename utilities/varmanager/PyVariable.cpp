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
# @file   PyVariable.cpp
# @brief  Python binding to the CVariable class.
# @author <fox@nscl.msu.edu>
*/

#include <Python.h>

#include "CVariableDb.h"
#include "CVarDirTree.h"
#include "CVariable.h"


#include "PyVarDb.h"
#include <string.h>
#include <exception>
#include <vector>

#include <iostream>

// Utility methods:

/**
 * @return bool true if obj is a database object.
*/
bool isDb(PyObject* obj) {
    return (strcmp("vardb.VarDb", obj->ob_type->tp_name) == 0);    
}
/**
 * @return bool true if obj is a DirTree object.
 */
bool isDirTree(PyObject* obj) {
    return (strcmp("dirtree.DirTree",  obj->ob_type->tp_name) == 0);
}



static PyObject* exception;                         // Exception class.

/**============================================================================
 *  Static (module) methods:
 */

/**
 *  Create a new variable.  This takes the form:
 *  variable.create(database, directory, path, type[, initial-value])
 *
 *   - database is  ...vardb.VarDb object.
 *   - directory is ...dirtree.Dirtree object or none.
 *   - path is a string indicating the variable path (all directories must exist)
 *   - type is a string indicating the correct variable type.
 *   - initial value is optional and is the initial value string.  This can be
 *     omitted or None in which case the variable value is the default value
 *     for that type.
 *   
 *
 *   The directory can be None.  In which case the path
 *   is interpreted to be root relative always.  Otherwisde relative paths
 *   are directory relative
 *
 *  @param self - I believe this is the module object(?)
 *  @param args - The argumetns of the method.
 *  @return PyObject*  - in this case Py_None
 */
static PyObject*
variable_create(PyObject* self, PyObject* args)
{
    PyObject*    db;
    PyObject*    dir;
    const char*  path;
    const char*  type;
    const char*  initialValue(0);
    
    // Parse the parameters:
    
    int nArgs = PyTuple_Size(args);
    if (nArgs == 4) {
        if(!PyArg_ParseTuple(args, "OOss", &db, &dir, &path, &type)) {
            PyErr_SetString(
                exception, "variable.create - Could not parse 4 parameters properly"
            );
            return NULL;
        }
    } else if (nArgs == 5) {
        if(!PyArg_ParseTuple(args, "OOsss", &db, &dir, &path, &type, &initialValue)) {
            PyErr_SetString(
                exception, "variable.create - Could not parse 5 parameters properly"
            );
            return NULL;
        }
    } else {
        PyErr_SetString(
            exception, "variable.create - accepts only 4 or 5 arguments"
        );
        return NULL;
    }
    
    // Check the types of the parameters:
    
    if(!isDb(db)) {
        PyErr_SetString(
            exception, "variable.create - first parameter must be a database"
        );
        return NULL;
    }
    
    if((dir != Py_None) && (!isDirTree(dir))) {
        PyErr_SetString(
            exception, "variable.create - second parameter must be None or a dirtree"
        );
        return NULL;
    }
    // Marshall the C++ object from the PyObjects:
    
    vardb_VarDbObject* vdbObj = reinterpret_cast<vardb_VarDbObject*>(db);
    
    // Create the variable -- map exceptions to python raises:
    
    try {
        if (dir == Py_None) {
            CVariable* v = CVariable::create(
                *(vdbObj->m_db), path, type, initialValue
            );
            delete v;
        } else {
            dirtree_DirTreeObject* dirObj =
                reinterpret_cast<dirtree_DirTreeObject*>(dir);
            CVariable* v = CVariable::create(
                *(vdbObj->m_db), *(dirObj->m_dir), path, type, initialValue
            );
            delete v;
        }
    }
    catch (std::exception& e) {
        PyErr_SetString(exception, e.what());
        return NULL;
    }
    
    Py_INCREF(Py_None);
    return Py_None;
}

/**
 * variable_ls
 *    Encapsulate CVariable::ls with a Python binding.  Note this is a module
 *    method not an  object method.  Usge:
 *
 *    variable.ls(db, dir[, path])
 *
 *    -   db   - is a database object.
 *    -   dir  - is a valid directory object.
 *    -   path - is an optional path that typically is relative to the dir.
 * 
 *
 *  @param self - I believe this is the module object(?)
 *  @param args - The argumetns of the method.
 *  @return PyObject*  - in this case Py_None
 *  @retval NULL  - An exception has been raised.
 *  @retval other - A list (possibly empty) of dicts, one for each variable in
 *                  the directory.  Each dict has the following keys:
 *                  - 'id'     - Id of the variable (primary key in table).
 *                  - 'name'   - Variable name.
 *                  - 'type'   - Type string of the variable (e.g. 'integer').
 *                  - 'typeId' - Id of the type in the type table.
 *                  - 'dirId'  - Id of the directory the variable lives in.
 */
PyObject*
variable_ls(PyObject* self, PyObject* args)
{
    PyObject*   db;
    PyObject*   dir;
    const char* path(0);
    
    // Parse parameters:
    
    int nArgs = PyTuple_Size(args);
    if(nArgs == 2) {
        if (!PyArg_ParseTuple(args, "OO", &db, &dir)) {
            PyErr_SetString(exception, "variable.ls - Unable to parse 2 aguments");
            return NULL;
        }
        
    } else if (nArgs == 3) {
        if (!PyArg_ParseTuple(args, "OOs", &db, &dir, &path)) {
            PyErr_SetString(exception, "variable.ls - Unable to parse 3 arguments");
            return NULL;
        }
    } else {
        PyErr_SetString(exception, "variable.ls - ls accepts nly 2 or 3 arguments");
        return NULL;
    }
    // validate parameter types:
    
    if(!isDb(db)) {
        PyErr_SetString(
            exception, "variable.ls - first parameter must be a databse object"
        );
        return NULL;
    }
    if (!isDirTree(dir)) {
        PyErr_SetString(
            exception,
            "variable.ls - Second parameter must be a directory tree object"
        );
        return NULL;
    }
    // Get the result raw vector.
    
    vardb_VarDbObject* dbObj      = reinterpret_cast<vardb_VarDbObject*>(db);
    dirtree_DirTreeObject* dirObj = reinterpret_cast<dirtree_DirTreeObject*>(dir);
    std::vector<CVariable::VarInfo> rawResult = CVariable::list(
        dbObj->m_db, *(dirObj->m_dir), path
    );
    
    // Marshall it into a list object:
    
    PyObject* result = PyList_New(0);
    for (int i = 0; i < rawResult.size(); i++) {
        CVariable::VarInfo v = rawResult[i];
        PyObject* itemDict = PyDict_New();
        
        PyDict_SetItemString(itemDict, "id",     PyInt_FromLong(v.s_id));
        PyDict_SetItemString(itemDict, "name",   PyString_FromString(v.s_name.c_str()));
        PyDict_SetItemString(itemDict, "type",   PyString_FromString(v.s_type.c_str()));
        PyDict_SetItemString(itemDict, "typeId", PyInt_FromLong(v.s_typeId));
        PyDict_SetItemString(itemDict, "dirId",  PyInt_FromLong(v.s_dirId));
        
        PyList_Append(result, itemDict);
    }
    
    // Return the list object to the caller:
    
    return result;
}

/**
 * variable_destroy
 *    Destroys variables.  This is quite polymorphic so the argument
 *    parsing is a bit interesting:
 *
 *    variable.destroy(db, id)
 *    -   db - a database object.
 *    -   id - an variable id (primary key).
 *
 *    variable.destroy(db, dir | None, path)
 *    -  db          - A database object.
 *    -  dir | None  - A base directory or None.
 *    -  path        - Path relative to dir (or / if dir is None).
 *
 */
PyObject*
variable_destroy(PyObject* self, PyObject* args)
{
    vardb_VarDbObject*       pDb(0);
    dirtree_DirTreeObject*   pDir(0);
    PyObject*                pVar(0);
    const char*              pPath(0);
    int                      id(-1);
    
    int nArgs = PyTuple_Size(args);
    if (nArgs == 0) {
        PyErr_SetString(
            exception, "variable.destroy - needs at least one argument"
        );
        return NULL;
    }
    
    // Because of how polymorphic this is we need to access the individual
    // tuple elements rather than doing a parse.
    // This also needs to  all be done in a try/catch block to map
    // exceptions into python raises:
    
    try {
        
        PyObject* pObj = PyTuple_GET_ITEM(args, 0);
        
        // TODO:  First parameter is a variable object!!!
        
        if(isDb(pObj)) {
            pDb  = reinterpret_cast<vardb_VarDbObject*>(pObj);
            
            // If the first object is a db, could be two or three parameters:
            
            if (nArgs == 2) {
    
                // arg1 could be an integer or a path string:
                
                pObj = PyTuple_GET_ITEM(args, 1);
                if (PyInt_Check(pObj)) {
                    id = PyInt_AsLong(pObj);
                    CVariable::destroy(*(pDb->m_db), id);
                } else {
                    // Everything has a string representation so
                    
                    pPath = PyString_AsString(pObj);
                    if  (pPath) {
                        CVariable::destroy(*(pDb->m_db), pPath);
                    } else {
                        return NULL;                // Already a raised error.
                    }
                }
    
            } else if (nArgs ==3) {
                // 3 args must be db, dir, string:
                
                pObj = PyTuple_GET_ITEM(args,1);
                if (isDirTree(pObj)) {
                    pDir = reinterpret_cast<dirtree_DirTreeObject*>(pObj);
                    pPath = PyString_AsString(PyTuple_GET_ITEM(args, 2));
                    if (pPath) {
                        CVariable::destroy(*(pDb->m_db), *(pDir->m_dir), pPath);
                    } else {
                        return NULL;
                    }
                } else {
                    PyErr_SetString(
                        exception,
                        "variable.destroy - 3 args - second must be a directory"
                    );
                    return NULL;
                }
            } else {
                PyErr_SetString(
                    exception,
                    "variable.destroy - If first arg is a database must be either 2 or three args"
                );
                return NULL;
            }
            
        } else {
            PyErr_SetString(
                exception,
                "variable.destroy - for this call first parameter must be a database"
            );
            return NULL;
        }
    }
    catch (std::exception& e) {
        PyErr_SetString(exception, e.what());
        return NULL;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

/*=============================================================================
 * Object methods (for the Variable instances):
 */

/**
 * Canonicals:
 */

/**
 * Variable_new
 *    Constructor (__new__ method).  Allocates storage and sets initial values
 *    for member variables.
 *
 *  @param type - Type object - must be for a variable.Variable object.
 *  @param args - Any additional positional parameters to __new__ (none)
 *  @param kwds - Any additional keyword parameters to __new__ (none)
 *  @return PyObject* Pointer to the allocated/initialized object.
 */
static PyObject*
Variable_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
    // Allocate the object:
    
    variable_VariableObject* self = reinterpret_cast<variable_VariableObject*>(
        type->tp_alloc(type, 0)
    );
    if (self) {
        self->m_var = NULL;
        return reinterpret_cast<PyObject*>(self);
    } else {
        PyErr_SetString(exception, "Could not allocate CVariable container object");
        return NULL;
    }
}
/**
 * Variable_init
 *   The __init__ method of the variable.Variable type.  There are several
 *   parameterizations that are all polymorphic:
 *   -  Variable(db, path=path)
 *   -  Variable(db, dir=dir, path=path)
 *   -  Variable(db, id=id)
 *
 *   Where the parameters in the calls above are:
 *   -   db   - A variable database in which the variable is already defined
 *              (see the module level create methods for establishing a variable)
 *   -   dir  - A directory which, if present sets a base directory from which
 *              relative paths are computed.
 *   -   path - Path to the diretoctory (relative or absolute).
 *   -   id   - The primary key of the variable in the variables table.
 *
 * @param  self  - Pointer to the object being initialized.
 * @param  args  - Positional parameters.  The first parameter must be a database
 *                 object...there can be only one positional parameter.
 * @param  kwds  - Dict containing keywords parameters. These can be:
 *                - 'dir' - optional the directory relative to which paths are
 *                   computed.
 *                - 'path' optional - path to the variable.
 *                - 'id'   optional - id of the variable.
 *  @note - one, and only one, of 'path', 'id' must be present.
 *  @note - if 'id' is present, 'dir' may not be.
 *  
 *  @return int  0 for success, -1 for failure with an exception raised.
 *  @note On success the self's m_var pointer is filled in with a pointer to a
 *        good CVariable object.
 *  @note C++ exceptions are converted into variable.error raises.
 *  
 */
static int
Variable_init(variable_VariableObject* self, PyObject* args, PyObject* kwds)
{
    vardb_VarDbObject*     db(0);
    dirtree_DirTreeObject* dir(0);
    const char*            pPath(0);
    int                    id(-1);
    static const char*     kwlist[] = {"db", "dir", "path", "id", NULL};
    
    // Let's see if we can parse all this properly:
    
    if(!PyArg_ParseTupleAndKeywords(
        args, kwds, "O|Osi", const_cast<char**>(kwlist), &db, &dir, &pPath, &id
    )) {
        return -1;
    }
    
    // Enforce parameter constraints:
      // db must be a db:
    
    if(!isDb(reinterpret_cast<PyObject*>(db))) {
        PyErr_SetString(
            exception, "Variable.__init__ - position param must be a database"
        );
        return -1;
    }
      // If dir is present it must be a directory:
      
    if(dir && (!isDirTree(reinterpret_cast<PyObject*>(dir)))) {
        PyErr_SetString(
            exception,
            "Variable.__init__ - 'dir' value must be a directory tree"
        );
        return -1;
    }
       // cant' have both id and path:
       
    if ((id != -1) && pPath) {
        PyErr_SetString(
            exception,
            "Variable.__init__ only one of 'dir' and 'path' is allowed."
        );
        return -1;
    }
       // Can't have id and dir:
    
    if ((id != -1) && dir) {
        PyErr_SetString(
            exception,
            "Variable.__init__ - 'id' makes 'dir' invalid"
        );
        return -1;
    }
    // Instantiate the variable (in a try/catch block) and set its value
    
    CVariable* pVar;
    try {
        if(!dir && pPath) {
            pVar = new CVariable(*db->m_db, pPath);
        }
        else if (dir &&  pPath) {
            pVar = new CVariable(*db->m_db, *dir->m_dir, pPath);
        } else if (id != -1) {
            pVar = new CVariable(*db->m_db, id);
        } else {
            PyErr_SetString(
                exception,
                "Variable.__init__  - invalid __init__ keyword combination"
            );
            return -1;
        }
        
        
    }
    catch(std::exception& e) {
        PyErr_SetString(exception, e.what());
        return -1;
    }
    self->m_var  = pVar;
    return 0;
    
}
/**
 * variable_Delete
 *    The __delete__ destructor - free the CVariable object:
 *
 *    @param self - Pointer to our python object wrapper.
 */
static void
variable_Delete(variable_VariableObject* self)
{
    delete self->m_var;
    self->ob_type->tp_free(reinterpret_cast<PyObject*>(self));
}

/**
 * instance methods on fully constructed objects:
 */

/**
 * varinstance_getid
 *    Return the Id of self.
 *
 * @param self - Pointer to the object being queried.
 * @param args - Positional parameters (must not be any)
 * @return PyIntObject
 */
static PyObject*
varinstance_getid(PyObject* self, PyObject* args)
{
    if (PyTuple_Size(args) != 0) {
        PyErr_SetString(exception, "Variable.getid - takes no parameters");
        return NULL;
    }
    variable_VariableObject* selfObj =
        reinterpret_cast<variable_VariableObject*>(self);
    PyObject* result = PyInt_FromLong(selfObj->m_var->getId());
    
    return result;
}


/**
 * varinstance_set
 *    Implements the set method of a variable instance.  This
 *    sets a new value for a variable.  All the constraint checking etc. is done
 *    by the underyling CVariable object.  We map any exceptions thrown to
 *    a raise of our exception object.
 * @param self - Pointer to the object being queried.
 * @param args - Positional parameters (must not be any)
 * @return Py_None
 * 
 */
static PyObject*
varinstance_set(PyObject* self, PyObject* args)
{
    variable_VariableObject* pSelf =
        reinterpret_cast<variable_VariableObject*>(self);
    const char*  pValue(0);
    
    // Parse the arg -- only a value string
    
    if(!PyArg_ParseTuple(args, "s", &pValue)) {
        return NULL;                    // already a raise done.
    }
    
    try {
        pSelf->m_var->set(pValue);
    }
    catch (std::exception& e) {
        PyErr_SetString(exception, e.what());
        return NULL;
    }
    
    // Return None:
    
    Py_INCREF(Py_None);
    return Py_None;
}

/**
 * varinstance_get
 *    Return the current value of the variable.
 *    
 * @param self - Pointer to the object being queried.
 * @param args - Positional parameters (must not be any)
 * @return PyString - the string value of the variable.
 * 
 */
static PyObject*
varinstance_get(PyObject* self, PyObject* args)
{
    // there cannot be any parameters:
    
    if (PyTuple_Size(args) > 0) {
        PyErr_SetString(exception, "variable.get - takes no parameters");
        return NULL;
    }
    variable_VariableObject* pSelf =
        reinterpret_cast<variable_VariableObject*>(self);
    std::string value = pSelf->m_var->get();
    
    // Create and return the result:
    
    PyObject* result = PyString_FromString(value.c_str());
    return result;
}

/**
 * varinstance_getName
 *    returns the name (without leading directory path) of a variable.
 *
 * @param self - Pointer to the object being queried.
 * @param args - Positional parameters (must not be any)
 * @return PyString - the string name of the variable.
 */
static PyObject*
varinstance_getName(PyObject* self, PyObject* args)
{
    // No parameters allowed:
    
    if (PyTuple_Size(args) > 0) {
        PyErr_SetString(exception, "variable.get - takes no parameters");
        return NULL;
    }
    variable_VariableObject* pSelf =
        reinterpret_cast<variable_VariableObject*>(self);
        
    std::string name = pSelf->m_var->getName();
    
    // Produce the result object.
    
    PyObject* result = PyString_FromString(name.c_str());
    return result;
}

PyObject*
varinstance_getDirectory(PyObject* self, PyObject* args)
{
    if(PyTuple_Size(args) > 0) {
        PyErr_SetString(
            exception, "variable.getDirectory - does not take arguments"
        );
        return NULL;
    }
     variable_VariableObject* pSelf =
        reinterpret_cast<variable_VariableObject*>(self);
    std::string dir = pSelf->m_var->getDirectory();
    
    PyObject* result = PyString_FromString(dir.c_str());
    return result;
    
}
/*------------------------------------------------------------------------------
 *  Glue:
/**
 * Module level method dispatch table:
 */
static PyMethodDef VariableClassMethods[] = {
    {"create" , variable_create, METH_VARARGS,
        "Create a new variable"},
    {"ls", variable_ls, METH_VARARGS, "List variables in a directory"},
    {"destroy", variable_destroy, METH_VARARGS, "Destroy a variable"},
    {NULL, NULL, 0, NULL}
};


/**
 * Object level dispatch table:
 */
static PyMethodDef VariableObjectMethods[] = {
    {"getId", varinstance_getid, METH_VARARGS, "Get object's id"},
    {"set",   varinstance_set,   METH_VARARGS, "Set variable value"},
    {"get",   varinstance_get,   METH_VARARGS, "Get variable value"},
    {"getName", varinstance_getName, METH_VARARGS, "Get variable name"},
    {"getDirectory", varinstance_getDirectory, METH_VARARGS,
        "Get path of directory containing variable"},
    {NULL, NULL, 0, NULL}
};
// Variable type object:

static PyTypeObject vardb_VariableType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "variable.Variable",             /*tp_name*/
    sizeof(variable_VariableObject), /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)(variable_Delete), /*tp_dealloc*/
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
    "Variable database Variable objects", /* tp_doc */
    0,		               /* tp_traverse */
    0,		               /* tp_clear */
    0,		               /* tp_richcompare */
    0,		               /* tp_weaklistoffset */
    0,		               /* tp_iter */
    0,		               /* tp_iternext */
    VariableObjectMethods,     /* tp_methods */
    0,                         /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)Variable_init,      /* tp_init */
    0,                         /* tp_alloc */
    Variable_new,                 /* tp_new */
};


/**
 * Module initialization:
 */

/**
 * initvariable
 *    Module initialization.
 */
PyMODINIT_FUNC
initvariable(void)
{
    PyObject* module;
    
    // Create the module:
    
    module = Py_InitModule3("variable", VariableClassMethods, "CVariable encapsulation");
    if(!module) {
        return;
    }
    
    
    // Errors detected by this module are reported via our exception object:
    
    exception = PyErr_NewException(const_cast<char*>("variable.error"), NULL, NULL);
    Py_INCREF(exception);
    PyModule_AddObject(module, "error", exception);
    
    // Add the Variable type:
    
    if (PyType_Ready(&vardb_VariableType) >= 0) {
        Py_INCREF(&vardb_VariableType);
        PyModule_AddObject(module, "Variable", reinterpret_cast<PyObject*>(&vardb_VariableType));
    }
}