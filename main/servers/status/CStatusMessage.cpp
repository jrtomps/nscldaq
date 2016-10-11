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
*/

/**
* @file   CStatusMesszage.cpp
* @brief  Implement API to implement status messages.
* @author <fox@nscl.msu.edu>
* @note See the nscldaq redmine wiki at:
*       https://swdev-redmine.nscl.msu.edu/projects/sfnscldaq/wiki/NSCLDAQ_status_aggregation
*       for more information about all of this.
*/


#include "CStatusMessage.h"
#include <stdexcept>

/**
 * CStatusDefinitions::RingStatistics::RingStatistics
 *    Construct a ring statistics object.  This sort of object accumulates
 *    the bits and pieces needed to send ring statistics messages and
 *    sends them.
 *
 *   @param socket - The ZMQ socket used to transport the messages.  This
 *                   code is agnostic about the type of the underlying
 *                   transport so long as the socket is in a state that
 *                   allows messages to be sent when endMessage is called.
 *  @param app     - Identifies the application.  This is put unmodified in the
 *                   message header part.
 */
CStatusDefinitions::RingStatistics::RingStatistics(zmq::socket_t& sock, std::string app) :
    m_socket(sock),
    m_applicationName(app),
    m_msgOpen(false)
{}

/**
 * CStatusDefinitions::RingStatistics::~RingStatistics
 *    Destroys the object.  It is a logic_error to do this when a message is
 *    still open:
 */
CStatusDefinitions::RingStatistics::~RingStatistics()
{
    if (m_msgOpen) {
        throw std::logic_error("RingStatistics object destroyed with open message");
    }
}
