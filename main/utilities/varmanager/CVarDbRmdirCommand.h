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
# @file   CVarDbRmdirCommand.h
# @brief  Wrap CVarDirTree::rmdir for Tcl applications
# @author <fox@nscl.msu.edu>
*/
#ifndef __CVARDBRMDIRCOMMAND_H
#define __CVARDBRMDIRCOMMAND_H

#include <TCLObjectProcessor.h>

class CTCLInterpreter;
class CTCLObject;

/**
 * @class CVarDbRmdirCommand
 *
 *    Command execution class that provides access to the
 *    CVarDirTree::rmdir operation:
 * \verbatim
 *    <command-name>  <handle> <path>
 * \endverbatim
 *
 * Where
 *   - <command-name> is the name the command is registered as, normally
 *     vardb::rmdir
 *   - <handle> is a handle that was gotten when opening the device database.
 *   - <path>   is the path to delete. Note that only the terminal node of the
 *              path is deleted and it is an error to delete a non empty directory.
 */
class CVarDbRmdirCommand : public CTCLObjectProcessor
{
    // canonicals:
    
public:
    CVarDbRmdirCommand(CTCLInterpreter& interp, const char* command);
    virtual ~CVarDbRmdirCommand();
    
    // CTCLObject processor interface overrides:
    
public:
    virtual int operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
};


#endif
