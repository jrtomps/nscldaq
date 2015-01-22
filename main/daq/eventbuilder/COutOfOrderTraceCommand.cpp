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
# @file   COutOfOrderTraceCommand.cpp
# @brief  Implement command to add out of order TCl callbacks.
# @author <fox@nscl.msu.edu>
*/

#include "COutOfOrderTraceCommand.h"
#include <TCLInterpreter.h>
#include <TCLObject.h>
#include <tcl.h>

#include "CFragmentHandler.h"

#include <list>
#include <string>
#include <algorithm>

/**
 * @class COOTraceCommandObserver
 *    This class is a CFragmentHandler::NonMonotonicTimestampObserver that
 *    holds a (possibly empty) list of commands that shouild be executed
 *    when observation occurs.  The commands will have the source id,
 *    the prior timestamp and the out of order timestamp appended to them
 *    before executed. If any script fails Tcl_BackgroundError is called
 *    with the error in the result of the interpreter.  In the event of
 *    error command processing will continue...potentially calling
 *    Tcl_BackgroundError more than once in an observation.
 */
class COOTraceCommandObserver : public CFragmentHandler::NonMonotonicTimestampObserver
{
    // Private data:
private:    
    CTCLInterpreter&       m_interp;      // Interpreter to use to execute the commands.
    std::vector<std::string> m_commands;    // Commands to execute.
    
    // Canonicals:
public:
    COOTraceCommandObserver(CTCLInterpreter& iterp);
    
    
    // Observation method required by the base class
    
    
    virtual void operator()(
        unsigned sourceid, uint64_t priorTimestamp, uint64_t thisTimestamp
    );
    
    // Other public methods
    
    void add(std::string command);
    void remove(std::string command);
    std::vector<std::string> list();
    
    
    // utility methods.

private:
    void doCommand(
        std::string, unsigned sourceId, uint64_t lastTs, uint64_t badTs
    );
};


/*------------------------------------------------------------------------------
 * Implementation of OutOfOrderTraceCommand
 *
 */

/**
 * constructor
 *
 * @param interp  - interpreter on which the command will be created/registered.
 * @param command - String that will invoke the command.
 */
COutOfOrderTraceCommand::COutOfOrderTraceCommand(
    CTCLInterpreter& interp, const char* pCommand
)  :
    CTCLObjectProcessor(interp, pCommand, true),
    m_pObserver(new COOTraceCommandObserver(interp))
{
    CFragmentHandler* pHandler = CFragmentHandler::getInstance();
    pHandler->addNonMonotonicTimestampObserver(m_pObserver);
    
}
/**
 * destructor
 */
COutOfOrderTraceCommand::~COutOfOrderTraceCommand()
{
    CFragmentHandler* pHandler = CFragmentHandler::getInstance();
    pHandler->removeNonMonotonicTimestampobserver(m_pObserver);
    delete m_pObserver;
}

/**
 * operator()
 *    Called in response to the command.
 *    - Bind all objv members to the interpreter.
 *    - Ensure we have a sub command and that it is legal.
 *    - dispatch to the appropriate command handler based on that subcommand.
 *
 *   @param interp   -  Interpreter that is running the command.
 *   @param objv     -  Vector of command word objects.
 *   @return int     - Return code to Tcl interptreter.
 *   @retval   TCL_OK - Correct command function.
 *   @retval   TCL_ERROR - Command failed, the interpreter result is an english
 *                        text indicating why.
 * @note Error handling is via exceptions that are thrown and caught in this
 *       function in  a way that turns the exception information into
 *       the result value.
 */
int
COutOfOrderTraceCommand::operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    bindAll(interp, objv);                       // Bind objv elements to the interp.
    try {
        if (objv.size() < 2) {
            throw std::string("Subcommand required but is missing");
        }
        
        // Pull out the subcommand and dispatch.  The final else of the if/elseif
        // chain below deals with bad subcommands:
        
        std::string subcommand = objv[1];
        if (subcommand == "add") {
            add(interp, objv);
        } else if (subcommand == "delete") {
            remove(interp, objv);
        } else if (subcommand == "list") {
            list(interp, objv);
        } else {
            throw std::string("Invalid subcommand");
        }
    }
    catch (std::string msg) {
        interp.setResult(msg);
        return TCL_ERROR;
    }
    
    return TCL_OK;
}

/**
 * add
 *   Adds a new command to the list of observer commands.  This just wraps the
 *   observer's add method.
 *  
 *   @param interp   -  Interpreter that is running the command.
 *   @param objv     -  Vector of command word objects.
 *
 * @note All errors are reported as std::string exceptions that the caller maps
 *       to Tcl interpreter results and ultimately TCL_ERROR returns.
 */
void
COutOfOrderTraceCommand::add(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    requireExactly(objv, 3);
    
    std::string command = objv[2];
    COOTraceCommandObserver* pObserver =
        reinterpret_cast<COOTraceCommandObserver*>(m_pObserver);
    pObserver->add(command);
    
}
/**
 * remove
 *    Remove a specific command from the set the observer runs.
 *    Note that if the command was registered more than once,
 *    the least recently registered one is removed.
 *    It is  a silent no-op to attempt to remove a command
 *    that is not registered.
 *
 *   @param interp   -  Interpreter that is running the command.
 *   @param objv     -  Vector of command word objects.
 *
 * @note All errors are reported as std::string exceptions that the caller maps
 *       to Tcl interpreter results and ultimately TCL_ERROR returns.
 */
void
COutOfOrderTraceCommand::remove(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    requireExactly(objv, 3);
    
    std::string command = objv[2];
    COOTraceCommandObserver* pObserver =
        reinterpret_cast<COOTraceCommandObserver*>(m_pObserver);
    pObserver->remove(command);
    
}
/**
 * list
 *   Sets the result to be a Tcl list containing the list of commands
 *   that are registered on the observer.
 *   @param interp   -  Interpreter that is running the command.
 *   @param objv     -  Vector of command word objects.
 *
 * @note All errors are reported as std::string exceptions that the caller maps
 *       to Tcl interpreter results and ultimately TCL_ERROR returns.
 */
void
COutOfOrderTraceCommand::list(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    requireExactly(objv, 2);
    COOTraceCommandObserver* pObserver =
        reinterpret_cast<COOTraceCommandObserver*>(m_pObserver);
        
    std::vector<std::string> commands = pObserver->list();
    CTCLObject result;
    result.Bind(interp);
    for(int i = 0; i < commands.size(); i++) {
        result += commands[i];
    }
    interp.setResult(result);
}
/*-----------------------------------------------------------------------------
 * Implementation of the COOTraceCommandObserver class.
 */

/**
 * constructor
 *    @param interp - The interpreter on which commands run by this observer
 *                     should execute.
 */
COOTraceCommandObserver::COOTraceCommandObserver(CTCLInterpreter& interp):
    m_interp(interp)
{}

/**
 * operator()
 *    Called to observe events.
 *    - Iterate through the commands and invoke doCommand for each of them.
 *
 * @param sourceId - Id of the queue reporting the event.
 * @param prior    - Timestamp prior to the bad one.
 * @param current  - Bad timestamp.
 */
void
COOTraceCommandObserver::operator()(unsigned sourceId, uint64_t prior, uint64_t current)
{
    for (int i = 0; i < m_commands.size(); i++) {
        doCommand(m_commands[i], sourceId, prior, current);
    }
}
/**
 * add
 *
 *   Add a new command to the list of commands.
 *
 * @param command - new command to add.
 *
 * It is perfectly fine to register the same command more than once.
 * See remove however.
 */
void
COOTraceCommandObserver::add(std::string command)
{
    m_commands.push_back(command);
}
/**
 * remove
 *    Remove a command from the set.
 *  @param command - command to remove.
 *
 *  @note it is a no op to remove a comment that's not in the list.
 *  @note If the command is in the list more than once the least recently
 *        added command is removed.
 */
void
COOTraceCommandObserver::remove(std::string command)
{
    std::vector<std::string>::iterator p =
        std::find(m_commands.begin(), m_commands.end(), command);
    if (p != m_commands.end()) {
        m_commands.erase(p);
    }
}
/**
 * list
 *   Return the vector of command observers.
 * @return std::vector<std::string>>
 */
std::vector<std::string>
COOTraceCommandObserver::list()
{
    return m_commands;
}
/**
 * doCommand
 *    Builds a command object from the command string, and the
 *    parameters then attempts to run it.  If there is an error,
 *    Tcl_BackgroundError is invoked.
 *
 * @param base - the base command to run.
 * @param id   - Source Id parameter.
 * @param last - Prior timestamp.
 * @param bad  - bad timestamp that caused the ruckus.
 */
void
COOTraceCommandObserver::doCommand(
    std::string base, unsigned id, uint64_t last, uint64_t bad
)
{
    
    
    CTCLObject command;
    command.Bind(m_interp);
    CTCLObject  wide;
    wide.Bind(m_interp);
    
    command += base;
    command += static_cast<int>(id);
    wide     = static_cast<Tcl_WideInt>(last);
    command += wide;
    wide     = static_cast<Tcl_WideInt>(bad);
    command += wide;
    
    int status = Tcl_GlobalEvalObj(m_interp.getInterpreter(), command.getObject());
    if (status != TCL_OK) {
        Tcl_BackgroundError(m_interp.getInterpreter());
    }
}


