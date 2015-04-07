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
# @file   CVarMgrSubCommand.h
# @brief  Dynamically created command object for subscription management.
# @author <fox@nscl.msu.edu>
*/

#ifndef CVARMGRSUBCOMMAND_H
#define CVARMGRSUBCOMMAND_H

#include <TCLObjectProcessor.h>
class CTCLInterpreter;
class CTCLObject;
class CVarMgrSubscriptions;

/**
 * @class CVarMgrSubCommand
 *
 *    This class is instantiated by the varmgr::subscribe command.
 *    it represents a command ensemble that actually subscribes to messages
 *    from a variable server.
 */
class CVarMgrSubCommand : public CTCLObjectProcessor
{
private:
    CVarMgrSubscriptions&  m_subscriptions;
    std::string            m_script;

public:
    CVarMgrSubCommand(
        CTCLInterpreter& interp, const char* command, CVarMgrSubscriptions& subs
    );
    virtual ~CVarMgrSubCommand();
    
    virtual int operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    
protected:
    void destroy(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void subscribe(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void read(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void wait(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void unsubscribe(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void notify(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    
    
private:
    static void readable(void* clientData, int mask);
};

#endif