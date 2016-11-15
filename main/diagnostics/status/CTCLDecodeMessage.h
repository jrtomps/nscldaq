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
# @file   CTCLDecodeMessage.h
# @brief  Provides decoders from status  messages to Tcl usable data.
# @author <fox@nscl.msu.edu>
*/
#ifndef CTCLDECODEMESSAGE_H
#define CTCLDECODEMESSAGE_H

#include <TCLObjectProcessor.h>
#include "CStatusMessage.h"


class CTCLInterpreter;
class CTCLObject;


/**
 * @class CTCLDecodeMessage
 *
 *   Command ensemble that knows how to decode status messages into
 *   Tcl usable information.  
 */
class CTCLDecodeMessage : public CTCLObjectProcessor
{
public:
    CTCLDecodeMessage(CTCLInterpreter& interp, const char* command = "statusdecode");
    virtual ~CTCLDecodeMessage();
    
    int operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    
    // Subcommand exectutors:
        
private:
    void decodeRingStatistics(CTCLInterpreter& interp, CTCLObject& msg);

    // Header handling:    
    
    const CStatusDefinitions::Header* extractHeader(CTCLObject& headerObj);
    CTCLObject  decodeHeader(
        CTCLInterpreter& interp, const CStatusDefinitions::Header& hdr
    );
    
    const CStatusDefinitions::RingStatIdentification* extractRingId(CTCLObject& ringIdObj);
    CTCLObject decodeRingIdent(
        CTCLInterpreter& interp, const CStatusDefinitions::RingStatIdentification& id
    );
    const CStatusDefinitions::RingStatClient* extractRingClientInfo(CTCLObject& clientObj);
    CTCLObject decodeRingClientInfo(
        CTCLInterpreter& interp, const CStatusDefinitions::RingStatClient& client
    );
};


#endif
