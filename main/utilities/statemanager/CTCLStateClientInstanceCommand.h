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
# @file   CTCLStateClientInstanceCommand.h
# @brief  Instance of a state client command ensemble.
# @author <fox@nscl.msu.edu>
*/
#ifndef CTCLSTATECLIENTINSTANCECOMMAND_H
#define CTCLSTATECLIENTINSTANCECOMMAND_H

#include "TCLObjectProcessor.h"
#include <CSynchronizedThread.h>

class CTCLInterpreter;
class CTCLObject;
class CStateClientApi;
class CTCLStateClientCommand;
class CTCLStateManagerInstanceCommand;

/**
 * @class CTCLStateClientInstanceCommand
 *
 *   Instances of this class represent command ensembles that
 *   are connected to a program's state client interface.  The command
 *   ensemble provides a bunch of sub commands for querying and setting
 *   stuff.  It also provides the ability to set up a pump of events to the
 *   interpreter's event loop that dispatch to a Tcl script in the interpreter.
 *
 */

class CTCLStateClientInstanceCommand : public CTCLObjectProcessor
{
private:
    typedef struct _Event {
        Tcl_Event                 s_event;
        char*                     s_newState;
        CTCLStateClientCommand*         s_pRegistry;
        CTCLStateClientInstanceCommand* s_pObject;
    } Event, *pEvent;
    
    // This is the legal way to avoid violating strict aliasing.
    
    typedef union _UEvent {
        Tcl_Event   u_baseEvent;
        Event       u_fullEvent;
    } UEvent, *pUEvent;
    
    class MessagePump;
private:
    CStateClientApi* m_pClient;
    MessagePump*     m_pPumpThread;
    std::string      m_stateChangeScript;
    CTCLStateManagerInstanceCommand* m_pSm;
    
    
    // Canonicals
public:
    CTCLStateClientInstanceCommand(
        CTCLInterpreter& interp, std::string name, std::string reqUri,
        std::string subUri, std::string programName,
        CTCLStateClientCommand* pCreator
    );
    virtual ~CTCLStateClientInstanceCommand();
    
    // Dispatcher operator:

public:    
    int operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    
    // handler methods for the subcommands
    
protected:
    void getState(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void setState(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void isEnabled(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void isStandalone(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void title(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void runNumber(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void recording(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void outring(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void inring(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void onStateChange(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    
    // Utility methods:
    
private:
    static int stateChangeHandler(Tcl_Event* pEvent, int flags);
    
    // Thread that serves as an event pump:
    
    class MessagePump :  public CSynchronizedThread
    {
    private:
        CTCLStateClientCommand* m_pRegistry;
        CStateClientApi*    m_pClient;
        Tcl_ThreadId        m_parent;
        bool                m_exit;
        CTCLStateClientInstanceCommand* m_pOuterObject;
    public:
        MessagePump(
            CStateClientApi* pClient, Tcl_ThreadId parent,
            CTCLStateClientInstanceCommand* outerObject,
            CTCLStateClientCommand*         pRegistry
        );

    public:
        void init();
        void operator()();
        void scheduleExit();
    };

};
#endif
