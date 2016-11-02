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
# @file   LogMessage.cpp
# @brief  Implement the LogMessage nested class.
# @author <fox@nscl.msu.edu>
*/

#include "CStatusMessage.h"
#include <os.h>
#include <cstdlib>
#include <cstring>

/**
 * constructor
 *    Constructs a log messagte emitter object for an application.
 *
 *  @param socket - socket on which the messages will be emitted.
 *  @param app    - Name of the application.
 */
CStatusDefinitions::LogMessage::LogMessage(zmq::socket_t& socket, std::string app) :
    m_socket(socket), m_application(app) {}
    
/** destructor:
 */
CStatusDefinitions::LogMessage::~LogMessage() {}

/**
 * CStatusDefinitions::LogMessage::Log
 *    Emits a log message
 *
 * @param sev  - message severity.
 * @paraM message - Message itself.
 */
void
CStatusDefinitions::LogMessage::Log(uint32_t sev, std::string message)
{
    // We need a header and a LogMessageBody where the latter is variable
    // sized:
    
    Header          hdr;
    formatHeader(
        hdr,
        MessageTypes::LOG_MESSAGE, sev, m_application.c_str()
    );
    
    
    size_t          bodySize = sizeof(LogMessageBody) + message.size() + 1;
    LogMessageBody* logBody =
        reinterpret_cast<LogMessageBody*>(calloc(1, bodySize));
    logBody->s_tod = std::time(nullptr);
    strcpy(logBody->s_message, message.c_str());
    
    // Send the messages:
    
    zmq::message_t header(sizeof(hdr));
    zmq::message_t body(bodySize);
    
    // The header:
    
    std::memcpy(header.data(), &hdr, sizeof(hdr));
    m_socket.send(header, ZMQ_SNDMORE);
    
    // The body:
    
    std::memcpy(body.data(), logBody, bodySize);
    m_socket.send(body, 0);
    
    free(logBody);
    
}