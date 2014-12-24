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
# @file   CVarDbVariableCommand.h
# @brief  Class that implements the tcl vardb::variable command.
# @author <fox@nscl.msu.edu>
*/

#ifndef _CVARDBVARIABLECOMMAND_H
#define _CVARDBVARIABLECOMMAND_H

#include <TCLObjectProcessor.h>
#include "CVarDbOpenCommand.h"

class CTCLInterpreter;
class CTCLObject;



/**
 * @class CVarDbVariableCommand
 *
 *    This class provides a command ensemble for manipulating
 *    variables in the variable database.
 *    The ensemble provides the following subcommands:
 *
 *    - create  - creates new variables.
 *    - destroy - Destroys existing variables.
 *    - set     - Sets the value of an existing variable.
 *    - get     - Gets the value of an existing variable.
 *    - ls      - Lists the variables in a given directory path.
 *
 *    For more information about the syntax of specific subcommands, see
 *    the comments in the implementation file.
 */
class CVarDbVariableCommand : public CTCLObjectProcessor
{
    // Canonicals
public:
    CVarDbVariableCommand(CTCLInterpreter& interp, const char* command);
    
    // Operations required by the base class:
public:
    virtual int operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    
    // Subcommand processors
    
protected:
    void create(
        CTCLInterpreter& interp, std::vector<CTCLObject>& objv,
        CVarDbOpenCommand::HandleState& state
    );
    void destroy(
        CTCLInterpreter& interp, std::vector<CTCLObject>& objv,
        CVarDbOpenCommand::HandleState& state
    );
    void set(
        CTCLInterpreter& interp, std::vector<CTCLObject>& objv,
        CVarDbOpenCommand::HandleState& state
    );
    void get(
        CTCLInterpreter& interp, std::vector<CTCLObject>& objv,
        CVarDbOpenCommand::HandleState& state
    );
    void ls(
        CTCLInterpreter& interp, std::vector<CTCLObject>& objv,
        CVarDbOpenCommand::HandleState& state
    );
};
#endif