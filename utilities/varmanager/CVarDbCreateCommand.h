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
# @file   CVarDbCreateCommand.h
# @brief  Define a Tcl command class to create variable database files.
# @author <fox@nscl.msu.edu>
*/

#ifndef _CVARDBCREATECOMMAND_H
#define _CVARDBCREATECOMMAND_H

#include <TCLObjectProcessor.h>

class CTCLInterpreter;
class CTCLObject;

/**
 * @class CVarDbCreateCommand
 *
 *   Command processor to create/initialize a new variable database file.
 *
 * Syntax:
 * 
 *    <command-name> filename
 *
 * This is just a thin jacket for CVariablDb::create.
 */
class CVarDbCreateCommand : public CTCLObjectProcessor
{
    // Canonicals:
public:
    CVarDbCreateCommand(CTCLInterpreter& interp, const char* command);
    virtual ~CVarDbCreateCommand();
    
    // call interface:
    
    virtual int operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
};


#endif