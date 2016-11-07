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
# @file   aggregator.cpp
# @brief  Simple PULL -> PUB aggregator for status messages.
# @author <fox@nscl.msu.edu>
*/
#include <zmq.hpp>
#include <sstream>
#include <cstdint>
#include <CPortManager.h>
#include <Exception.h>
#include <cstdlib>
/**
 * This program advertises two services via the NSCL port manager.
 * The first service 'StatusAggregator' is a ZMQ PULL socket.  Status providers
 * can PUSH messages to that port.  'StatusPublisher' is a PUB socket.  All
 * messages received via the PULL socket are forwarded out to the PUB socket.
 *
 * This proxy service, therefore allows remote (and local for that matter)
 * programs to have a single, discoverable point at which to point SUB sockets
 * to receive all, or a subset of status information from _all_ providers without
 * the subscribers needing to know about the individual providers or the
 * providers needing to know about the subscribers.
 *
 */

static const char* PULL_SERVICE = "StatusAggregator";
static const char* PUB_SERVICE  = "StatusPublisher";

/**
 *  openPubSocket
 *     Opens the publication socket.
 *     -  Allocates a port for the 'StatusPublisher' service.
 *     -  Creates a new ZMQ PUB socket.
 *     -  binds the socket to the port indicated by the port manager.
 *
 *   @param manager - References the DAQ Port manager object.
 *   @param context - ZMQ context reference.
 *   @return zmq::socket_t*  - Pointerr to newly created socket.
 */
zmq::socket_t*
openPubSocket(CPortManager& manager, zmq::context_t& context)
{
    zmq::socket_t* result;
    int port = manager.allocatePort(PUB_SERVICE);
    result   = new zmq::socket_t(context, ZMQ_PUB);
    
    std::stringstream binding;
    binding << "tcp://*:" << port;
    result->bind(binding.str().c_str());
    
    return result;
}

/**
 * openPullSocket
 *     Advertise the port "StatusAggregator" with DAQPort manager and create a
 *     new zmq::socket_t*  that is a PULL type socket listening on that port
 *     for connections.
 *
 *   @param manager - DAQ port manager object.
 *   @param context - ZMQ Context (reference).
 *   @return zmq::socket_t* - socket bound to the allocated port.
 */
zmq::socket_t*
openPullSocket(CPortManager& manager, zmq::context_t& context)   
{
    int port = manager.allocatePort(PULL_SERVICE);
    zmq::socket_t* result = new zmq::socket_t(context, ZMQ_PULL);
    
    std::stringstream binding;
    binding << "tcp://*:" << port;
    result->bind(binding.str().c_str());
    
    return result;
}

/**
 * main
 *    Entry point - no command line parameters are required -- any provided at
 *    invocation time are silently ignored.
 */
int
main(int argc, char** argv)
{
    CPortManager manager;
    zmq::context_t  context(1);
    zmq::socket_t*  receiver  = openPullSocket(manager, context);
    zmq::socket_t*  publisher = openPubSocket(manager, context);
    
    while(1) {
        std::uint64_t more(0);
        size_t        s(sizeof(more));
        zmq::message_t msg;
        
        // Note that the logic below properly forwards multipart messages.
        
        receiver->recv(&msg);
        receiver->getsockopt(ZMQ_RCVMORE, &more, &s);
        
        publisher->send(msg, more ? ZMQ_SNDMORE : 0);
    }
}

