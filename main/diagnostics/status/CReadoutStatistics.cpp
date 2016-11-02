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
# @file   CReadoutStatistics.cpp
# @brief  Implement formatting of readout statistics messages.
# @author <fox@nscl.msu.edu>
*/
#include "CStatusMessage.h"
#include <stdexcept>

/**
 * CStatusDefinitions::CReadoutStatistics::CReadoutStatistics (constructor)
 *
 *   Create an instance of the class.
 */
CStatusDefinitions::ReadoutStatistics::ReadoutStatistics(
    zmq::socket_t& socket, std::string app
) :
    m_socket(socket),
    m_appName(app),
    m_runStartTime(0),
    m_haveOpenRun(false)
{
    
}

/**
 *  For now the destructor is empty:
 */
CStatusDefinitions::ReadoutStatistics::~ReadoutStatistics()
{}

/**
 * CStatusDefinitions::ReadoutStatistics::beginRun
 *    Describe a change in the run.   This allows a readout program
 *    to communicate a change in the run number and title as well as to establish
 *    a new timebase from which the run elapsed time is computed.  Note that
 *    if you really want to know when runs start, stop and so on, you should
 *    be looking at state change messages.
 *
 *  @param runNumber  - New run number in effect.
 *  @param title      - New title in effect.
 *  @note as a side-effect, the current time is set as the run start time.
 */
void
CStatusDefinitions::ReadoutStatistics::beginRun(
    uint32_t runNumber, std::string title
)
{
    // Mark that we've seen a run start and set the time base for the elapsed time
    
    m_runStartTime = std::time(nullptr);
    m_haveOpenRun  = true;
    m_runNumber    = runNumber;
    m_title        = title;
    
    
    // Format the header and send it:
    
    emitHeader();    
    
    // Format the run info and send it:
    
    ReadoutStatRunInfo info = formatIdent();
    
    zmq::message_t infoMsg(sizeof(info));
    memcpy(infoMsg.data(), &info, sizeof(info));
    
    m_socket.send(infoMsg, 0);
    
}
/**
 * CStatusDefinitions::ReadoutStatistics::emitStatistics
 *
 *   Emit a statistics message.
 *
 * @param triggers  - Number of triggers the system responded to.
 * @param events    - Number of events that were emitted.
 * @param bytes     - Number of bytes that were emitted.
 */
void CStatusDefinitions::ReadoutStatistics::emitStatistics(
    std::uint64_t triggers, std::uint64_t events, std::uint64_t bytes
)
{
    // We need to have at least one start of run in order to know how long
    // things have been running:
    
    if (!m_haveOpenRun) {
        throw std::logic_error("No run has evern been started");
    }
    emitHeader();    

    // We're sending the run ident:
    
    
    ReadoutStatRunInfo info = formatIdent();
    
    // And of course the statistics block:
    
    // Figure out the body fields and create one:
    
    time_t    now     = std::time(nullptr);    // Current time is needed.
    uint64_t  elapsed = now - m_runStartTime;
    
    ReadoutStatCounters stats = {now, elapsed, triggers, events, bytes};

    // Send the message body parts.

    zmq::message_t id(sizeof(info));
    zmq::message_t stat(sizeof(stats));
    
    std::memcpy(id.data(), &info, sizeof(info));
    std::memcpy(stat.data(), &stats, sizeof(stats));
    
    m_socket.send(id,  ZMQ_SNDMORE);
    m_socket.send(stat, 0);
}
/*----------------------------------------------------------------------------
 * Private utilities.
 */

/**
 *CStatusDefinitions::ReadoutStatistics::formatIdent
 *
 * Format the identification message:
 *
 * @return CStatusDefinitions::ReadoutStatRunInfo
 */
CStatusDefinitions::ReadoutStatRunInfo
CStatusDefinitions::ReadoutStatistics::formatIdent()
{
    ReadoutStatRunInfo info = {m_runStartTime, m_runNumber, ""};
    strncpy(info.s_title, m_title.c_str(), sizeof(info.s_title) -1);
    info.s_title[sizeof(info.s_title) - 1] = '\0';
    return info;    
}
/**
 * CStatusDefinitions::ReadoutStatistics::emitHeader
 *
 *   Emits a header item.
 */
void
CStatusDefinitions::ReadoutStatistics::emitHeader()
{
   Header hdr;
   formatHeader(
        hdr,
        MessageTypes::READOUT_STATISTICS, SeverityLevels::INFO,
        m_appName.c_str()
    );
   zmq::message_t h(sizeof(hdr));
   std::memcpy(h.data(), &hdr, sizeof(hdr));
   m_socket.send(h, ZMQ_SNDMORE);
}
