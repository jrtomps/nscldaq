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
# @file   CVarDbLsCommand.h
# @brief  Define class that provides ls functionality.
# @author <fox@nscl.msu.edu>
*/

#ifndef _CVARDBLSCOMMAND_H
#define _CVARDBLSCOMMAND_H


#include <TCLObjectProcessor.h>

class CTCLInterpreter;
class CTCLObject;

/**
 * @class CVarDbLsCommand
 *
 *    Provides directory listing functionality for the variable database
 *    tcl package.
 */
class CVarDbLsCommand : public CTCLObjectProcessor
{
    // canonical methods:
public:
    CVarDbLsCommand(CTCLInterpreter& interp, const char* command);
    virtual ~CVarDbLsCommand();
    
    // Methods required by the CTCLObjectProcessor interface:
    
public:
    int operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    
};


#endif