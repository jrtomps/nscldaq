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
# @file   CStateManagerCommand.cpp
# @brief  implemenet the statemanager command.
# @author <fox@nscl.msu.edu>
*/

#include "CTCLStateManagerCommand.h"
#include <TCLInterpreter.h>
#include <TCLObject.h>
#include <Exception.h>
#include "CTCLStateManagerInstanceCommand.h"
#include <stdexcept>




/**
 * constructor
 *    @param interp - the interpreter on which the command is
 *           registered
 *    @param command - name of the command.
 *    
 */
CTCLStateManagerCommand::CTCLStateManagerCommand(
    CTCLInterpreter& interp, const char* command
) : CTCLObjectProcessor(interp, command, true)
{}

/**
 * destructor
 *   Must destroy all the commands I've created.
 */
CTCLStateManagerCommand::~CTCLStateManagerCommand()
{

    /*
     *  This loop is this way because deleting a map entry invalidates
     *  the iterator used.
     */
    
    while (!m_instances.empty()) {
        std::map<std::string, CTCLStateManagerInstanceCommand*>::iterator
            p = m_instances.begin();
            destroy(p);
            
    }
}
/**
 * operator()
 *    Processes the command.  At this level we just differentiate
 *    between -delete and creation:
 *
 * @param interpreter - References the interpeter running the command.
 * @param objv        - Vector of words that make up the command.
 * @return int        - TCL_OK - success, TCL_ERROR on error.
 */
int
CTCLStateManagerCommand::operator()(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    bindAll(interp, objv);

    try {
        requireAtLeast(objv, 2, "Insufficient parameters");
        
        if(std::string(objv[1])== "-delete") {
            destroy(interp, objv);
        } else {
            create(interp, objv);
        }
    }
    catch (std::exception & e) {
        interp.setResult(e.what());
        return TCL_ERROR;
    }
    catch (CException& e) {
        interp.setResult(e.ReasonText());
        return TCL_ERROR;
    }
    catch (std::string msg) {
        interp.setResult(msg);
        return TCL_ERROR;
    }
    catch (...) {
        interp.setResult("Unanticipated exception type caught");
        return TCL_ERROR;
    }
    
    return TCL_OK;
}

/**
 * create
 *    Create an instance and register it.
 *
 * @param interp - interpreter executing the command.
 * @param objv   - Vector of command words.
 */
void
CTCLStateManagerCommand::create(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    requireExactly(objv, 4, "Exactly 4 parameters are required");
    
    std::string name   = objv[1];
    std::string requrl = objv[2];
    std::string suburl = objv[3];
    
    CTCLStateManagerInstanceCommand* pCommand =
        new CTCLStateManagerInstanceCommand(interp, name, requrl, suburl);
    m_instances[name] = pCommand;
}
/**
 * destroy
 *    Process the -destroy switch
 * @param interp - interpreter executing the command.
 * @param objv   - Vector of command words.
 */
void
CTCLStateManagerCommand::destroy(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    requireExactly(objv, 3, "-delete requires exactly 3 parameters");
    std::string command = objv[2];
    
    std::map<std::string, CTCLStateManagerInstanceCommand*>::iterator p =
        m_instances.find(command);
    if (p != m_instances.end()) {
        destroy(p);
    } else {
        throw std::invalid_argument("No such instance command");
    }
}
/*--------------------------------------------------------------------------
 * utilities
 */


/**
 * destroy
 *    Destroy an instance object given an iterator to it in the map.
 *
 *  @param p - iterator pointing to the map entry.
 */
void
CTCLStateManagerCommand::destroy(
    std::map<std::string, CTCLStateManagerInstanceCommand*>::iterator p
)
{
    // This order is used to ensure the item is not deleted but still
    // in the map.
    
    CTCLStateManagerInstanceCommand* pInstance = p->second;
    m_instances.erase(p);
    delete pInstance;
}