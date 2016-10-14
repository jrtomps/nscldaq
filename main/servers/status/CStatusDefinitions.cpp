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
# @file   CStatusDefinitions.cpp
# @brief  Executable members of the outer CStatusDefinitions class
# @author <fox@nscl.msu.edu>
*/

#include "CStatusMessage.h"
#include <os.h>

/**
 * formatHeader
 *    Formats a message header.  The message header is the first message
 *    segment in a status message.  Therefore this method is used by all of the
 *    nested classes.
 *
 *   @param type     -- Type of message being created.
 *   @param severity -- Severity of the message.
 *   @param appName  -- Name of the application that's formatting this message.
 *   @return CStatusDefinitions::Header - the formatted header object.
 */
CStatusDefinitions::Header
CStatusDefinitions::formatHeader(uint32_t type, uint32_t severity, const char* appName)
{
    /* Build/send the header. */
    
      // Fill in the header fields that are easy:
      
    CStatusDefinitions::Header hdr = {
        type, severity, "", ""
    };
      // Fill in the application name.
      
    std::strncpy(
        hdr.s_application, appName,
        sizeof(hdr.s_application) - 1
    );
    hdr.s_application[sizeof(hdr.s_application) -1 ] = '\0';
    
        // Fill in the source with the fqdn of this host:
    
    std::string host = Os::hostname();
    std::strncpy(hdr.s_source, host.c_str(), sizeof(hdr.s_source) -1);
    hdr.s_source[sizeof(hdr.s_source) -1]  = 0;
    
    return hdr;    
}