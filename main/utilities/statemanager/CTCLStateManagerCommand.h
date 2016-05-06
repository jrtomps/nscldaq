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
# @file   CTCLStateManagerCommand.h
# @brief  Creator/deletor for state manager object/ensembles.
# @author <fox@nscl.msu.edu>
*/
#ifndef CTCLSTATEMANAGERCOMMAND_H
#define CTCLSTATEMANAGERCOMMAND_H
#include <TCLObjectProcessor.h>
#include <map>

class CTCLInterpreter;
class CTCLObject;
class CTCLStateManagerInstanceCommand;

/**
 * @class
 *    CTCLStateManagerCommand
 *
 *  This class is a creator/detroyer of state manager
 *  instances.  State manager instances are command esembles
 *  (objects) whose sub commands (methods) provide a TCL API
 *  to an instance of the CStateManager class.
 *
 *  There are two forms for this command:
 *
 * \verbatiom
 * statemanager mgrname requri suburi
 * \endverbatim
 *
 * and
 *
 * \verbatim
 * statemanager -delete mgrname
 * \verbatim
 *
 * where
 *    - mgrname is the name of a state manager ensemble that is
 *      being created or deleted.
 *    - requri - is the URI of the request port of a
 *      variable database server.
 *    - suburi - is the URI of the subscripton port of a
 *      variable database server.
 */

class CTCLStateManagerCommand :  public CTCLObjectProcessor
{
private:
    std::map<std::string, CTCLStateManagerInstanceCommand*> m_instances;
public:
    CTCLStateManagerCommand(CTCLInterpreter& interp, const char* command);
    virtual ~CTCLStateManagerCommand();
    
public:
    int operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
protected:
    void create(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void destroy(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
private:
    void destroy(
        std::map<std::string, CTCLStateManagerInstanceCommand*>::iterator p
    );

};
#endif