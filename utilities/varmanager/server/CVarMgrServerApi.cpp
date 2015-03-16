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
# @file   CVarMgrServerApi.cpp
# @brief  Implement the API based on a ZMQ server connection.
# @author <fox@nscl.msu.edu>
*/
#include "CVarMgrServerApi.h"
#include <stdio.h>
#include <string.h>
#include <string>

/**
 * constructor
 *    - Create the zmq context and socket.
 *    - set the wd to "/"
 *
 *  @param[in] server - Name of the server (this could also be e.g. "127.0.0.1").
 *  @param[in] port   - Server's request port.
 */
CVarMgrServerApi::CVarMgrServerApi(const char* server, int port) :
    m_pContext(0),
    m_pSocket(0),
    m_wd("/")
{
    char* pUri = new char[strlen(server) + 1 + 20];
    
    try {
        m_pContext = new zmq::context_t(1);
        m_pSocket  = new zmq::socket_t(*m_pContext, ZMQ_REQ);
        
        
        sprintf(pUri, "tcp://%s:%d", server, port);
        m_pSocket->connect(pUri);
    }
    catch (...) {
        delete []pUri;
        delete m_pSocket;
        delete m_pContext;
        throw;
    }
    
    
    delete []pUri;    
}
    
CVarMgrServerApi::~CVarMgrServerApi()
{
    delete m_pSocket;
    delete m_pContext;
}
    
    // Interface implemented:
    

void CVarMgrServerApi::cd(const char* path) { }
/**
 * getwd
 *    Return the current working directory string.
 *    The cd method is assumed to do the verification so
 *    we just return the member string.
 *    
 *  @return std::string - value of the current working directory.
 */
std::string
CVarMgrServerApi::getwd()
{
    return m_wd;
}
/**
 * mkdir
 *    Make a new directory.
 *
 *  @param[in] path - the path to the new directory.
 */
void
CVarMgrServerApi::mkdir(const char* path)
{
    
    transaction("MKDIR", path, "");
    
}
/**
 * rmdir
 *    Remove the specified directory.
 *
 *  @param[in] path - The path to remove.
 */
void
CVarMgrServerApi::rmdir(const char* path)
{
    transaction("RMDIR", path, "");
}
/**
 * declare
 *   Create a variable of a known data type in an existing directory with,
 *   optionally, an initial value.
 *
 * @param[in] path - Path of the variable.
 * @param[in] type - Type of variable to create (e.g. "integer").
 * @param[in] initial - If non null the initial value of the variable.
 */
void
CVarMgrServerApi::declare(const char* path, const char* type, const char* initial)
{
    // Build up the data 2 parameter:
    
    std::string data2 = type;
    data2     += '|';
    if (initial) {
        data2 += initial;
    }
    transaction("DECL", path, data2);
}

void CVarMgrServerApi::set(const char* path, const char* value) { }
std::string CVarMgrServerApi::get(const char* path) { return ""; }
void CVarMgrServerApi::defineEnum(const char* typeName, EnumValues values) { }
void CVarMgrServerApi::defineStateMachine(const char* typeName, StateMap transitions) { }

    // Utilities:
    
bool CVarMgrServerApi::existingDirectory(std::string path) {return true; }

/**
 * transaction
 *   Performas a transaction with the database server:
 *
 * @param[in] command - operation to perform.
 * @param[in] data1   - First data field.
 * @param[in] data2   - Second data field
 *
 * @return std::string - data part of the return.
 * @throw CException - if the reply status is not OK.
 */
std::string
CVarMgrServerApi::transaction(std::string command, std::string data1, std::string data2)
{
    sendMessage(command, data1, data2);
    std::pair<std::string, std::string> reply = getReply();
    if (reply.first != "OK") {
        throw CVarMgrApi::CException(reply.second);
    }
    return reply.second;
}

/**
 * sendMessage
 *   Marshall a message from its fields and send it to the server.
 *
 * @param[in] command - the operation to perform.
 * @param[in] data1   - First operand.
 * @param[in] data2   - second operand.
 */
void
CVarMgrServerApi::sendMessage(std::string command, std::string data1, std::string data2)
{
    std::string messageString = command;
    messageString            += ":";
    messageString            += data1;
    messageString            += ":";
    messageString            += data2;
    
    zmq::message_t request(const_cast<char*>(
        messageString.c_str()), messageString.size(), 0);
    
    m_pSocket->send(request); 
}

/**
 * getReply
 *    Get the status and data fields of the reply message as a pair of strings.
 *
 * @return std::pair<std::string, std::string> First is the status, second
 *                        the data.
 */
std::pair<std::string, std::string>
CVarMgrServerApi::getReply()
{
    zmq::message_t reply;
    m_pSocket->recv(&reply);
    
    // Recover the message as a string:
    
    size_t messageSize = reply.size();
    char* replyString  = new char[messageSize+1];
    memset(replyString, 0, messageSize+1);
    memcpy(replyString, reply.data(), messageSize);
    std::string replyS(replyString);
    delete []replyString;
    
    // Break the message up into status and data:
    
    size_t sepLoc = replyS.find(':');
    if (sepLoc == std::string::npos) {
        throw CVarMgrApi::CException("Reply message from server is ill-formed");
    }
    std::string status = replyS.substr(0, sepLoc);
    std::string data;
    if (sepLoc < replyS.size()) {
        data = replyS.substr(sepLoc+1);    
    }
    return std::pair<std::string, std::string>(status, data);
    
}
