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
# @file   PyVarDb.h
# @brief  Header file for the VarDb python type. 
# @author <fox@nscl.msu.edu>
*/

/**
 * Expose the object struct so that other types that need it during init
 * can get what they need from it..
 */
#include <Python.h>

class CVariableDb;

/* Storage for a vardb.VarDb object. */

typedef struct {
    PyObject_HEAD
    /* Type-specific fields go here. */
    
    CVariableDb*    m_db;
} vardb_VarDbObject;