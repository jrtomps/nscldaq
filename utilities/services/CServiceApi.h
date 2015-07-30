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
# @file   CServiceApi.h
# @brief  Header for the services api.
# @author <fox@nscl.msu.edu>
*/

#ifndef CSERVICEAPI_H
#define CSERVICEAPI_H

#include <string>
#include <map>

class CVarMgrApi;

/**
 * @class CServiceApi
 *    Provides an API to the service description part of the variable database.
 */
class CServiceApi
{
private:
    CVarMgrApi*  m_pApi;
public:
    static const char* m_ServiceDir;             // So non API can find it.
public:
    CServiceApi(const char* reqUri);
    virtual ~CServiceApi();
    
public:
    bool exists();
    void create();
    void create(const char* name, const char* command, const char* host);
    void setHost(const char* name, const char* newHost);
    void setCommand(const char* name, const char* newCommand);
    void remove(const char* name);
    std::map<std::string, std::pair<std::string, std::string> > list();
    std::pair<std::string, std::string> list(const char* name);
    
    // Utilities:
    
private:
    void setDir(const char* name);
    std::string programPath(const char* name);
    void recursiveDelete(const char* path);
};
#endif