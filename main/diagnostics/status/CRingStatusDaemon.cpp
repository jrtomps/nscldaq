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
# @file   CRingStatusDaemon.cpp
# @brief  Application class for the ring status/statistics daemon.
# @author <fox@nscl.msu.edu>
*/
#include "CRingStatusDaemon.h"
#include "CPublishRingStatistics.h"
#include <zmq.hpp>
#include <thread>
#include <chrono>

/**
 * constructor
 *    @param socket - socket on which the data will be published (pushed).
 *    @param interval - Number of seconds between status messages.
 */
CRingStatusDaemon::CRingStatusDaemon(zmq::socket_t& socket, unsigned interval) :
    m_interval(interval),
    m_pPublisher(new CPublishRingStatistics(socket, "RingStatisticsDaemon")),
    m_active(true)
{
    
}
/**
 * destructor   - kill off the publisher object.
 */
CRingStatusDaemon::~CRingStatusDaemon()
{
    delete m_pPublisher;
}


/**
 * operator()
 *    Execute the application:
 */
void
CRingStatusDaemon::operator()()
{
    while(m_active) {
        (*m_pPublisher)();                           // Publish the statistic
        std::this_thread::sleep_for(std::chrono::seconds(m_interval));
    }
    
}