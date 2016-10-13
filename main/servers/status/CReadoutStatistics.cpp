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
    // Format a header:
    
    
}
