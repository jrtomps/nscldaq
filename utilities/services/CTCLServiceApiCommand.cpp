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
# @file   CTCLServiceApiCommand.cpp
# @brief  implement the services creational
# @author <fox@nscl.msu.edu>
*/

#include "CTCLServiceApiCommand.h"
#include "CTCLServiceApiInstance.h"
#include <TCLInterpreter.h>
#include <TCLObject.h>
#include <Exception.h>
#include <stdexcept>

/**
 * constructor
 *  @param interp - Reference to the interpreter object on which we register.
 *  @param command - command base string.
 */
CTCLServiceApiCommand::CTCLServiceApiCommand(CTCLInterpreter& interp, const char* command):
    CTCLObjectProcessor(interp, command, true)
{}

/**
 * Destructor
 */
CTCLServiceApiCommand::~CTCLServiceApiCommand()
{
    while (!m_instanceCommands.empty()) {
        std::map<std::string, CTCLServiceApiInstance*>::iterator p =
            m_instanceCommands.begin();
        delete p->second;
        m_instanceCommands.erase(p);
    }
}

/**
 * operator()
 *    Implement the command itself.
 * @param interp - interpreter performing the command.
 * @param objv   - words that make up the command.
 * @return int   - TCL_OK, TCL_ERROR.
 */
int
CTCLServiceApiCommand::operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    bindAll(interp, objv);
    try {
        requireExactly(objv, 3, "services command needs name/-delete and uri/name");
        
        std::string p1(objv[1]);
        std::string p2(objv[2]);
        
        if (p1 == "-delete") {
            std::string command = p2;
            std::map<std::string, CTCLServiceApiInstance*>::iterator p =
                m_instanceCommands.find(command);
            if (p != m_instanceCommands.end()) {
                delete p->second;
                m_instanceCommands.erase(p);
            } else {
                throw std::logic_error("No such service instance command in delete");
            }
        } else {
        
            std::string command = p1;
            std::string uri     = p2;
            
            // NO duplicates allowed:
            
            if(m_instanceCommands.count(command) > 0) {
                throw std::logic_error("service - command already exists");
            }
            
            // Create the command and register it:
            
            m_instanceCommands[command] =
                new CTCLServiceApiInstance(interp, command.c_str(), uri);
        }
    }
    catch (std::exception& e) {
        interp.setResult(e.what());
        return TCL_ERROR;
    }
    catch (std::string msg) {
        interp.setResult(msg);
        return TCL_ERROR;
    }
    catch (CException& e) {
        interp.setResult(e.ReasonText());
        return TCL_ERROR;
    }
    catch (...) {
        interp.setResult("Unexpected exception type caught in service creator");
        return TCL_ERROR;
    }
    
    return TCL_OK;
}