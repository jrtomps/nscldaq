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
# @file   CStatusSubscription.h
# @brief  Define the API for subscribing to status messages.
# @author <fox@nscl.msu.edu>
*/
#ifndef CSTATUSSUBSCRIPTION_H
#define CSTATUSSUBSCRIPTION_H

#include <map>
#include <list>
#include "CStatusMessage.h"
#include <zmq.hpp>

/**
 * @class CStatusSubscription
 *    Encapsulates a set of status message subscriptions on a zmq::socket_t
 *    For more information about this, see:
 *
 *     https://swdev-redmine.nscl.msu.edu/projects/sfnscldaq/wiki/Subscription_API
 *
 *    (NSCL internal only link).
 *
 *    @note - the lifetime of the subscription is only the lifetime of the
 *            object.  If the object is destroyed, the subscription it has
 *            registered will also die alone.
 */

class CStatusSubscription
{
    // Data types:
public:
    typedef std::list<std::pair<size_t, CStatusDefinitions::Header> > Subscription;
    typedef Subscription::iterator                                    SubscriptionIterator;
    
    typedef std::list<uint32_t>                                       RequestedTypes;
    typedef std::list<uint32_t>                                       RequestedSeverities;

private:
    typedef std::map<unsigned, Subscription>                          SubscriptionRegistry;
    
    // Instance data
private:
    zmq::socket_t&          m_socket;
    unsigned                m_sequence;
    SubscriptionRegistry    m_registry;
    
    // Canonicals:
    
    CStatusSubscription(zmq::socket_t& sock);
    virtual ~CStatusSubscription();
    
public:
    unsigned subscribe(
        const RequestedTypes& types, const RequestedSeverities& sev,
        const char* app=nullptr, const char* source=nullptr
    );
    unsigned subscribe(Subscription& sub);
    
    void unsubscribe(unsigned sub);

    // Utilities:
    
private:    
    void legalSubscription(
        const RequestedTypes& types, const RequestedSeverities& sev,
        const char* app=nullptr, const char* source=nullptr
    );
    Subscription buildSubscription(
        const RequestedTypes& types, const RequestedSeverities& sev,
        const char* app=nullptr, const char* source=nullptr
    );
    unsigned registerSubscription(Subscription& subs);
    void unsubscribe(SubscriptionIterator s, SubscriptionIterator e);
    
};

#endif