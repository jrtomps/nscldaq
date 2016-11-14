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

class CTCLInterpreter;
class CTCLObject;

/**
 * @class CTCLDecodeMessage
 *
 *   Command ensemble that knows how to decode status messages into
 *   Tcl usable information.  Subcommands describe the type of message being
 *   decoded e.g.:
 * \verbatim
 *    statusdecode ringstatistics part-list
 * \endverbatim
 *    returns the decoded ring statistics message whose raw message parts are
 *    in part-list.
 */
class CTCLDecodeMessage : public CTCLObjectProcessor
{
public:
    CTCLDecodeMessage(CTCLInterpreter& interp, const char* command = "statusdecode");
    virtual ~CTCLDecodeMessage();
    
    int operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    
    // Subcommand exectutors:
        
private:
    void decodeRingStatistics(
        CTCLInterpreter& interp, std::vector<CTCLObject>& objv
    );
};


#endif
