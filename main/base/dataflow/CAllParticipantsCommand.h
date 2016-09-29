/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2014.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Authors:
             Ron Fox
             Jeromy Tompkins 
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

/**
 * @file CAllParticipantsCommand.h
 * @brief Tcl interface to CConnectivity::getAllParticipants.
 */

#ifndef CALLPARTICIPANTSCOMMAND_H
#define CALLPARTICIPANTSCOMMAND_H

#include <TCLObjectProcessor.h>

class CTCLInterpreter;
class CTCLObject;

/**
 * @class CAllParticipantsCommand
 *    Command processor for a Tcl command (getAllParticipants) which 
 *    returns the list of all hosts participating in the dataflow
 *    with the specified host as a starting point.
 *    Command syntax is;
 *
 * \verbatim
 *     getAllParticipants ?host?
 * \endverbatim
 * 
 *  Where host is a host that is known to participate in the dataflow.
 *  If the optional host is omitted, it defaults to "localhost"
 */

class CAllParticipantsCommand : public CTCLObjectProcessor
{
public:
  CAllParticipantsCommand(
    CTCLInterpreter& interp, const char* cmd = "getAllParticipants"
  );
  virtual ~CAllParticipantsCommand();

  int operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
};

#endif
