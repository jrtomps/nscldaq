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
#include <cstdlib>
#include <cstring>

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
    m_msgOpen(false),
    m_producer(0)
{}

/**
 * CStatusDefinitions::RingStatistics::~RingStatistics
 *    Destroys the object. 
 */
CStatusDefinitions::RingStatistics::~RingStatistics()
{
    std::free(m_producer);              // Kill off dynamic storage.
    m_producer = 0;
}

/**
 * CStatusDefinitions::RingStatistics::startMessage
 *   Start a new message.
 *
 * @param ring - the name of the ring the messasge is associated with.l
 * @throw std::logic_error if there's an open message.
 */
void
CStatusDefinitions::RingStatistics::startMessage(std::string ring)
{
    if(m_msgOpen) {
        throw std::logic_error("Message is open");
    }
    
    m_ringName = ring;
    m_msgOpen  = true;
    
    // Newly opened messages don't have producers yet:
    
    std::free(m_producer);
    m_producer = 0;
}
/**
 * CStatusDefinitions::RingStatistics::addProducer
 *    Add the producer to the message.  Note that the producer is optional,
 *    however you can only add one of them to a message else a logic_error
 *    is thrown.
 *
 *  @param command - Command words that make up the consumer program.
 *  @param ops     - Number of puts to the ring.
 *  @param bytes   - Number of bytes sent to the ring
 */
void
CStatusDefinitions::RingStatistics::addProducer(
    std::vector<std::string> command, uint64_t ops, uint64_t bytes
)
{
    // if there's a producer throw:
    
    if(m_producer) {
        throw std::logic_error("Ring statistic messages can only have one producer");
    }
    // Figure out the size of the producer struct..it's going to be the size
    // of the base struct added to the space required for the strings:
    
    size_t pSize = sizeof(CStatusDefinitions::RingStatClient);
    for (int i = 0; i < command.size(); i++) {
        pSize += std::strlen(command[i].c_str()) + 1;   // +1 for the null terminator.        
    }
    pSize++;                                           // Extra null terminator.
    
    m_producer = reinterpret_cast<CStatusDefinitions::RingStatClient*>(
        std::calloc(pSize, 1)
    );
    // Fill in the storage:
    
    m_producer->s_operations = ops;
    m_producer->s_bytes      = bytes;
    m_producer->s_isProducer = true;
    
    char* p = m_producer->s_command;          // note this is prefilled with 0.
    for (int i = 0; i < command.size(); i++) {
        std::strcpy(p, command[i].c_str());
        p += std::strlen(command[i].c_str()) + 1;
    }
}
/**
 * CStatusDefinitions::RingStatistics::addConsumer
 *   Add a consumer to the message.
 *
 *   @param command  - Vector of command words.
 *   @param ops      - Number of get ops performed by this consumer.
 *   @param bytes    - Number of bytes gotten by this consumer.
 */
void
CStatusDefinitions::RingStatistics::addConsumer(
    std::vector<std::string> command, uint64_t ops, uint64_t bytes
)
{
    
}
