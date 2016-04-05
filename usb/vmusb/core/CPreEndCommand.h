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
# @file   CPreEndCommand.h
# @brief  Command that performs operations just prior to an end of run.
# @author <fox@nscl.msu.edu>
*/

#ifndef CPREENDCOMMAND_H
#define CPREENDCOMMAND_H
#include <TCLObjectProcessor.h>

class CTCLInterpreter;
class CTCLObject;

/**
 * @class CPreEndCommand
 *
 *     Provides an execution class that initiates the operations just prior
 *     to an end of run (specifically the performStopOperations).  Note that for
 *     users to use the performStopOperations can be a bit problematic because
 *     the VMUSB is still in daq/autonomous mode.  It is therefore necessary to
 *     use the acquisition control/command queues to ask it to give up the
 *     VM/USB with ACQ off and data flushed prior to doing anything interesting
 *     through the VMUSB (of course with a secondary controller all bets are
 *     off).
 */
class CPreEndCommand : public TCLObjectProcessor
{
public:
    CPreEndCommand(CTCLInterpreter& interp);
    virtual ~CPreEndCommand();
    
    int operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    
    void perform();
};


#endif