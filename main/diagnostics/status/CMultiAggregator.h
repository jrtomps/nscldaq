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
# @file   CMultiAggregator.h
# @brief  <brief description>
# @author <fox@nscl.msu.edu>
*/

#ifndef CMULTIAGGREGATOR_H
#define CMULTIAGGREGATOR_H
#include <zmq.hpp>
#include <set>
#include <list>
#include <string>
#include <ctime>

class CPortManager;

/**
 * @class CMultiAggregator
 *
 *      This class is the main application class for the multi-node status
 *      aggregation proxy.
 *      -   A XPUB socket is created bound on the service for which we publish.
 *      -   An XSUB socket is created.
 *
 *      Once this initialization is complete; the pseudo code is:
 *
 *      while 1
 *          discoverNodes
 *          disconnectFromDeadNodes
 *          connectToNewNodes
 *          forward data for a  while.
 *
 *  The idea is that for the most part data/subscriptions etc. are forwarded
 *  from the XSUB -> XPUB socket and subscriptions get forwarded from
 *  XPUB -> XSUB.   From time to time, however we need to adjust the set of
 *  connections we have on the XPUB socket as the data flow structure can
 *  change dynamically in NSCLDAQ -  hence the passes through discoverNodes,
 *  disconnectFromDeadNodes and connectToNewNodes.
 *
 * @note - the messages received on the XSUB socket need to be saved and replayed
 *         on any new sockets.   This can be problematic as we don't want that
 *         history to grow without bounds.   We therefore maintain a limited depth
 *         queue which purges its back end.  This is suitable for a small number
 *         of subscribers that set up relatively static subscriptions.
 *         Not really sure what else to do.
 */
class CMultiAggregator
{
private:
    zmq::context_t&   m_zmqContext;
    zmq::socket_t    m_XSUBSocket;
    zmq::socket_t    m_XPUBSocket;
    CPortManager*    m_pPortManager;
    
    std::string      m_subscriptionService;
    std::string      m_publicationService;
    int              m_nDiscoveryInterval;
    std::time_t      m_nLastDiscoveryTime;
    
    std::set<std::string> m_connectedNodes;
    std::list<std::list<zmq::message_t*> > m_savedSubscriptions;
    
    std::string      m_publisherURI;
    
public:
    CMultiAggregator(
        const char* subscriptionService, const char* publicationService, int discoveryInterval
    );
    // Start for thread;
    CMultiAggregator(
        const char* subscriptionService, int discoveryInterval
    );
    virtual ~CMultiAggregator();
    
    void operator()();
    
    // These are intended to be used when the object runs as  a thread.
    
    std::string getPublisherURI() const {
        return m_publisherURI;
    }
    zmq::context_t& getZmqContext() {
        return m_zmqContext;
    }
    
private:
    std::set<std::string> discoverNodes();
    void disconnectDeadNodes(const std::set<std::string>& nodes);
    void connectNewNodes(const std::set<std::string>& nodes);
    void forwardMessages();
    std::string createUri(const std::string& node, const std::string& service);
    int  translatePort(const std::string& node, const std::string& service);
    void trimSavedMessages();
    void replaySubscriptions();
    std::list<zmq::message_t*> readMultipart(zmq::socket_t& s);
    void sendMultipart(zmq::socket_t& s, const std::list<zmq::message_t*>& parts);
    void freeMultiPart(std::list<zmq::message_t*>& parts);
public:
    static size_t MAX_SAVED_SUBSCRIPTION_MESSAGES;
};

#endif
