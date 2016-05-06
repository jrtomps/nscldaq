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
# @file   CVarDbMkdirCommand.h
# @brief  Define tcl bindings to make directories in the variable database
# @author <fox@nscl.msu.edu>
*/

#ifndef _CVARDBMKDIRCOMMAND_H
#define _CVARDBMKDIRCOMMAND_H


#include <TCLObjectProcessor.h>

class CTCLInterpreter;
class CTCLObject;

/**
 * @class CVarDbMkdirCommand
 *
 *    This class provides support for the mkdir command in the variable database
 *    Tcl binding.
 *
 *    The command syntax is:
 *
 *\verbatim
 *    <command> ?-nopath? <database> <dirpath>
 *
 * \endverbatim
 * Where:
 *   - <command> is the command name, usually ::vardb::mkdir
 *   - <database> Is a database handle gotten from the open operation
 *   - <dirpath> Is a directory path.
 *   - -nopath if provided indicates that all directories in the path up until
 *             the terminal node must already exist.  Without it intermediate path
 *             elements are created automatically.
 */
class CVarDbMkdirCommand : public CTCLObjectProcessor
{
    // Canonicals:
    
public:
    CVarDbMkdirCommand(CTCLInterpreter& interp, const char* command);
    virtual ~CVarDbMkdirCommand();
    
    // Object processor interface:
    
    int operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
};

#endif