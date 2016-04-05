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
# @brief  Perform actions prior to actual end of run.
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
 *      The preend command performs the end run actions prior to turning off
 *      data taking mode for the CCUSB and flushing buffers.  These are currently
 *      the stop actions in the high level controller.  Note to authors of that
 *      code that you need to turn off data taking and flush pending data out to
 *      the router in order to perform useful work there so this is a bit
 *      of a difficult bit of code to write.  This can be done, however by doing an
 *      ACQUIRE and RELEASE operation with the readout thread prior to doing
 *      your dirty work.
 */
class CPreEndCommand : public CTCLObjectProcessor
{
public:
    CPreEndCommand(CTCLInterpreter& interp);
    virtual ~CPreEndCommand();
    
public:
    int operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
};

#endif