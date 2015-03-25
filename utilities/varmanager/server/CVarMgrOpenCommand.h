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
# @file   CVarMgrOpenCommand.h
# @brief  Open a variable manager database (via factories).
# @author <fox@nscl.msu.edu>
*/
#ifndef CVARMGROPENCOMMAND_H
#define CVARMGROPENCOMMAND_H
#include <TCLObjectProcessor.h>

#include <map>
#include <string>

class CTCLInterpreter;
class CTCLObject;
class CVarMgrApi;

/**
 * @class CVarMgrOpenCommand
 *
 *    Class to implement the varmgr::open command.  This command
 *    manufactures an API object and associates a handle (in Tcl
 *    a text string) to stand in for that object.
 *
 * \verbatim
 *    open uri
 * \endverbatim
 */
class CVarMgrOpenCommand : public CTCLObjectProcessor
{
private:
    static std::map<std::string, CVarMgrApi*> m_OpenDatabases;
    static int                                m_handleNo;
public:
    CVarMgrOpenCommand(CTCLInterpreter& interp, const char* pCommand);
    virtual ~CVarMgrOpenCommand();
    
    int operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    
    // Handle management -- this requires a bit of exposure...other commands
    // need to get to API objects given handles and the close command needs
    // to destroy/remove an api object:
    
    static CVarMgrApi* handleToApi(const char* pHandle);
    static void close(const char* pHandle);
    
};

#endif
