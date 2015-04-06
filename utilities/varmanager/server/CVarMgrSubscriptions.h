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
# @file   CVarMgrSubscriptions.h
# @brief  Manage variable manager server subscriptions
# @author <fox@nscl.msu.edu>
*/


#ifndef CVARMGRSUBSCSRIPTIONS_H
#define CVARMGRSUBSCSRIPTIONS_H

#include <string>
#include <set>
#include <zmq.hpp>
#include <stdexcept>

/**
 * @class CVarMgrSubscriptions
 *
 * This class manages subsriptiions to the variable dat abase manager.
 * Change notification is the mechanism used to make programs aware
 * of changes to the variable database without requiring expensive polling.
 * This is handled by creating a SUB socket and connecting it to the
 * PUB port of the server.
 */
class CVarMgrSubscriptions
{
    // Public data types:
public:
    typedef struct _Message {
        std::string      s_path;
        std::string      s_operation;
        std::string      s_data;
    } Message, *pMessage;
    
    // Internal data types:
private:
   typedef std::set<std::string> Subscriptions;
   
   // Object data:
private:
    Subscriptions m_subscriptions;
    zmq::context_t* m_pContext;
    zmq::socket_t*  m_pSocket;
    
    // Canonicals:
public:
    CVarMgrSubscriptions(const char* host, int port);
    CVarMgrSubscriptions(const char* host, const char* service);
    virtual ~CVarMgrSubscriptions();
    
    // Selectors:
    
    zmq::socket_t* socket();
    int  fd();
    
    // Changing the subsriptions:
    
public:
    void subscribe(const char* pathPrefix);
    void unsubscribe(const char* pathPrefix);
    
    // Waiting for notificatinos:
    
public:
    bool waitmsg(unsigned int milliseconds = -1);
    
    void operator()(int milliseconds = -1);

    // I/O
    
    bool readable();
    Message read();
    
    // Notification handler for operator().
    // Subclass and override.
    
    virtual void notify(const pMessage message) {}
    
    // We throw exceptions too:
    
public:
    class CException : public std::runtime_error {
    public:
        CException(std::string what) noexcept :
            runtime_error(what) {}
        CException(const char* what) noexcept :
            runtime_error(what) {}
    };
    
    // utilities:
    
private:
    int translateService(const char* host, const char* serviceName);
    void initialize(const char* host, int port);
    
};

#endif

