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
# @file   <filename>
# @brief  <brief description>
# @author <fox@nscl.msu.edu>
*/

#ifndef CPREENDCOMMAND_H
#define CPREENDCOMMAND_H

#include <TCLPackagedObjectProcessor.h>
class CTCLInterpreter;
class CTCLObject;

/**
 * @class CPreEndCommand
 *     This class executes the preend command.   That command is invoked just prior
 *     to ending a run and is expected to perform all of the end run tear down
 *     needed just prior to actually stopping the run.   This normally consists
 *     of work to ensure there will be no more triggers, while keeping the trigger
 *     loop alive.
 *
 */
class CPreEndCommand : public CTCLPackagedObjectProcessor
{
public:
    CPreEndCommand(CTCLInterpreter& interp);
    virtual ~CPreEndCommand();
    
public:
    virtual int operator()(
        CTCLInterpreter& interp, std::vector<CTCLObject>& objv
    );
};

#endif