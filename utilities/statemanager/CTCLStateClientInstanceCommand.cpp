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
# @file   CTCLStateClientInstanceCommand.cpp
# @brief  Implements Tcl ensemble representing a state client intance.
# @author <fox@nscl.msu.edu>
*/


#include "CTCLStateClientInstanceCommand.h"
#include "CTCLStateClientCommand.h"
#include "TCLInterpreter.h"
#include "TCLObject.h"
#include "CStateClientApi.h"
#include <stdexcept>
#include <stdio.h>


/**
 * constructor
 *
 * @param interp  - Interpreter reference
 * @param name    - New command to create/register (ensemble name)
 * @param requri  - URI for the request port to the variable database server.
 * @param suburi  - URI for the subscription port to the variable database server.
 * @param programName - Name of the program we are attached to.
 * @param pRegistry - The creator of instances.
 * 
 */
CTCLStateClientInstanceCommand::CTCLStateClientInstanceCommand(
    CTCLInterpreter& interp, std::string name,
    std::string requri, std::string suburi,
    std::string programName,
    CTCLStateClientCommand* pRegistry
) :
    CTCLObjectProcessor(interp, name.c_str(), true),
    m_pClient(0),
    m_pPumpThread(0)
{
    m_pClient = new CStateClientApi(
        requri.c_str(), suburi.c_str(), programName.c_str()
    );
    m_pPumpThread = new MessagePump(
        m_pClient, Tcl_GetCurrentThread(), this, pRegistry
    );
    m_pPumpThread->start();
    
    
}
/**
 * destructor
 */
CTCLStateClientInstanceCommand::~CTCLStateClientInstanceCommand()
{
    m_pPumpThread->scheduleExit();
    m_pPumpThread->join();
    delete m_pPumpThread;
    
    delete m_pClient;
}

/**
 * operator()
 *    Dispatches the command.
 *
 *  @param interp - interpreter executing the command.
 *  @param objv   - Vector representing the command words.
 */
int
CTCLStateClientInstanceCommand::operator()(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    bindAll(interp, objv);
    try {
        requireAtLeast(
            objv, 2, "Command needs at least a subcommand"
        );
        std::string subcommand = objv[1];
        if (subcommand == "getstate") {
            getState(interp, objv);
        } else if (subcommand == "setstate") {
            setState(interp, objv);
        } else if (subcommand == "isenabled") {
            isEnabled(interp, objv);
        } else if (subcommand == "isstandalone") {
            isStandalone(interp, objv);
        } else if (subcommand == "title") {
            title(interp, objv);
        } else if (subcommand == "runnumber") {
            runNumber(interp, objv);
        } else if (subcommand == "recording") {
            recording(interp, objv);
        } else if (subcommand == "outring") {
            outring(interp, objv);
        } else if (subcommand == "inring"){
            inring(interp,objv);
        } else if (subcommand == "onStateChange") {
            onStateChange(interp, objv);
        } else {
            throw std::invalid_argument("Invalid subcommand");
        }
    }
     catch (std::string msg) {
        interp.setResult(msg);
        return TCL_ERROR;
    }
    catch (std::exception& e) {
        interp.setResult(e.what());
        return TCL_ERROR;
    }
    catch (CException& e) {
        interp.setResult(e.ReasonText());
        return TCL_ERROR;
    }
    catch (...) {
        interp.setResult("stateclient Unanticipated exception type caught");
        return TCL_ERROR;
    }
    
    
    return TCL_OK;
}

/*----------------------------------------------------------------------
 * Subcommand executors:
 */

/**
 * getState
 *   Set the interpreter result with the program's current state.
 * @param interp - interpreter executing the command.
 * @param objv   - command words.
 */
void
CTCLStateClientInstanceCommand::getState(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    requireExactly(objv, 2, "getstate subcommand takes no other parameters");
    interp.setResult(m_pClient->getState());    
}
/**
 * setState
 *    Sets a new program state.  Requires an additional parameter that
 *    is the new state.
 * 
 * @param interp - interpreter executing the command.
 * @param objv   - command words.
 */
void
CTCLStateClientInstanceCommand::setState(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    requireExactly(objv, 3, "setstate subcommand needs (only) a new state");
    std::string newState = objv[2];
    m_pClient->setState(newState);
}

/**
 * isEnabled
 *   Set interpreter result to 1 if program is enabled else to 0.
 *
 * @param interp - interpreter executing the command.
 * @param objv   - command words.
 */
void
CTCLStateClientInstanceCommand::isEnabled(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    requireExactly(objv, 2, "isenabled  has no additional parameters");
    
    interp.setResult(m_pClient->isEnabled() ? "1" : "0");
}
/**
 * isStandalone
 *   Sets the interpreter result to 1 if the program is standalone
 *   0 otherwise.
 *
 * @param interp - interpreter executing the command.
 * @param objv   - command words.
 */
void
CTCLStateClientInstanceCommand::isStandalone(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    requireExactly(objv, 2, "isstandalone has no additional parameters");
    interp.setResult(m_pClient->isStandalone() ? "1" : "0");
}

/**
 * title
 *   Set the interpreter result with the title value.
 * @param interp - interpreter executing the command.
 * @param objv   - command words.
 */
void
CTCLStateClientInstanceCommand::title(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    requireExactly(objv, 2, "title needs no additional parameters");
    interp.setResult(m_pClient->title());
}
/**
 * runNumber
 *   set the interpreter result with the current run number.
 *
 * @param interp - interpreter executing the command.
 * @param objv   - command words.
 */
void
CTCLStateClientInstanceCommand::runNumber(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    requireExactly(objv, 2, "runnumber needs no additional parameters");
    
    unsigned run = m_pClient->runNumber();
    char runString[1000];
    sprintf(runString, "%u", run);
    interp.setResult(runString);
}
/**
 * recording
 *    set the interpreter result to 1 if recording is on
 *    0 otherwise.
 * @param interp - interpreter executing the command.
 * @param objv   - command words.
 */
void
CTCLStateClientInstanceCommand::recording(
   CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    requireExactly(objv, 2, "recording needs no additional parameters");
    interp.setResult(m_pClient->recording() ? "1" : "0");
}
/**
 * outring
 *    Set the interpreter result to the output ring name.
 * @param interp - interpreter executing the command.
 * @param objv   - command words.
 */
void
CTCLStateClientInstanceCommand::outring(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    requireExactly(objv, 2, "outring needs no additional parameters");
    interp.setResult(m_pClient->outring());
}
/**
 * inring
 *   set the interpreter result to the input ring name.
 *   @param interp - interpreter executing the command.
 * @param objv   - command words.
 */
void
CTCLStateClientInstanceCommand::inring(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    requireExactly(objv, 2, "inring needs no additional parameters");
    interp.setResult(m_pClient->inring());
}
/**
 * onStateChange
 *    Replaces the current state change handler.  Note that an empty string
 *    eliminates state change handling.
 *
 *  @param interp - interpreter running the command.
 *  @param objv   - Command word objects.. needs a script parameter.
 */
void
CTCLStateClientInstanceCommand::onStateChange(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv       
)
{
    requireExactly(objv, 3, "onStateChange needs (only) a script parameter");
    m_stateChangeScript = std::string(objv[2]);
}
/*-----------------------------------------------------------------------
 * Callback handling
 */

/**
 * stateChangeHandler
 *    This method is scheduled for state changes by the message pump.
 *    If m_stateChangeScript is non empty the new state is appended and
 *    the script is run in global scope.
 *
 *    Regardless, the event is freed.
 * @param pEvent - pointer to the Tcl event that will be a *pUEvent.
 * @param flags  - value of flags from Tcl_ServiceEvent.
 */
int
CTCLStateClientInstanceCommand::stateChangeHandler(Tcl_Event* pEvent, int flags)
{
    pUEvent pE = reinterpret_cast<pUEvent>(pEvent);

    /* If the object no longer exists we need to get out of here real damned quick */
    
    if (pE->u_fullEvent.s_pRegistry->isViable(pE->u_fullEvent.s_pObject)) {
    
        std::string newState(pE->u_fullEvent.s_newState);
        CTCLStateClientInstanceCommand* pObject = pE->u_fullEvent.s_pObject;
        
        // If there is a handler, construct the script:
        
        std::string script = pObject->m_stateChangeScript;
        if  (script != "") {
            script += " ";
            script += newState;
            CTCLInterpreter* pInterp = pObject->getInterpreter();
            pInterp->GlobalEval(script);
        }
    }
    
    // Free storage before I forget:
    
    Tcl_Free(pE->u_fullEvent.s_newState);
    
    return 1;
}

/*-------------------------------------------------------------------
 *  Message pump implementation:
 */

/**
 * construtor
 *   @param pClient - pointer to the shared client api.
 *   @param parent  - thread id of the parent.
 */
CTCLStateClientInstanceCommand::MessagePump::MessagePump(
    CStateClientApi* pClient, Tcl_ThreadId parent,
    CTCLStateClientInstanceCommand* outerObject,
    CTCLStateClientCommand* pRegistry
) : m_pRegistry(pRegistry), m_pClient(pClient), m_parent(parent), m_exit(false),
    m_pOuterObject(outerObject)
{}

/**
 * init - synchronized init
 */
void
CTCLStateClientInstanceCommand::MessagePump::init() {}

/**
 * scheduleExit
 *   schedule the thread to exit at the next chance.
 */
void
CTCLStateClientInstanceCommand::MessagePump::scheduleExit()
{
    m_exit = true;
}
/**
 * operator()
 *   Main loop, process messages and schedule events
 *   until m_exit is true:
 *
 */
void
CTCLStateClientInstanceCommand::MessagePump::operator()()
{
    while(! m_exit) {
        std::string newState;
        m_pClient->waitTransition(newState,  250);
        
        // If our outer object is interested:
        // Create the event structure and fill it in:
        
        if (m_pOuterObject->m_stateChangeScript != "") {
            pUEvent pEvent = reinterpret_cast<pUEvent>(Tcl_Alloc(sizeof(UEvent)));
            
            pEvent->u_fullEvent.s_pObject = m_pOuterObject;
            pEvent->u_fullEvent.s_pRegistry = m_pRegistry;
            pEvent->u_fullEvent.s_newState = Tcl_Alloc(newState.size() +1);
            strcpy(pEvent->u_fullEvent.s_newState, newState.c_str());
            
            
            pEvent->u_baseEvent.proc =
                CTCLStateClientInstanceCommand::stateChangeHandler;
            
            
            Tcl_ThreadQueueEvent(m_parent, &(pEvent->u_baseEvent),TCL_QUEUE_TAIL);
            Tcl_ThreadAlert(m_parent);
        }
    }
}

