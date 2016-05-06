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
# @file   PyVarDb.cpp
# @brief  Python bindings to CVariableDb
# @author <fox@nscl.msu.edu>
*/

#include <Python.h>
#include "PyVarDb.h"
#include <CVariableDb.h>
#include <exception>
#include <iostream>

/** Module specific exception */

static PyObject* exception;

/*-----------------------------------------------------------------------------
 *  Module level methods (commands).
 */

/**
 * vardb_create
 *    Class level method to create a new variable database.
 *
 *    Calling convention is standard Python verb convention.
 */
static PyObject*
vardb_create(PyObject* self, PyObject* args)
{
    const char* pDbPath;
    
    if (!PyArg_ParseTuple(args, "s", &pDbPath)) {
        return NULL;
    }
    
    try {
        CVariableDb::create(pDbPath);
    }
    catch (std::exception& e) {
        PyErr_SetString(exception, e.what());
    }
    
    Py_INCREF(Py_None);
    return Py_None;
}


/**
 * Module level methods.  These will be the CVariableDb class level methods.
 */

static PyMethodDef VarDbClassMethods[] =  {
    {"create", vardb_create, METH_VARARGS,
       "Create a new variable database"},
    {NULL, NULL, 0, NULL}                /* End of method definition marker */
};


/*-----------------------------------------------------------------------------
 * VarDb type/class.
 */


/*---------------------------------------------------------------------------
 * VarDb methods:
 */

/**
 * VarDb_new - constructor
 *
 *   @param type - Pointer to the type data structure.
 *   @param args - Argument list passed to constructor.
 *   @param kwds - keyword parameters passsed to constructor.
 *
 *   @return PyObject* pointer to the newly created object's data structure.
 */

static PyObject*
VarDb_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
    
    // Allocate the actual object:
    
    vardb_VarDbObject* self = reinterpret_cast<vardb_VarDbObject*>(
        type->tp_alloc(type, 0)
    );
    if (self) {
        self->m_db = NULL;                // _init_ will create the db.
        return reinterpret_cast<PyObject*>(self);
    } else {
        PyErr_SetString(exception, "Could not allocate CVariableDb container object");
        return NULL;
    }
}
/**
 * VarDb_init
 *   Initialize the contents of an item.
 *
 * @param self - Pointer to our own objet.
 * @param args - Parameter list to init (without the self).
 * @param kwargs  - Keyword arguments to init.
 */
static int
VarDb_init(vardb_VarDbObject* self, PyObject* args, PyObject* kwds)
{
    // Get the name of the db we are constructing on:
    
    
    const char* pDbName;
    
    if (!PyArg_ParseTuple(args, "s", &pDbName)) {
        PyErr_SetString(exception, "Need a db filename to create a VarDb");
        return -1;
    }
    // Destroy any pre-exising m_db:
    // and replace it with the requested one:
    
    delete self->m_db;
    try {
        self->m_db = new CVariableDb(pDbName);
    }
    catch(std::exception& e) {
        PyErr_SetString(exception, e.what());
        return -1;
    }
    return 0;


}
/**
* VarDb_delete Destructor
* 
* @param self - Pointer to the object to destroy.
*/
static void
VarDb_delete(vardb_VarDbObject* self)
{
    delete self->m_db;               // Destroy the database object
    self->ob_type->tp_free(reinterpret_cast<PyObject*>(self));   // destroy object storage.
}





/*  Type object that describes the methods etc. of objects of VarDbType: */

static PyMethodDef VarDbObjectMethods [] = {
     {NULL, NULL, 0, NULL}                /* End of method definition marker */   
};


static PyTypeObject vardb_VarDbType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "vardb.VarDb",             /*tp_name*/
    sizeof(vardb_VarDbObject), /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)(VarDb_delete), /*tp_dealloc*/
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
    "Variable database objects", /* tp_doc */
    0,		               /* tp_traverse */
    0,		               /* tp_clear */
    0,		               /* tp_richcompare */
    0,		               /* tp_weaklistoffset */
    0,		               /* tp_iter */
    0,		               /* tp_iternext */
    VarDbObjectMethods,        /* tp_methods */
    0,                         /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)VarDb_init,      /* tp_init */
    0,                         /* tp_alloc */
    VarDb_new,                 /* tp_new */
};




/*----------------------------------------------------------------------------
 * Module initialization
 */

/**
 *  initvardb
 *     This is called by Python when the module is loaded to initialize
 *     the module.
 */
PyMODINIT_FUNC
initvardb(void)
{
    PyObject* module;
    
    
    /* Create the module object  */
    
    module  = Py_InitModule3("vardb", VarDbClassMethods, "CVariableDb encapsulation");
    if (module == NULL) {
        return;                             /* Failure */
    }
    /*
     * Add the VarDbType to the module:
     */
    
    if (PyType_Ready(&vardb_VarDbType) < 0) {
        return;
    }
    Py_INCREF(&vardb_VarDbType);
    PyModule_AddObject(module, "VarDb", reinterpret_cast<PyObject*>(&vardb_VarDbType));
    
    /* Add the vardb exception type.  We'll map internal
     * C++ exceptions into this.
     */
    exception = PyErr_NewException(const_cast<char*>("vardb.error"), NULL, NULL);
    Py_INCREF(exception);
    PyModule_AddObject(module, "error", exception);
    
}