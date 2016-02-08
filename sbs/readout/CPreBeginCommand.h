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
# @file   CPreBeginCommand.h
# @brief  Command class for the prebegin command - two phase run starts.
# @author <fox@nscl.msu.edu>
*/
#ifndef CPREBEGINCOMMAND_H
#define CPREBEGINCOMMAND_H




#include <TCLPackagedObjectProcessor.h>
#include <vector>
#include <string>


// Forward class defs:

class CTCLInterpreter;
class CTCLObject;


/**
 * @class CPreBeginCommand
 *    This class is a command processor for the prebegin command.  The prebegin
 *    command provides support for a two stage state transition from inactive to
 *    active.  prebegin asks the experiment specific code to perform device
 *    initialization and transitions to the 'starting' state.  From here
 *    a begin command is required to clear digitizers, start the trigger loop
 *    and transition to the active (data taking state).
 *
 *    Note that this separation is optional.  If no prebegin command is issued
 *    prior to the begin command, the begin command functions as before.
 */
class CPreBeginCommand : public CTCLPackagedObjectProcessor
{
public:
    CPreBeginCommand(CTCLInterpreter& interp);
    virtual ~CPreBeginCommand();
    
public:
    virtual int operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);

};
#endif