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
# @file   CVarDbCdCommand
# @brief  Define class that provides the cd command for the vardb package.
# @author <fox@nscl.msu.edu>
*/

#ifndef _CVARDBCOMMAND_H
#define _CVARDBCOMMAND_H

#include <TCLObjectProcessor.h>

class CTCLInterpreter;
class CTCLObject;

/**
 * @class CVarDbCdCommand
 *
 *   Provides the change directory command to the vardb Tcl package.
 *   Syntax is:
 *\verbatim
 *  <command> <dbhandle> <path>
 * \endverbatim
 *
 * Where
 *  - <command> is the name of the command (usually ::vardb::cd)
 *  - <dbhandle> is a database handle that was gotten from an open operation.
 *  - <path> is a path (relative to the current cd or absolute).
 */
class CVarDbCdCommand : public CTCLObjectProcessor
{
    // Canonicals
public:
    CVarDbCdCommand(CTCLInterpreter& interp, const char* command);
    virtual ~CVarDbCdCommand();
    
    // CTCLObjectProcessor interface:
public:
    virtual int operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
};

#endif