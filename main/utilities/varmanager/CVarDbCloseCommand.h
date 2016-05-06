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
# @file   CVarDbCloseCommand.h
# @brief  Define command processor for the vardb package's close command.
# @author <fox@nscl.msu.edu>
*/

#ifndef _CVARDBCLOSECOMMAND_H
#define  _CVARDBCLOSECOMMAND_H

#include "TCLObjectProcessor.h"

class CTclInterpreter;
class CTclObject;

/**
 * @class CVarDbCloseCommand
 *    Provides a command that closes a variable database. This command
 *    is going to require the CVarDbOpenCommand class as we're going to use
 *    it's handle database (in fact CVarDbOpenCommand::close is actually going
 *    to do the underlying close).
 */
class CVarDbCloseCommand : public CTCLObjectProcessor
{
public:
    CVarDbCloseCommand(CTCLInterpreter& interp, const char* command);
    virtual ~CVarDbCloseCommand();
    
    // object operations for the CTCLObjectCommand interface:
    
    virtual int operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
};

#endif