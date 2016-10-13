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
# @file   Constants.cpp
# @brief  Provide storage for the  constants of CStatusDefinitions:
# @author <fox@nscl.msu.edu>
*/
#include <CStatusMessage.h>


// MessageTypes:

const std::uint32_t CStatusDefinitions::MessageTypes::RING_STATISTICS          = 0x00000000;
const std::uint32_t CStatusDefinitions::MessageTypes::EVENT_BUILDER_STATISTICS = 0x00000001;
const std::uint32_t CStatusDefinitions::MessageTypes::READOUT_STATISTICS       = 0x00000002;
const std::uint32_t CStatusDefinitions::MessageTypes::LOG_MESSAGE              = 0x00000003;
const std::uint32_t CStatusDefinitions::MessageTypes::STATE_CHANGE             = 0x00000004;

// keep this updated as new types are added:

const std::uint32_t CStatusDefinitions::MessageTypes::FIRST_FREE_TYPE          = 0x00000005;


const std::uint32_t CStatusDefinitions::MessageTypes::FIRST_USER_TYPE          = 0x80000000;

/// Messages severity levels:

 const std::uint32_t CStatusDefinitions::SeverityLevels::DEBUG   = 0x00000000;
 const std::uint32_t CStatusDefinitions::SeverityLevels::INFO    = 0x00000001;
 const std::uint32_t CStatusDefinitions::SeverityLevels::WARNING = 0x00000002;
 const std::uint32_t CStatusDefinitions::SeverityLevels::SEVERE  = 0x00000003;
 const std::uint32_t CStatusDefinitions::SeverityLevels::DEFECT  = 0x00000004;