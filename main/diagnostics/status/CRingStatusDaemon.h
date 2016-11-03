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
# @file   CRingStatusDaemon.h
# @brief  Application class for the ring status daemon.
# @author <fox@nscl.msu.edu>
*/

#include <zmq.hpp>

class CPublishRingStatistics;

/**
 *  @class CRingStatusDaemon
 *     This class is the main application code for the ringstatus daemon.
 *     The daemon periodically sends ring status/statistics information
 *     to a specific ZMQ socket.  Normally, this socket is a PUSH socket
 *     that has been connected to the StatusAggregator service.  StatusAggregator
 *     is a proces that PULLs messages from several sources and proxies them into
 *     a PUB/SUB on a well known port (StatusAggregatorPUB).
 *
 *     This rigmarole allows status subscribers to be blissfully unaware of all
 *     the programs that can emit status information by providing a single
 *     point of subscription to the messages emitted by those program.s
 *     
 */
class CRingStatusDaemon
{
private:
    unsigned                m_interval;
    CPublishRingStatistics* m_pPublisher;
    bool                    m_active;
public:
    CRingStatusDaemon(zmq::socket_t& socket, unsigned interval);
    virtual ~CRingStatusDaemon();
    
public:
    void operator()();
    
    void halt() {m_active = false;}           // For testing ask the loop to stop.
};