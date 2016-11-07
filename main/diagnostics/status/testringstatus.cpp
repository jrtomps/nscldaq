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
# @file   testringstatus.cpp
# @brief  End to end test program for ring status subscriptions.
# @author <fox@nscl.msu.edu>
*/

#include <zmq.hpp>
#include <CPortManager.h>
#include <CStatusSubscription.h>
#include <CStatusMessage.h>
#include <iostream>
#include <sstream>
#include <assert.h>
#include <cstdint>
#include <cstdlib>
#include <cstring>

/**
 *  This test program subscribes to ring status updates and just dumps
 *  the message parts it gets to stdout.  It is not intended to be
 *  installed but is just intended for test purposes.
 */

const char* Service ="StatusPublisher";           // Publication service.

/**
 *  Joint a bunch of null terminated strings into a space separated
 *  std::string
 */
static std::string joinString(const char* p)
{
    std::string result;
    
    while (*p) {
        result += p;
        result += " ";         // test program so don't worry about extr trailing " "
        p += std::strlen(p) + 1;
    }
    return result;
}

/**
 * @return - port on which the service is running
 */

static int
getPort()
{
    CPortManager portManager;
    std::vector<CPortManager::portInfo> ports = portManager.getPortUsage();
    
    for (int i = 0; i < ports.size(); i++) {
        if (ports[i].s_Application == Service) {
            return ports[i].s_Port;
        }
    }
    std::cerr << "Be sure the status aggregator is running\n";
    std::exit(EXIT_FAILURE);
}


/**
 * @param   newly created socket.
 * @return  Susbcription object
 */
static CStatusSubscription*
subscribe(zmq::socket_t& sock)
{
    CStatusSubscription* pSub = new CStatusSubscription(sock);
    CStatusSubscription::RequestedTypes types = {
        CStatusDefinitions::MessageTypes::RING_STATISTICS
    };
    CStatusSubscription::RequestedSeverities sevs = {
        CStatusDefinitions::SeverityLevels::INFO
    };
    pSub->subscribe(types, sevs);
    
    return pSub;
}

// Output methods:

std::ostream&
operator<<(std::ostream& s, const CStatusDefinitions::Header& h)
{
    s << "Header: " << h.s_type << " " << h.s_severity << " " << h.s_application
        << " " << h.s_source;
        
    return s;
}

std::ostream&
operator<<(std::ostream& s, const CStatusDefinitions::RingStatIdentification& id)
{
    std::time_t t = static_cast<std::time_t>(id.s_tod);
    s << std::ctime(&t) << " Ring: " << id.s_ringName;
    
    return s;
}

std::ostream&
operator<<(std::ostream& s, const CStatusDefinitions::RingStatClient& client)
{
    std::string type = (client.s_isProducer ? "Producer " : "Consumer ");
    
    s << type << joinString(client.s_command) << " "
      << client.s_operations << " ops " << client.s_bytes << " bytes";
    
    return s;
}


/**
 * main
 */
int main(int argc, char** argv)
{
    int port = getPort();
    std::stringstream connection;
    connection << "tcp://localhost:" << port;
    
    zmq::context_t ctx(1);
    zmq::socket_t sock(ctx, ZMQ_SUB);
    sock.connect(connection.str().c_str());
    
    CStatusSubscription* sub = subscribe(sock);
    
    // Process message groups, each group is a header, a ring id
    // followed by zero or more client parts.
    
    while(1)
    {
        zmq::message_t header;
        zmq::message_t ringId;
        
        // Get the header and ensure it's really a ring status message:
        
        sock.recv(&header);
        CStatusDefinitions::Header* pHeader =
            reinterpret_cast<CStatusDefinitions::Header*>(header.data());
        assert(pHeader->s_type == CStatusDefinitions::MessageTypes::RING_STATISTICS);
        
        std::cout << *pHeader << std::endl;
        
        sock.recv(&ringId);
        CStatusDefinitions::RingStatIdentification* pRingId =
            reinterpret_cast<CStatusDefinitions::RingStatIdentification*>(ringId.data());
        std::cout  << *pRingId << std::endl;
        
        std::uint64_t more(0);
        size_t        s(sizeof(more));
        
        sock.getsockopt(ZMQ_RCVMORE, &more, &s);
        
        // Process the statistics message segments;
        
        while(more) {
            zmq::message_t client;
            sock.recv(&client);
            CStatusDefinitions::RingStatClient* pClient =
                reinterpret_cast<CStatusDefinitions::RingStatClient*>(client.data());
                
            std::cout << *pClient << std::endl;
            
            sock.getsockopt(ZMQ_RCVMORE, &more, &s);
        }
        std::cout << "-------------------------\n";
    }
    
}

