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
# @file   CTCLServiceApiCommand.h
# @brief  Define the service crational command.
# @author <fox@nscl.msu.edu>
*/
#ifndef CTCLSERVICEAPICOMMAND_H
#define CTCLSERVICEAPICOMMAND_H
#include <TCLObjectProcessor.h>
#include <map>
#include <string>

class CTCLInterpreter;
class CTCLObject;
class CTCLServiceApiInstance;

/**
 * @class CTCLServiceApiCommand
 *    Define a creational command for service objects.  Service objects
 *    are wrappers of the libService Api connected to a specific database instance
 *    via a specific method.
 */
class CTCLServiceApiCommand : public CTCLObjectProcessor
{
private:
    std::map<std::string, CTCLServiceApiInstance*>  m_instanceCommands;
public:
    CTCLServiceApiCommand(CTCLInterpreter& interp, const char* command);
    virtual ~CTCLServiceApiCommand();
    
    int operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
};
#endif
