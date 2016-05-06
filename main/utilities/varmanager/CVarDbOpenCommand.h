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
# @file   CVarDbOpenCommand.h
# @brief  Define command processor class for var db open command.
# @author <fox@nscl.msu.edu>
*/

#ifndef _CVARDBOPENCOMMAND_H
#define _CVARDBOPENCOMMAND_H

#include <TCLObjectProcessor.h>
#include <string>
#include <map>

class CTCLInterpreter;
class CTCLObject;
class CVariableDb;
class CVarDirTree;
class CVarDbOpenCommand;



/**
 * @class CVarDbOpenCommand
 *
 * Processes the open command in the vardb package.  This is typically assigned the
 * command name ::vardb::open.  The open command takes a single parameter, a database
 * file, and creates a CVariableDb object from it.  That object is assigned a
 * handle string and that is returned to the script.
 * The class maintains a map of handle -> CVariableDb pointers (static) and provides a static
 * method to obtain the CVariableDb pointer associated with a handle from the
 * handle string.  This allows other commands to be written using only the
 * handle to refer to the database.
 *
 * Note that this makes handles shared across all interpreter instances.
 */
class CVarDbOpenCommand : public CTCLObjectProcessor
{
    // Exported data types:
public:
    typedef struct _HandleState {
        CVariableDb* s_db;                            // actual database object.
        CVarDirTree* s_cd;                            // cwd state for handle
    } HandleState, *pHandleState;
    
    // Private storage.
private:
    static std::map<std::string, HandleState> m_handleMap;
    static int m_handleNo;
    
    
    // Static public methods:
public:
    static HandleState  translateHandle(std::string handle);
    static void         close(std::string handle);
    static std::string  nextHandle();
    
    // Canonicals:
public:    
    CVarDbOpenCommand(CTCLInterpreter& interp, const char* command);
    virtual ~CVarDbOpenCommand();
    
    // Interface methods:
    
    int operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
};

#endif