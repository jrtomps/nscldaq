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
# @file   CVarMgrFileApi.h
# @brief  Variable manager API for file databases (no server).
# @author <fox@nscl.msu.edu>
*/

#ifndef CVARMGRFILEAPI_H
#define CVARMGRFILEAPI_H
#include "CVarMgrApi.h"

class CVariableDb;
class CVarDirTree;

class CVarMgrFileApi : public CVarMgrApi
{
    CVariableDb*   m_pDb;
    CVarDirTree*   m_pWd;
    
    // Canonicals:
public:
    CVarMgrFileApi(const char* pFilePath);
    virtual ~CVarMgrFileApi();
    
    // Interface implemented:
    
public:
    virtual void cd(const char* path = "/") ;
    virtual std::string getwd() ;
    virtual void mkdir(const char* path)    ;
    virtual void rmdir(const char* path)    ;
    virtual void declare(const char* path, const char* type, const char* initial=0) ;
    virtual void set(const char* path, const char* value) ;
    virtual std::string get(const char* path) ;
    virtual void defineEnum(const char* typeName, EnumValues values) ;
    virtual void defineStateMachine(const char* typeName, StateMap transitions) ;
};

#endif