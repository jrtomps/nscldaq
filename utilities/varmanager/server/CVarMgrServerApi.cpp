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
#include <CVarDirTree.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <iostream>
#include <unistd.h>

zmq::context_t* CVarMgrServerApi::m_pContext(0);

/**
 * constructor
 *    - Create the zmq context and socket.
 *    - set the wd to "/"
 *
 *  @param[in] server - Name of the server (this could also be e.g. "127.0.0.1").
 *  @param[in] port   - Server's request port.
 */
CVarMgrServerApi::CVarMgrServerApi(const char* server, int port) :
    m_pSocket(0),
    m_wd("/")
{
    char Uri[strlen(server) + 1 + 200];
    if (!m_pContext)  m_pContext = new zmq::context_t(2);
    
    try {
       
        m_pSocket  = new zmq::socket_t(*m_pContext, ZMQ_REQ);
        
        
        sprintf(Uri, "tcp://%s:%d", server, port);
        m_pSocket->connect(Uri);
    }
    catch (...) {
        delete m_pSocket;
        throw;
    }
     
}
    
CVarMgrServerApi::~CVarMgrServerApi()
{
    delete m_pSocket;
}
    
    // Interface implemented:
    
/**
 * cd
 *    Set the current working directory.
 *
 *  @param[in] path - new working directory
 */
void CVarMgrServerApi::cd(const char* path)
{
    std::string wd = makePath(path);
    
    if (existingDirectory(wd)) {
        m_wd = wd;
    } else {
        throw CVarMgrApi::CException("CVarMgrServerApi::cd - Invalid/nonexistent directory");
    }
}
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
    transaction("MKDIR", makePath(path), "");
    
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
    transaction("RMDIR", makePath(path), "");
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
    transaction("DECL", makePath(path), data2);
}
/**
 * set
 *     Set a variable to a new value.
 *  @param[in] path - path to the variable.
 *  @param[in] value - New proposed value for the variable.
 */
void CVarMgrServerApi::set(const char* path, const char* value)
{
    transaction("SET", makePath(path), value);
}
/**
 * get
 *  Return the value of a variable.
 *
 * @param path - path to the variable.
 * @return std::string - current value of the variable.
 */
std::string CVarMgrServerApi::get(const char* path) {
    return transaction("GET", makePath(path), "");
}
/**
 * defineEnum
 *   Define a new enumerated type.
 *
 *  @param[in] typeName - Name of the type to be used when declaring new variables
 *                        of that type.
 *  @param[in] values   - Set of values variables of this type can have.
 *
 */   
void CVarMgrServerApi::defineEnum(const char* typeName, EnumValues values)
{
    transaction("ENUM", typeName, join(values, '|'));
}
/**
 * defineStateMachine
 *    Define a new data type that is a statemachine.
 *  @param[in] typeName    - Name of the new state machine type.
 *  @param[in] transitions - Transition map created with addTransition from the
 *                           base class.
 */
void CVarMgrServerApi::defineStateMachine(const char* typeName, StateMap transitions)
{
    // Marshall the state transition map into the final parameter:
    
    std::vector<std::string> stateStrings;
    for(CVarMgrApi::StateMap::iterator p = transitions.begin(); p != transitions.end(); p++) {
        std::string states = p->first;             // State name.
        if (p->second.size() > 0) {
            states += ',';
            states += join(p->second,',');
        }
        stateStrings.push_back(states);
        
    }
    // stateStrings can be joined with a '|' to make the final data parameter:
    
    transaction("SMACHINE", typeName, join(stateStrings, '|'));
}

/**
 * ls
 *   list directories.
 *
 * @param[in] path  - Optional path parameter.
 *                    - If not provided the wd is listed.
 *                    - If provided and absolute the path is listed.
 *                    - If provided and relative the wd combined with the path are listed.
 * @return std::vector<std::string> - vector of subdirectories in the path.
 */
std::vector<std::string>
CVarMgrServerApi::ls(const char* path)
{
    std::string parent = m_wd;
    if (path) {
        parent = makePath(path);
    }
    std::string result = transaction("DIRLIST", parent, "");
    std::set<std::string> dirSet = processDirList(result);
    
    std::set<std::string>::iterator p = dirSet.begin();
    std::vector<std::string> resultVec;
    while (p != dirSet.end()) {
        resultVec.push_back(*p);
        p++;
    }
    return resultVec;
    
}

    // Utilities:

/**
 *  existingDirectory
 *     See if the specified directory exists in the database
 *
 *  @param[in] path - path to check.
 *  @return bool - true the path exists, false it does not.
 */
bool CVarMgrServerApi::existingDirectory(std::string path)
{
    // Break the path up into the parent directory and the
    // directory we are looking for in that directory.
    
    std::vector<std::string> pathList = CVarDirTree::parsePath(path.c_str());
    
    // Special case, the path is empty - root directory always exists:
    
    if (pathList.size() == 0) {
        return true;
    }
    std::string subdir = pathList.back();
    pathList.pop_back();
    
    std::string parent("/");        
    parent += join(pathList, '/');
    
    std::string result = transaction("DIRLIST", parent, "");
    
    std::set<std::string> dirSet = processDirList(result);
    return dirSet.count(subdir) == 1;
    
}

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
#ifdef DEBUG        
        std::cerr << "Failed on " << command << ":" << data1 << ":" << data2 <<std::endl;
        std::cerr << reply.first << " " << reply.second << std::endl;
#endif
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
    char reqsz[messageString.size()+1];
    strcpy(reqsz, messageString.c_str());
    zmq::message_t request(messageString.size());
    memcpy(request.data(), reqsz, messageString.size());
    
    int status = m_pSocket->send(request);
    
    if(status == -1) {
        perror("Failed to send");
        throw CException("Failed to send");
    }
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


/**
 * join
 *    Joins the strings in a vector into a single string with the specified
 *    separator.
 *
 *  @param[in] values - strings to join.
 *  @param[in] sep     - Separator between strings.
 *  @return std::string - generated string.
 */
std::string
CVarMgrServerApi::join(const CVarMgrApi::EnumValues& values, char sep)
{
    std::string result;
    
    // Edge case of empty vector -> empty string.
    
    if (values.size() == 0) {
        return result;
    }
    
    // All but the last just append the strings and a sep.
    for (int i =0; i < values.size()-1; i++) {
        result += values[i];
        result += sep;
    }
    // Last string is appended without a sep.
    
    result += values.back();
    
    return result;
}
/**
 * join
 *   Joins the strings that make up a set into a single string with the
 *   specified separator.
 *
 * @param[in] values - strings to join.
 * @param[in] sep    - separator character.
 * @return std::string - Joined string.
 */
 std::string
 CVarMgrServerApi::join(const std::set<std::string>& values, char sep)
 {
    std::string result;
    std::set<std::string>::iterator p = values.begin();
    
    // This will leave an extra trailing separator:
    while (p != values.end()) {
        result += *p;
        result += sep;
        p++;
    }
    // Get rid of the last trailing separator before returning.
    
    result = result.substr(0, result.size()-1);
    
    return result;
 }
 /**
  * makePath
  *    Create the actual path for a directory:
  *    - If the path is absolute it is as is.
  *    - If the path is relative it's combined with the working directory.
  *
  *  @param[in] path - an absolute or relative path.
  *  @return std::string - the final path.
  */
 std::string
 CVarMgrServerApi::makePath(const char* path)
 {
    std::string result;
    if (CVarDirTree::isRelative(path)) {
        result = m_wd;
        result += '/';
        result += path;
    } else {
        result = path;
    }
    return canonicalize(result);
 }
 /**
  * canonicalize
  *    Canonicalize a path.  Currently this is done by  just
  *    resolving the ..'s in the path.
  *    TODO: Play around with the implementation of CVarDirTree::canonicalize
  *          so that the code I stole from there can be factored out
  *  @param[in] path - input path
  *  @return std::string - canonicalized path
  */
 std::string
 CVarMgrServerApi::canonicalize(std::string path)
 {
    std::vector<std::string> pathVec = CVarDirTree::parsePath(path.c_str());
    
    // For each .. remaining in the path, we need to remove it
    // and the prior element of the vector.  If that would make us
    // run off the front of the vector throw and exception.
    
    
    std::vector<std::string>::iterator i = pathVec.begin();
    while(i != pathVec.end()) {
        if (*i == "..") {
            if(i == pathVec.begin()) {
                throw CException(
                    "CVarMgrServerApi::canonicalize -- attempted t go above root directory"
                );
            }
            i--;
            i = pathVec.erase(i);
            i = pathVec.erase(i);
        } else {
            i++;
        }
    }
    std::string cPath = "/";
    cPath += join(pathVec, '/');
    return cPath;                       // Empty pathVec implies /.
 }
 /**
  * processDirList
  *    Get the set of directories from the data part of a DIRLIST transaction
  * @param[in] dirlist - The data field of the DIRLIST reply.
  * @return std::set<std::string> - whose elements are the directories in the dirlist.
  */
 std::set<std::string>
 CVarMgrServerApi::processDirList(std::string dirlist)
 {
    std::set<std::string> result;
    
    while(dirlist.size() > 0) {
        size_t nextSep = dirlist.find("|");
        
        // No separator means this is the last dir:
        
        if (nextSep == std::string::npos) {
            result.insert(dirlist);
            dirlist = "";                  // Done.
        } else {
            result.insert(dirlist.substr(0, nextSep));
            dirlist = dirlist.substr(nextSep+1);
        }
    }
    
    return result;
 }
 