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
# @file   CVarMgrSubscribe.h
# @brief  The subscribe command - creates subscription subcommands.
# @author <fox@nscl.msu.edu>
*/

#ifndef CVARMGRSUBSCRIBE_H
#define CVARMGRSUBSCRIBE_H

#include <TCLObjectProcessor.h>
class CTCLInterpeter;
class CTCLObject;


/**
 * @class CVarMgrSubscribe
 *
 * Creator for subscription commands:
 *
 * \verbatim
 *    varmgr::subscribe uri command
 * \verbatim
 *
 * Creates a subscription command that is connected to the publication server
 * described by URI.  The subscription command is implemented as
 * CVarMgrSubCommand which implements a command ensemble.
 */
class CVarMgrSubscribe : public CTCLObjectProcessor
{
public:
    CVarMgrSubscribe(CTCLInterpreter& interp, const char* pCommand);
    virtual ~CVarMgrSubscribe();
    
    virtual int operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
private:
    std::pair<std::string, int>  decodeUri(std::string uri);
    int portFromService(std::string host, std::string service);
};

#endif
