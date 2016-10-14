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
CStatusDefinitions::LogMessage::Log(
    CStatusDefinitions::SeverityLevels sev, std::string message
)
{
    
}