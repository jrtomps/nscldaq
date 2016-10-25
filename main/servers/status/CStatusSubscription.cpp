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
# @file   CStatusSubscription.cpp
# @brief  Implement the subscription api.
# @author <fox@nscl.msu.edu>
*/

#include "CStatusSubscription.h"
#include <stdexcept>
#include <cstring>
#include <cstddef>
/**
 * construction
 *    Initialize all of the fields.
 *
 *  @param sock - the zmq socket that will be used for making the subscriptions.
 */
CStatusSubscription::CStatusSubscription(zmq::socket_t& sock) :
    m_socket(sock), m_sequence(0)
{}

/**
 * Destructor:
 *    Unsubscribe for each element of the map.  The map itself can be
 *    destroyed perfectly happily by itself.
 */
CStatusSubscription::~CStatusSubscription()
{
    for(auto pMap = m_registry.begin(); pMap != m_registry.end(); pMap++) {
        for (auto sub = pMap->second.begin(); sub != pMap->second.end(); sub++) {
            size_t                      nBytes = sub->first;
            CStatusDefinitions::Header& data(sub->second);
            
            m_socket.setsockopt(ZMQ_UNSUBSCRIBE, &data, nBytes);
        }
    }
}

/**
 * subscribe
 *    The 'normal' subscription entry;
 *    - Validate the subscription spec,
 *    - Build a subscription list
 *    - Perform the subscription.
 *    - Register the subscription in the registry:
 *
 * @param type - set of requested types.
 * @param sev - Set of requestsed severities.
 * @param app - Application (if any) we want.
 * @param source - Source from which we want information.
 */
unsigned
CStatusSubscription::subscribe(
    const RequestedTypes& types, const RequestedSeverities& sev,
    const char* app, const char* source
)
{
    legalSubscription(types, sev, app, source);
    Subscription sublist = buildSubscription(types, sev, app, source);
    return subscribe(sublist);
}
/**
 * subscribe
 *    This overload is intended to create ZMQ subscriptions from a list of
 *    previously formatted subscriptions.  This could have been created by
 *    a user or it could have been created by the subscribe method above.
 *
 * @param subs - List of preformatted subscriptions.  Each subscription is
 *               a pair consisting of a CStatusDefinitions::Header struct
 *               and a size indicating the number of bytes of that header
 *               that are relevent for the subscription.
 * @return unsigned  - A handle which allows the user to remove this
 *               subscription.
 * @note - Subscription problems get reported as exceptions.  Therefore
 *         if an exception is thrown we back off each of the subscriptions
 *         that succeeded for this call
 */
unsigned
CStatusSubscription::subscribe(Subscription& sub)
{
    // We're going to keep track of where we are in the iteration so that
    // we can back out the successes: in case of an exception:
    
    SubscriptionIterator p = sub.begin();
    try {
        size_t bytes = p->first;
        CStatusDefinitions::Header& header(p->second);
        m_socket.setsockopt(ZMQ_SUBSCRIBE, &header, bytes);
        
        p++;
    }
    catch(...) {
        unsubscribe(sub.begin(), p);
        throw;
    }
    return registerSubscription(sub);
}

/*----------------------------------------------------------------------------
 *    Private utility functions.  Some for code re-use others just to make
 *    the logic of the public methods look cleaner than they have any right to
 *    look.
 */

/**
 *  legalSubscription
 *     Checks that a subscription is fully legal.  Throws an std::logic
 *     error if the subscription does not pass the legality test.  The
 *     subscription is illegal if the source is specified, but the
 *     application is not.  This is not legal because ZMQ requires us to
 *     filter only on the front of a message and the application field comes
 *     after the source field.
 *
 *     If the subscription is illegal an std::logic_error is thrown.
 *
 *   @param types   - List of types being subscribed to.
 *   @param sev     - List of severities.
 *   @param app     - Application to match (NULL If not used).
 *   @param source  - Source to match, NULL if not used.
 */
void
CStatusSubscription::legalSubscription(
    const RequestedTypes& types, const RequestedSeverities& sev,
    const char* app, const char* source
)
{
    if (source && (!app)) {
        throw std::logic_error("Illegal subscription.");
    }
}
/**
 * buildSubscripton.
 *    Builds the list of subscriptions.
 *    - Special case. If nothing is specified we can return a single 0 length
 *      subscription descriptor.
 *    - Determine how much of the header is relevant:
 *       *  If types are specified but nothing else we just need that.
 *       *  If severity is specified we need to go through that.
 *       *  If the app is specified we need to go through that as well.
 *       *  If the source is specified, we need the entire struct.
 *    - Produce the subscription description list and return it.
 */
CStatusSubscription::Subscription
CStatusSubscription::buildSubscription(
    const RequestedTypes& types, const RequestedSeverities& sev,
    const char* app, const char* source
)
{
    Subscription result;
    // Special case?
    
    if ((!types.size()) && (!sev.size()) && (!app) && (!source)) {
        std::pair<size_t, CStatusDefinitions::Header> subElement;
        subElement.first = 0;
        std::memset(&(subElement.second), 0, sizeof(CStatusDefinitions::Header));
        result.push_back(subElement);
    } else {
        RequestedSeverities allSeverities = {
            CStatusDefinitions::SeverityLevels::DEBUG,
            CStatusDefinitions::SeverityLevels::INFO,
            CStatusDefinitions::SeverityLevels::SEVERE,
            CStatusDefinitions::SeverityLevels::DEFECT
        };
        // Not the trivial case;  Figure out the value of size_t -- assume this is
        // a legal subscription.
        
        size_t bytes;
        if(types.size()) {
            bytes = offsetof(CStatusDefinitions::Header, s_severity);
        }
        if (sev.size()) {
            bytes = offsetof(CStatusDefinitions::Header, s_application);
        }
        if(app) {
            bytes = offsetof(CStatusDefinitions::Header, s_source);
        }
        if (source) {
            bytes = sizeof(CStatusDefinitions::Header);
        }
        
        // If types is not supplied, use all types (we know something is used).
        
        RequestedTypes usedTypes = types;
        if (usedTypes.size() == 0) {
            for (auto i = 0; i < CStatusDefinitions::MessageTypes::FIRST_FREE_TYPE; i++) {
                usedTypes.push_back(i);
            }
        }
        // If sev is empty but there's an app, we need to use all severities:
        
        RequestedSeverities usedSeverities = sev;
        if ((sev.size() == 0) && app) {
            usedSeverities = allSeverities;
        }
        
        // Now we should be able to build up the result.
        
        for (auto pT = usedTypes.begin(); pT != usedTypes.end(); pT++) {
            
            if (usedSeverities.size()) {
                // Need to inner loop over the severities too:
                
                for(auto pS = usedSeverities.begin(); pS != usedSeverities.end(); pS++) {
                    CStatusDefinitions::Header h;
                    std::memset(&h, 0, sizeof(h));
                    h.s_type     = *pT;
                    h.s_severity = *pS;
                    if (app) {
                        std::strncpy(h.s_application, app, sizeof(h.s_application) - 1);
                    }
                    if (source) {
                        std::strncpy(h.s_source, source, sizeof(h.s_source) - 1);
                    }
                    result.push_back(
                        std::pair<size_t, CStatusDefinitions::Header>(bytes, h)
                    );
                 }
                
            } else {
                // Don't need to loop over the sev:
                
                CStatusDefinitions::Header h;
                std::memset(&h, 0, sizeof(h));
                h.s_type = *pT;
                
                // If there are no severities we're done...since there's no APP etc.
                
                result.push_back(
                    std::pair<size_t, CStatusDefinitions::Header>(bytes, h)
                );
            }
        }
        
    }    
    return result;
}
/**
 * registerSubscription
 *    Assign a handle to the subscription and enter it into the subscription map.
 *
 *  @param sub - List of ZMQ subscriptions that correspond to an API subscription.
 *  @return unsigned - handle to the subscription.
 */
unsigned
CStatusSubscription::registerSubscription(Subscription & subs)
{
    unsigned result = m_sequence++;
    m_registry[result] = subs;
    return result;
}
/**
 * unsubscribe
 *    Given iterators into a subscrition does the ZMQ unsubscriptions for
 *    [s, end)
 *
 *   @param s    - Iterator to the first item to unsubscribe.
 *   @param e    - Final iterator in the usubscription.  Note that
 *                 *e is not usubscribed.
 */
void
CStatusSubscription::unsubscribe(SubscriptionIterator s, SubscriptionIterator e)
{
    // For the rest of us newbies,the final parameter to std::for_each is a
    // that makes 'this' available in its body scope.  The lambda is invoked
    // each iteration of for_each.
    std::for_each(
        s, e,
        [this](std::pair<size_t, CStatusDefinitions::Header>& item) mutable {
            m_socket.setsockopt(ZMQ_UNSUBSCRIBE, &(item.second), item.first);
        }
    );
}

