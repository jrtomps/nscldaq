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
# @file   CTCLStateClientCommand.h
# @brief  Creational for state client command ensembles.
# @author <fox@nscl.msu.edu>
*/
#ifndef CTCLSTATECLIENTCOMMAND_H
#define CTCLSTATECLIENTCOMMAND_H

#include <TCLObjectProcessor.h>
#include <map>

class CTCLIntepreter;
class CTCLObject;
class CTCLStateClientInstanceCommand;

/**
 * @class CTCLStateClientCommand
 *
 * This command creates state client command ensembles.
 * a state client command ensemble represents:
 *    *  A connection to the REQ and PUB/SUB parts of the database that is
 *       aware of a program name.
 *    *  A command ensemble that allows an applicaion to manipulate this
 *       connection and, in an application with an event loop, to handle
 *       state transitions as an event handler.
 */
class CTCLStateClientCommand : public CTCLObjectProcessor
{
private:
    std::map<std::string, CTCLStateClientInstanceCommand*>  m_createdCommands;

    // canonicals
    
public:
    CTCLStateClientCommand(CTCLInterpreter& interp, const char* command);
    virtual ~CTCLStateClientCommand();

public:
    int operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);

private:
    void deleteEnsemble(std::string name);
    void createEnsemble(
        CTCLInterpreter& interp,
        std::string name, std::string reqUri, std::string subURI,
        std::string programName
    );
    
    
};


#endif