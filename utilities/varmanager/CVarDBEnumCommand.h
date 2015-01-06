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
# @file   CVarDbEnumCommand.h
# @brief  Tcl command class to create enumeration types.
# @author <fox@nscl.msu.edu>
*/

#ifndef _CENUMCOMMAND_H
#define _CENUMCOMMAND_H

#include "TCLObjectProcessor.h"

class CTCLInterpreter;
class CTCLObject;

/**
 * @class CVarDbEnumCommand
 *   Class that provides the enum command to Tcl.  This command is used
 *   to create enumerated data types for example:
 *
 *   /verbatim
 *   ::vardb::enum $dbhandle colors [list red green blue]
 *   ::vardb::variable create $dbhandle /myvar colors green
 *   /endverbatim
 *
 *  Creates a data type called colors which can have the values red, green
 *  or blue and then creates a varaible myvar in the root directory of that
 *  type with an initial value of green.
 */
class CVarDbEnumCommand : public CTCLObjectProcessor
{
    // canonicals
    
public:
    CVarDbEnumCommand(CTCLInterpreter& interp, const char* pCommand);
    virtual ~CVarDbEnumCommand();
    
    // Operations required by CTCLObjectProcessor:
    
public:
    int operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
};

#endif