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
# @file   CVarMgrSubscriptions.cpp
# @brief  Implement variable manager server subscriptions class.
# @author <fox@nscl.msu.edu>
*/

#include "CVarMgrSubscriptions.h"
#include <stdio.h>
#include <os.h>
#include <CPortManager.h>
#include <Exception.h>
#include <string.h>
#include <errno.h>

/**
 * constructor
 *     @param host - Name of the host to which t we will connect (or dotted IP).
 *     @param port - Numbe of the port on which the publisher is listening.
 *     @throw CException - if the connection fails.
 */
CVarMgrSubscriptions::CVarMgrSubscriptions(const char* host, int port) :
    m_pContext(0), m_pSocket(0)
{
    
    initialize(host, port);    
}
/**
 * constructor:
 *   @param host - host to which we are connecting.
 *   @param serviceName - Named service to which we will connect.
 */

CVarMgrSubscriptions::CVarMgrSubscriptions(const char* host, const char* service) :
    m_pContext(0), m_pSocket(0)
{
    int port = translateService(host, service);              // Throws if can't
    initialize(host, port);
        
}
/**
 * destructor
 */
CVarMgrSubscriptions::~CVarMgrSubscriptions()
{
    delete m_pSocket;
    delete m_pContext;
    
}
/**
 * fd
 *   @return int - the file descriptor that underlies the socket.
 *                 this can be used in poll(2) or select(2) calls but
 *                 you need to call readable to know if there's really
 *                 anything readable.
 */
int  CVarMgrSubscriptions::fd()
{
    int fd;
    size_t size(sizeof(fd));

    m_pSocket->getsockopt(ZMQ_FD, &fd, &size);
    
    return fd;
}
/**
 * socket
 *    Return the zmqsocket that underlies this set of subscriptions.
 *
 *  @return zmq::socket_t - socket to the publisher.
 */
zmq::socket_t* CVarMgrSubscriptions::socket()
{
    return m_pSocket;
};
/**
 *  subscribe
 *     Add a new subscription to the object.
 *
 *  @param pathPrefix - the path to subscribe to notifications on
 *  @throw CException if the path duplicates an existing subscription.
 */
void CVarMgrSubscriptions::subscribe(const char* pathPrefix)
{
    if (m_subscriptions.count(pathPrefix) > 0) {
        throw CException("Duplicate subscription");
    }
    
    m_pSocket->setsockopt(ZMQ_SUBSCRIBE, pathPrefix, strlen(pathPrefix));
    m_subscriptions.insert(pathPrefix);
}
/**
 *  unsubscribe
 *     Remove an existing subscription.
 *  @param pathPrefix - the prefix we no longer want to hear about.
 *  @throw CException - if there is no such subscription
 */
void CVarMgrSubscriptions::unsubscribe(const char* pathPrefix)
{
    if (m_subscriptions.count(pathPrefix) == 0) {
        throw CException("No such subscription");
    }
    m_pSocket->setsockopt(ZMQ_UNSUBSCRIBE, pathPrefix, strlen(pathPrefix));
    m_subscriptions.erase(m_subscriptions.find(pathPrefix));
}
/**
 * waitmsg
 *    Wait for a message to be available on the socket with a timeout.
 * @param milliseconds - The upper limit on the number of milliseconds in the
 *                      timeout.  The  actual timeout may be shorter if
 *                       the function's use of zmq__poll is interrupted by
 *                       a signal. Such a case is treated as a timeout.
 *                       If milliseconds is -1 (default) there is no timeout.
 *                       If milliseconds is 0 this will test without blocking.
 * @return bool - true if there are messages waiting to be read.
 */
bool CVarMgrSubscriptions::waitmsg(int milliseconds)
{
    zmq_pollitem_t item =  { (void*)(*m_pSocket), -1, ZMQ_POLLIN, 0};
    
    int nItems = zmq_poll(&item, 1, milliseconds == -1 ? -1 : milliseconds*1000);
    
    return nItems == 1;
}

/**
 * readable
 *    Return true if the socket has at least one message waiting to be read.
 *    That is if a call to read would return immediately.
 *  @return bool - true if there are message waiting
 */
bool CVarMgrSubscriptions::readable() {
    return waitmsg(0);                      // Hard to go wrong here.
}
/**
 * read
 *    Read the next publication message.  The message is broken into its
 *    fields and returned as a message struct.
 *
 * @return CVarMgrSubscriptions::Message - the decoded data read.
 */

CVarMgrSubscriptions::Message CVarMgrSubscriptions::read() {
    Message decodedMessage;
    zmq::message_t msg(1000);

    while(!m_pSocket->recv(&msg)) {
        if ((errno != EAGAIN) && (errno != EINTR )) {
            throw CException(strerror(errno));
        }
        // Retry the read...
    }
    // Stringify the message:
    
    size_t nChars = msg.size();
    void*  pMsg   = msg.data();
    char   szMsg[nChars+1];
    memset(szMsg, 0, nChars+1);
    memcpy(szMsg, pMsg, nChars);    // null terminated string.
    std::string msgString(szMsg);
    
    // Split the colon separated fields:
    // In this code we assume a well formed message (bad?).
    
    size_t cLoc = msgString.find_first_of(':');
    decodedMessage.s_path = msgString.substr(0, cLoc);
    msgString = msgString.substr(cLoc+1);
    
    cLoc = msgString.find_first_of(':');
    decodedMessage.s_operation = msgString.substr(0, cLoc);
    decodedMessage.s_data      = msgString.substr(cLoc+1);
    
    return decodedMessage;
}
    

/**
 * operator()
 *    This is a mini event loop pass for subscriptions.
 *    -   The function runs a waitmsg
 *    -   If a message arrives it is read.
 *    -   Arrived messages are passed to the notify method.
 *    notify is virtual and normally needs to be overidden in a subclass to
 *    be useful
 *
 *    @param milliseconds - Maximum milliseconds to wait for a message.
 */
void CVarMgrSubscriptions::operator()(int milliseconds)
{
    if (waitmsg(milliseconds)) {
        Message msg = read();
        notify(&msg);
    }
}


/*-------------------------------------------------------------------------------
 * utility methods:
 */

/**
 * translateService
 *    Translate a service into a port
 *
 *   @param host - host in which we do the translation.
 *   @param serviceName - name of the service to translate.
 *   @return int    - Port number.
 *   @throw CException if the pot does not translate
 */
int
CVarMgrSubscriptions::translateService(const char* host, const char* serviceName)
{
    try {
        CPortManager mgr(host);
        std::string userName = Os::whoami();
        
        std::vector<CPortManager::portInfo> services = mgr.getPortUsage();
        for (int i =0; i < services.size(); i++) {
            if ((services[i].s_User == userName) &&
                (services[i].s_Application == serviceName)
            ) {
                return services[i].s_Port;
            }
        }
        throw CException("No translation for the port");
    }
    catch (::CException& e) {
        throw(CException(e.ReasonText()));
    }
    
}
/**
 * initialize
 *    Common initialization code used by all constructors:
 *
 * @param host -  Name of the host we are connecting to.
 * @param port - Port on host to which we'll connect.
 * @throw CException on detected errors.
 */
void
CVarMgrSubscriptions::initialize(const char* host, int port)
{
    // Create the context and socket:
    
    m_pContext = new zmq::context_t(1);
    m_pSocket  = new zmq::socket_t(*m_pContext, ZMQ_SUB);
    
    // Construct the URI and connect the socket to the server:
    
    char uri[200];
    sprintf(uri, "tcp://%s:%d", host, port);
    try {
        m_pSocket->connect(uri);
    } catch (zmq::error_t& e) {
        throw CException(e.what());
    }
    
}
