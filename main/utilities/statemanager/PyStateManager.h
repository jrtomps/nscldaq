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
# @file   PyStateManager.h
# @brief  Header for classes 'derived' from the state manager.
# @author <fox@nscl.msu.edu>
*/
#include <Python.h>

#ifndef PYSTATEMANAGER_H
#define PYSTATEMANAGER_H

/*
 * State manager Api Object storage:
 */
typedef struct {
    PyObject_HEAD
    
    CStateManager*  m_pApi;
    CStateProgram*  m_pPrograms;
    
} statemanager_ApiObject;



extern PyMethodDef* StateManagerMethods;

#endif