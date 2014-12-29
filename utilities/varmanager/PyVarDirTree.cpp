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
# @file   PyVarDirTree.cpp
# @brief  Python encapsulation of the CVarDirTree class.
# @author <fox@nscl.msu.edu>
*/


#include <Python.h>
#include <vector>
#include <string>
#include <iostream>

#include "CVariableDb.h"
#include "CVarDirTree.h"
#include "PyVarDb.h"

static PyObject* exception;

/*---------------------------------------------------------------------------
 *  module level methods
 */

/**
 * dirtree_parsePath
 *    Encapsulate the CVarDirTree parsePath static method.
 */
static PyObject*
dirtree_parsePath(PyObject* self, PyObject* args)
{
    const char* path;
    std::vector<std::string> parsedPath;
    
    if(!PyArg_ParseTuple(args, "s", &path)) {
        PyErr_SetString(exception, "parsePath requires a path to parse");
        return NULL;
    }
    
    parsedPath = CVarDirTree::parsePath(path);
    PyObject* resultList = PyList_New(parsedPath.size());
    if (resultList == NULL) return NULL;
    
    for (int i = 0; i < parsedPath.size(); i++) {
        PyObject* element = PyString_FromString(parsedPath[i].c_str());
        if (element == NULL) return NULL;
        if(PyList_SetItem(resultList, i, element) == -1) {
            std::cerr << "Failed to add item " << i << " (" << parsedPath[i] << ")\n";
            return NULL;
        }
    }
    return resultList;
}
/**
 * dirtree_isRelative
 *   Python wrapper for CVarDirTree::isRelative static method
 */
static PyObject*
dirtree_isRelative(PyObject* self, PyObject* args)
{
    const char* path;
    std::vector<std::string> parsedPath;
    
    if(!PyArg_ParseTuple(args, "s", &path)) {
        PyErr_SetString(exception, "parsePath requires a path to parse");
        return NULL;
    }
    bool relative = CVarDirTree::isRelative(path);
    
    return PyBool_FromLong(relative ? 1L : 0L);
}

/**
 * Module level method dispatch table:
 */
static PyMethodDef DirTreeClassMethods[] = {
    {"parsePath", dirtree_parsePath, METH_VARARGS,
        "Break a filesystem path into path components."},
    {"isRelative", dirtree_isRelative, METH_VARARGS,
        "True if a filepath is relative"},
    {NULL, NULL, 0, NULL}
};

/*-----------------------------------------------------------------------------
 * Wrapping of actual CVarDirTree objects
 */


/**
 *  Implementation of 'special' methods
 */

/**
 * DirTree_new
 *    Create a DirTree object and do first round initialization of its
 *    data fields.  This is __new__ not __init__.
 *
 *  @param type - Type object for which we are creating a new instance.
 *  @param args - Parameters to the new method.
 *  @param kwargs - Keword parameters to the new method.
 *
 *  @return PyObject*  Actually a dirtree_DirTreeObject*
 */
static PyObject*
DirTree_new(PyTypeObject* type, PyObject* args, PyObject* kwargs)
{
    // Allocate the new object storage:
    
    dirtree_DirTreeObject* self = reinterpret_cast<dirtree_DirTreeObject*>(
        type->tp_alloc(type, 0)
    );
    if (self) {
        self->m_dir = NULL;                    // __init__ does construction.
        
        // Set the m_dirPyObj to an empty string for now:
        
        self->m_dbPyObj = PyString_FromString("");
        if (self->m_dbPyObj == NULL) {        // Failed to make the string destroy.
            Py_DECREF(self);
            return NULL;
        }
        
        return reinterpret_cast<PyObject*>(self);
    } else {
        PyErr_SetString(exception, "Failed to allocated DirTree storage");
        return NULL;
    }
}
/**
 * Dirtree_init
 *   Initialize the dirtree object.  The parameter passed to us must be a
 *   VarDb object (tp_name == "vardb.VarDb") which must have been initted so that
 *   there's a non null m_db member.  Otherwise we'll raise an exception with
 *   'an informative error message'.
 *
 *   @param self   - Pointer to the object being initialized.
 *   @param args   - tuple containing the non-keyword parameters.
 *   @param kwargs = The keyword/value args.
 *
 *   @note we should only have one arg and it should be an object.
 *
 *   @return int - 0 for success -1 for failure. If failure, the exception
 *                 will be raised.
 */
static int
DirTree_init(dirtree_DirTreeObject* self, PyObject* args, PyObject* kwds)
{
    PyObject*   pDbPyObj;                 // Python object of the database.
    
    // Extract the datatabase object, Ensure it's a vardb.VarDb and extract the
    // CVarDb pointer.
    
    if (!PyArg_ParseTuple(args, "O", &pDbPyObj)) {
        PyErr_SetString(exception, "Initialization requires a vardb.VarDb object");
        return -1;
    }
    if(std::string("vardb.VarDb") != pDbPyObj->ob_type->tp_name) {
        PyErr_SetString(exception, "Initialization requires a vardb.VarDb object");
        return -1;
    }
    vardb_VarDbObject* pVarDbPyObj = reinterpret_cast<vardb_VarDbObject*>(pDbPyObj);
    CVariableDb* db  = pVarDbPyObj->m_db;
    
    
    // Construct the dirtree object we need but inside a try/catch block where
    // the C++ std::exception objects get transformed into python exception raises.
    
    
    
    try {
        self->m_dir = new CVarDirTree(*db);
    }
    catch (std::exception& e) {
        PyErr_SetString(exception, e.what());
        return -1;
    }
    // We need to hold a reference to the database object to ensure it does not
    // get destroyed out from underneath us:
    
    Py_DECREF(self->m_dbPyObj);            // Done with existing one (from new e.g.).
    self->m_dbPyObj = pDbPyObj;
    Py_INCREF(self->m_dbPyObj);
    
    // Initialization correctly completed:
    
    return 0;
}
/**
 * DirTree_delete
 *    Delete the object:
 *    -   Delete the m_dir object
 *    -   Decrement our reference to the DB object so we don't stand in the way
 *        of its destruction if it's time.
 *    -   Deallocate the storage.
 *
 *  @param self - Pointer to our object.
 */
static void
DirTree_delete(dirtree_DirTreeObject* self)
{
    delete self->m_dir;
    Py_DECREF(self->m_dbPyObj);
    self->ob_type->tp_free(reinterpret_cast<PyObject*>(self));
}

/**
 * Implementation of 'ordinary' object methods
 */

/**
 * DirTree_getRootId
 *  @param self - Pointer to  my object.
 *  @param args - Pointer to tuple containing non keyword args.
 *  
 *  @return PyInt* - The id of the root directory.
 */
static PyObject*
DirTree_getRootId(PyObject* self, PyObject* args)
{
    dirtree_DirTreeObject* pSelf = reinterpret_cast<dirtree_DirTreeObject*>(self);
    if (PyTuple_Size(args) != 0) {
        PyErr_SetString(exception, "wdPath takes no parameters");
        return NULL;
    }    
    int id = pSelf->m_dir->getRootid();
    return PyLong_FromLong(id);
}

/**
 * DirTree_mkdir
 *   Create a new directory.  The directoy path can be either absolute or
 *   relative. std::exceptions thrown will be turned into raises of our
 *   exception
 *
 *  @param self - Pointer to  my object.
 *  @param args - Pointer to tuple containing non keyword args.
 *                These are:
 *                - path (required)
 *                - Boolean flag to control if missing elements are created (optional)
 *  @return Py_None
 */
static PyObject*
DirTree_mkdir(PyObject* self, PyObject* args)
{
    dirtree_DirTreeObject* pSelf = reinterpret_cast<dirtree_DirTreeObject*>(self);
    const char*            path;
    bool                   makePath(true);
    PyObject*              createPathObj;
    
    if (PyTuple_Size(args) == 2) {
        if (!PyArg_ParseTuple(args, "sO", &path, &createPathObj)) {
            PyErr_SetString(exception, "Could not convert objects in two parameter call");
            return NULL;
        }
        int iMakePath = PyObject_IsTrue(createPathObj);
        if (iMakePath == -1 || (!PyBool_Check(createPathObj))) {
            PyErr_SetString(exception, "Second parameter to mkdir must be a boolean-like param");
            return NULL;
        }
        makePath = iMakePath == 1 ? true : false;
    } else if (PyTuple_Size(args) == 1) {
        if (!PyArg_ParseTuple(args, "s", &path)) {
                PyErr_SetString(exception, "Directory path is a required parameter");
                return NULL;
        }        
    } else {
        PyErr_SetString(exception, "mkdir takes one or two parameters");
        return NULL;
    }
    
    
    
    // try to create the directory:
    
    try {
        pSelf->m_dir->mkdir(path, makePath);
    }
    catch(std::exception& e) {
        PyErr_SetString(exception, e.what());
        return NULL;
    }
    // Return None:
    
    Py_INCREF(Py_None);
    return Py_None;
}
/**
 * DirTree_cd
 *   Change the working directory of this object.
 * 
 *  @param self - Pointer to  my object.
 *  @param args - Pointer to tuple containing non keyword args.
 *                These are:
 *                - path (required)
 *  @return Py_None
 *  @note std::exceptions thrown are turned into raises of our python exception.
 */
static PyObject*
DirTree_cd(PyObject* self, PyObject* args)
{
    dirtree_DirTreeObject* pSelf = reinterpret_cast<dirtree_DirTreeObject*>(self);
    const char* path;
    
    if (!PyArg_ParseTuple(args, "s", &path)) {
        PyErr_SetString(exception, "cd requires a path string");
        return NULL;
    }
    try {
        pSelf->m_dir->cd(path);
    }
    catch(std::exception& e) {
        PyErr_SetString(exception, e.what());
        return NULL;
    }
    
    Py_INCREF(Py_None);
    return Py_None;
}
/**
 * DirTree_wdPath
 *    Return the current directory path as an absolute path string.
 *
 *  @param self - Pointer to  my object.
 *  @param args - Pointer to tuple containing non keyword args. Must be empty
 *  @return PyString - The current directory string as an absolute, canonicalized path.
 */
static PyObject*
DirTree_wdPath(PyObject* self, PyObject* args)
{
    dirtree_DirTreeObject* pSelf = reinterpret_cast<dirtree_DirTreeObject*>(self);
    if (PyTuple_Size(args) != 0) {
        PyErr_SetString(exception, "wdPath takes no parameters");
        return NULL;
    }
    std::string sResult;
    PyObject*   result;
    try {
        sResult = pSelf->m_dir->wdPath();
        result  = PyString_FromString(sResult.c_str());
    }
    catch (std::exception& e) {
        PyErr_SetString(exception, e.what());
        return NULL;
    }
    return result;
}

/**
 * DirTree_getwd
 *   Return information about the current working directory:
 *
 *  @param self - Pointer to  my object.
 *  @param args - Pointer to tuple containing non keyword args Must be empty
 *  @return PyDict - Information with the following keys:
 *                    - 'id' - Id of the node in the directory table.
 *                    - 'name' - Name of the last path element.
 *                    - 'parent' - id of the node's parent.
 */
static PyObject*
DirTree_getwd(PyObject* self, PyObject* args)
{
    dirtree_DirTreeObject* pSelf = reinterpret_cast<dirtree_DirTreeObject*>(self);
    if (PyTuple_Size(args) != 0) {
        PyErr_SetString(exception, "getwd takes no parameters");
        return NULL;
    }
    
    // Note that this method in CVarDirTree does not throw:
    
    CVarDirTree::DirInfo info = pSelf->m_dir->getwd();
    
    // Create and fill in the result dict.
    
    PyObject* result = PyDict_New();
    PyObject* element;
    if (!result) {
        PyErr_SetString(exception, "getwd could not create the return dict");
        return NULL;
    }
    
    // 'id'
    
    element = PyLong_FromLong(info.s_id);
    if (!element) {
        PyErr_SetString(exception, "getwd could not create 'id' element");
        return NULL;
    }
    if(PyDict_SetItemString(result, "id", element) == -1) {
        PyErr_SetString(exception, "Could not add 'id' to dict");
        return NULL;
    }
    
    // 'name'
    
    element = PyString_FromString(info.s_name.c_str());
    if (!element) {
        PyErr_SetString(exception, "getwd could not create 'name' value");
        return NULL;
    }
    if(PyDict_SetItemString(result, "name", element) == -1) {
        PyErr_SetString(exception, "getwd could not add 'name' to dict");
        return NULL;
    }
    
    // 'parent'
    
    element = PyLong_FromLong(info.s_parentId);
    if (!element) {
        PyErr_SetString(exception, "getwd could not create 'parent' value");
        return NULL;
    }
    if (PyDict_SetItemString(result, "parent", element) == -1) {
        PyErr_SetString(exception, "getwd could not add 'parent' to dict");
        return NULL;
    }
    
    return result;
}
/**
 * DirTree_ls
 *    List the contents of a directory
 *    @note In this implementation only directories exist in directories.
 *
 *  @param self - Pointer to  my object.
 *  @param args - Pointer to tuple containing non keyword args. Must be empty/
 *                
 *  @return tuple - containing names of items in the directory.
 */
static PyObject*
DirTree_ls(PyObject* self, PyObject* args)
{
    dirtree_DirTreeObject* pSelf = reinterpret_cast<dirtree_DirTreeObject*>(self);
    if (PyTuple_Size(args) != 0) {
        PyErr_SetString(exception, "ls takes no parameters");
        return NULL;
    }
    //  ls can't throw exceptions
    
    std::vector<CVarDirTree::DirInfo> listing = pSelf->m_dir->ls();
    
    PyObject* result = PyTuple_New(listing.size());   // Add by append.
    if (!result) {
        PyErr_SetString(exception, "ls - unable to allocate result tuple");
        return NULL;
    }
    
    for (int i = 0; i < listing.size(); i++) {
        PyObject* element = PyString_FromString(listing[i].s_name.c_str());
        if (!element) {
            PyErr_SetString(exception,"ls - unable to create a string for a subdir");
            return NULL;
        }
        PyTuple_SET_ITEM(result, i, element);
    }
    
    return result;
    
}
/**
 * DirTree_rm
 *    Remove a directory from the tree.
 *
 *  @param self - Pointer to  my object.
 *  @param args - Pointer to tuple containing non keyword args.
 *                - path to the directory to remove.
 *                
 *  @return Py_None.
 *  @note std::exception throws get mapped to a raise of our python exception.
 */
static PyObject*
DirTree_rmdir(PyObject* self, PyObject* args)
{
    dirtree_DirTreeObject* pSelf = reinterpret_cast<dirtree_DirTreeObject*>(self);
    const char*            path;
    
    if (!PyArg_ParseTuple(args, "s", &path)) {
        PyErr_SetString(exception, "rmdir - could not parse path string");
        return NULL;
    }
    
    try {
        pSelf->m_dir->rmdir(path);
    }
    catch (std::exception& e) {
        PyErr_SetString(exception, e.what());
        return NULL;
    }
    
    Py_INCREF(Py_None);
    return Py_None;
}




// Structs needed to define methods and type methods:

//    Method definitions.

static PyMethodDef DirTreeObjectMethods [] = {
    {"getRootId", DirTree_getRootId, METH_VARARGS,
        "Get the id of the root directory" },
    {"mkdir", DirTree_mkdir, METH_VARARGS,
        "Create a new directory"},
    {"cd", DirTree_cd, METH_VARARGS, "Change default directory"},
    {"wdPath", DirTree_wdPath, METH_VARARGS,
        "Get working directory path strings"},
    {"getwd", DirTree_getwd, METH_VARARGS,
        "Return information about the working directory"},
    {"ls", DirTree_ls, METH_VARARGS,
        "List directory contents"},
    {"rmdir", DirTree_rmdir, METH_VARARGS,
        "Remove a directory from the tree"},
    {NULL, NULL, 0, NULL}                /* End of method definition marker */   
    
};

//    Directory type struct.

static PyTypeObject dirtree_DirTreeType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "dirtree.DirTree",         /*tp_name*/
    sizeof(dirtree_DirTreeObject), /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)(DirTree_delete), /*tp_dealloc*/
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
    "Variable database Directory tree", /* tp_doc */
    0,		               /* tp_traverse */
    0,		               /* tp_clear */
    0,		               /* tp_richcompare */
    0,		               /* tp_weaklistoffset */
    0,		               /* tp_iter */
    0,		               /* tp_iternext */
    DirTreeObjectMethods,        /* tp_methods */
    0,                         /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)DirTree_init,      /* tp_init */
    0,                         /* tp_alloc */
    DirTree_new,                 /* tp_new */
};

/**
 * initdirtree
 *    Module initialization.
 */
PyMODINIT_FUNC
initdirtree(void)
{
    PyObject* module;
    
    // Create the module:
    
    module = Py_InitModule3("dirtree", DirTreeClassMethods, "CVarDirTree encapsulation");
    if(!module) {
        return;
    }
    
    
    // Errors detected by this module are reported via our exception object:
    
    exception = PyErr_NewException(const_cast<char*>("dirtree.error"), NULL, NULL);
    Py_INCREF(exception);
    PyModule_AddObject(module, "error", exception);
    
    // Add the DirTree type object which is used to construct new DirTree objects:
    
    if (PyType_Ready(&dirtree_DirTreeType) < 0) {
        return;                             // failed.
    }
    Py_INCREF(&dirtree_DirTreeType);
    PyModule_AddObject(
        module, "DirTree", reinterpret_cast<PyObject*>(&dirtree_DirTreeType)
    );
    
}