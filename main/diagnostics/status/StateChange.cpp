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
# @file   StateChange.cpp
# @brief  Emit State change status messages.
# @author <fox@nscl.msu.edu>
*/

#include "CStatusMessage.h"
#include <ctime>
#include <cstring>
#include <zmq.hpp>
#include <stdexcept>

/**
 * CStatusDefinitions::StateChange::StateChange (constructor)
 *    Construct an object that can emit state changes on a ZMQ socket.
 *
 *  @param socket - References the socket that is used to transport
 *                  the information.
 *  @param app    - Name of the application.
 */
CStatusDefinitions::StateChange::StateChange(
    zmq::socket_t& socket, std::string app
) :
    m_socket(socket), m_application(app)
{}
/**
 * StatusDefinitions::StateChange::~StateChange
 *
 *   Destroy the object.
 */
CStatusDefinitions::StateChange::~StateChange() {}

/**
 * CStatusDefinitions::StateChange::logChange
 *    Log a state change
 *    - Emits a header of the appropriate type with INFO severity.
 *    - Formats and emits a StateChangeBody message.
 *  @param leaving  - The state that's being left.
 *  @param entering - The state that's being entered.
 */
void
CStatusDefinitions::StateChange::logChange(
    std::string leaving, std::string entering
)
{
    // The strings must be appropriately sized:
    
    
    // Format the header
    
    Header h;
    formatHeader(
        h,
        MessageTypes::STATE_CHANGE, SeverityLevels::INFO, m_application.c_str()
    );
    
    // Format the body:
    
    StateChangeBody body;
    std::memset(&body, 0, sizeof(body));
    
    // Strings must fit in state slots:
    
    if (leaving.size() >= sizeof(body.s_leaving)) {
        throw std::logic_error("Leaving state is too long for status message");
    }
    if (entering.size() >= sizeof(body.s_entering)) {
        throw std::logic_error("Entering state is too long for status message");
    }
    
    body.s_tod = std::time(nullptr);
    std::strcpy(body.s_leaving, leaving.c_str());
    std::strcpy(body.s_entering, entering.c_str());
    
    // Turn the structs into zmq messages::
    
    zmq::message_t hMsg(sizeof(Header));
    zmq::message_t bodyMsg(sizeof(body));
    
    std::memcpy(hMsg.data(), &h, sizeof(Header));
    std::memcpy(bodyMsg.data(), &body, sizeof(StateChangeBody));
    
    // Send the messages:
    
    m_socket.send(hMsg, ZMQ_SNDMORE);
    m_socket.send(bodyMsg, 0);
    
}
