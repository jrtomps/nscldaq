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
# @file   CVarMgrServerApi.h
# @brief  Class that implements CVarMgrApi for connections with servers.
# @author <fox@nscl.msu.edu>
*/
#ifndef CVARMGRSERVERAPI_H
#define CVARMGRSERVERAPI_H
#include "CVarMgrApi.h"

#include <zmq.hpp>

/**
 * @class CVarMgrServerApi
 *
 * Implements the CVarMgrApi over connections to a varmgrServer program.
 * In this case, the server is assumed to be connected to us via a
 * REQ/REP zmq socket.
 */
class CVarMgrServerApi : public CVarMgrApi
{
    // object data  TODO:  Is it important to not proliferate zmq::context_t
    //                     objects?
private:
    static zmq::context_t* m_pContext;
    zmq::socket_t*  m_pSocket;
    std::string     m_wd;
    
    // Canonicals:
public:    
    CVarMgrServerApi(const char* host, int port);
    virtual ~CVarMgrServerApi();
    
public:

    virtual void cd(const char* path = "/") ;
    virtual std::string getwd() ;
    virtual void mkdir(const char* path)    ;
    virtual void rmdir(const char* path)    ;
    virtual void declare(const char* path, const char* type, const char* initial=0) ;
    virtual void set(const char* path, const char* value) ;
    virtual std::string get(const char* path) ;
    virtual void defineEnum(const char* typeName, CVarMgrApi::EnumValues values) ;
    virtual void defineStateMachine(const char* typeName, CVarMgrApi::StateMap transitions) ;
    virtual std::vector<std::string> ls(const char* path=0);
private:
    bool existingDirectory(std::string dirname);
    std::string transaction(std::string command, std::string data1, std::string data2);
    void sendMessage(std::string command, std::string data1, std::string data2);
    std::pair<std::string, std::string> getReply();
    std::string join(const CVarMgrApi::EnumValues& values, char sep);
    std::string join(const std::set<std::string>& values, char sep);
    std::string makePath(const char* path);
    std::string canonicalize(std::string path);
    std::set<std::string> processDirList(std::string dirlist);
    

};

#endif