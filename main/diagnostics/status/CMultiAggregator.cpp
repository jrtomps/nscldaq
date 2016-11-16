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
# @file   CMultiAggregator.cpp
# @brief  Implement the aggregation proxy.
# @author <fox@nscl.msu.edu>
*/
#include <config.h>
#include "CMultiAggregator.h"
#include <CConnectivity.h>
#include <CPortManager.h>
#include <sstream>
#include <iostream>
#include <cstdlib>


// Static members:

size_t CMultiAggregator::MAX_SAVED_SUBSCRIPTION_MESSAGES(1000);   // Saved SUB/UNSUB messages.

// local one-liners:
template<class T>
bool in(const T& item, const std::set<T>& s)
{
    return s.count(item) > 0;
}

/**
 * constructor:
 *  - Save the parameters.
 *  - Create the XSUB socket.
 *  - Create/bind the XPUB socket.
 *
 * @param subscriptionService - name of service XSUB should connect to.
 * @param publicationService  - name of services XPUB should advertise.
 * @param discoveryInterval   - Seconds between discovery passes.
 */
CMultiAggregator::CMultiAggregator(
    const char* subscriptionService, const char* publicationService,
    int discoveryInterval
) :
    m_zmqContext(1),
    m_XSUBSocket(m_zmqContext, ZMQ_XSUB),
    m_XPUBSocket(m_zmqContext, ZMQ_XPUB),
    m_pPortManager(0),
    m_subscriptionService(subscriptionService),
    m_publicationService(publicationService),
    m_nDiscoveryInterval(discoveryInterval)
{
    // Wait a decent interval for the port manager to startup -- die if not.
    
    if(!CPortManager::waitPortManager(10)) {
        std::cerr << "Local port manager does not appear to be running\n";
        std::exit(EXIT_FAILURE);
    }
    
    // bind the XPUB socket to a specific advertised port:
    
    m_pPortManager = new CPortManager();
    int port       = m_pPortManager->allocatePort(m_publicationService);
    
    std::stringstream uri;
    uri << "tcp://*:" << port;
    m_XPUBSocket.bind(uri.str().c_str());       // Connections on all interfaces.
}

/**
 * destructor
 *    Must destroy the port manager object.
 */
CMultiAggregator::~CMultiAggregator()
{
    delete m_pPortManager;
}

/**
 * operator()
 *    This is the entry point of the entire app
 *    See the header for the functionality.
 */
void CMultiAggregator::operator()()
{
    while(1) {
        
        // Update connections:
        
        std::set<std::string> currentNodes   = discoverNodes();
        disconnectDeadNodes(currentNodes);
        connectNewNodes(currentNodes);
        m_connectedNodes = currentNodes;
        m_nLastDiscoveryTime = std::time(nullptr);
        
        // Proxy for m_nDiscoveryInterval secs.
        
        forwardMessages();                    
    }
}
/*------------------------------------------------------------------------------
 * private utilities:
 */

/**
 * discoverNodes
 *    Produce the set of nodes involved in the dataflow that runs
 *    Through localhost:
 */
std::set<std::string>
CMultiAggregator::discoverNodes()
{
    CConnectivity    c("localhost");
    std::vector<std::string> hosts = c.getAllParticipants();
    
    std::set<std::string> result(hosts.begin(), hosts.end());
    
    return result;
}

/**
 * disconnectDeadNodes
 *    Note ZMQ 4.0 and later support disconnects so without that this is a noop.
 *    take all the nodes in m_connectedNodes that are not in the current nodes
 *    and disconnect them.
 *
 *   @param current - Current nodes.
 */
void
CMultiAggregator::disconnectDeadNodes(const std::set<std::string>& current)
{
    // If the library does not have zmq_disconnect(), we're stuck still
    // getting messages from nodes we'd like to drop..
    
#ifdef ZMQ_HAVE_DISCONNECT 
    for_each(
        m_connectedNodes.begin(), m_connectedNodes.end(),
        [this, current](const std::string& item) -> void {
            if (!in(item, current)) {
                try {
                    std::string uri = createUri(item, m_subscriptionService);
                    m_XSUBSocket.disconnect(uri.c_str());
                } catch(...) {}                   // In case service vanished.
            }
        }
    );
#endif
}
/**
 * connectNewNodes
 *    Add XSUB connections for nodes that are new.  Note that having done
 *    that we need to reply our subs.
 *
 *  @param current - current node configuration.
 */
void
CMultiAggregator::connectNewNodes(const std::set<std::string>& current)
{
    bool replay = false;
    for_each(current.begin(), current.end(), [this, &replay](const std::string& item) -> void {
        if(!in(item, m_connectedNodes)) {
            try {
                std::string uri = createUri(item, m_subscriptionService);
                m_XSUBSocket.connect(uri.c_str());
                replay = true;                // At least one connection added.
            } catch (...) {}                 // in case service vanished.
        }
    
    });
    
    if (replay) {
        replaySubscriptions();
    }
}
/**
 *  forwardMessages
 *     Forward messages from m_XPUBSocket -> m_XSUBSocket and
 *     back until the time between 'now' and m_nLastDiscoveryTime is <= 0.
 */
void
CMultiAggregator::forwardMessages()
{
    std::time_t now = std::time(nullptr);
    
    while((now - m_nLastDiscoveryTime) < m_nDiscoveryInterval) {
        zmq_pollitem_t items[2] = {
            {(void*)(m_XSUBSocket), -1, ZMQ_POLLIN, 0},
            {(void*)(m_XPUBSocket), -1, ZMQ_POLLIN, 0}
        };
        long timeout = m_nDiscoveryInterval - (now - m_nLastDiscoveryTime); // Remainder of interval
        
        int status = zmq_poll(items, 2, timeout);
        
        // Forward XSUB?
        
        if (items[0].revents) {
            std::list<zmq::message_t*> message = readMultipart(m_XSUBSocket);
            sendMultipart(m_XPUBSocket, message);
            freeMultiPart(message);
        }
        
        // Forward XPUB?  Note these messages must be saved for replay to
        // newly added endpoints.
        
        if(items[1].revents) {
            std::list<zmq::message_t*> message = readMultipart(m_XPUBSocket);
            sendMultipart(m_XSUBSocket, message);
            m_savedSubscriptions.push_back(message);
            trimSavedMessages();
        }
        now = std::time(nullptr);               // Update current time.
    }
}
/**
 * createUri
 *    Create a URI string for a service in a potentially Remote node.
 *    If the service does not exist, an exception is throw indicating that.
 *
 *  @param node  - Name or stringified IP of the host.
 *  @param service - service (from which we get the port).
 *
 *  @return std::string  (tcp://node:port).
 */
std::string
CMultiAggregator::createUri(const std::string& node, const std::string& service)
{
    int port = translatePort(node, service);
    if (port == -1) {
        throw std::string("Could not translate port");
    }
    std::stringstream  uri;
    uri << "tcp://" << node << ":" << port;
    
    return uri.str();
}
/**
 * translatePort
 *    Translate a host/service into a port number.
 *    - We assume there's at most one service with that name.
 *
 *  @param host  - host we're translating in.
 *  @param service - name of the service.
 *  @return int   - Port number found.
 *  @retval -1    - no translation.
 */
int
CMultiAggregator::translatePort(const std::string& node, const std::string& service)
{
    CPortManager manager(node);
    std::vector<CPortManager::portInfo> info = manager.getPortUsage();
    
    for (auto i = info.begin(); i != info.end(); i++) {
    
       if (i->s_Application == service) return i->s_Port;
    }
    
    // Not found.
    
    return -1;
}
/**
 * trimSavedMessages
 *   Remove saved subscription messages to trim the list back down below
 *   CMultiAggregator::MAX_SAVED_SUBSCRIPTION_MESSAGES.
 *   The messages are removed from the front of the saved list as newer messages
 *   are put on the back.
 */
void
CMultiAggregator::trimSavedMessages()
{
    ssize_t nToRemove = m_savedSubscriptions.size() - CMultiAggregator::MAX_SAVED_SUBSCRIPTION_MESSAGES;
    /*
     *  Anything we do to get 'cute' - e.g. compute iterators is O(2n) while just
     *  looping over popping front is O(n):
     */
    while (nToRemove > 0) {
        m_savedSubscriptions.pop_front();
        nToRemove--;
    }
}
/**
 * replaySubscriptions
 *    Replay the saved subscription list.  This is necessary when we add a new
 *    connection as at that time the newly connected publisher knows nothing
 *    about the existing subscriptions.
 *
 */
void
CMultiAggregator::replaySubscriptions()
{
    for_each(
        m_savedSubscriptions.begin(), m_savedSubscriptions.end(),
        [this](std::list<zmq::message_t*> pMessage) -> void {
            sendMultipart(m_XSUBSocket, pMessage);
        }
    );
}
/**'
 * readMultipart
 * 
 *     Read a (potentially) multipart message from a socket.
 *
 *  @param s   - reference to the zmq::socket_t from which the read will be done.
 *  @return std::list<zmq::message_t*> - the list of message parts that make up the
 *               full message.
 */
std::list<zmq::message_t*>
CMultiAggregator::readMultipart(zmq::socket_t& s)
{
    std::list<zmq::message_t*> result;
    uint64_t more(0);
    size_t   sMore(sizeof(more));
    do {
        zmq::message_t* pMessage = new zmq::message_t;
        s.recv(pMessage);
        result.push_back(pMessage);
        
        s.getsockopt(ZMQ_RCVMORE, &more, &sMore);
    } while (more);
    
    return result;
}
/**
 * sendMultipart
 *    Send a (potentially) multipart message.
 *
 *  @param s -- The socket on which to send it.;
 *  @param message - list of message part pointers.
 */
void
CMultiAggregator::sendMultipart(zmq::socket_t& s, const std::list<zmq::message_t*>& message)
{
    auto p = message.begin();
    do {
        zmq::message_t* part = *p;
        p++;                          // Do this now so we know if ZMQ_SNDMORE:
        
        s.send(*part, (p == message.end()) ? 0 : ZMQ_SNDMORE);
        
    } while (p != message.end());
}
/**
 * freeMultipart
 *    Free the contents of a multipart message;
 *
 * @param message -  List of message parts.  On exit, the zmq::message_t* it
 *                   contains will have been deleted while the list itself
 *                   will be emptied.
*/
void
CMultiAggregator::freeMultiPart(std::list<zmq::message_t*>& parts)
{
    while (!parts.empty()) {
        delete parts.front();
        parts.pop_front();
    }
}
