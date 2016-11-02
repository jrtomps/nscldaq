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
# @file   CTCLSubscription.h
# @brief  Defines classes for Tcl bindings to CStatusSubscription
# @author <fox@nscl.msu.edu>
*/

#ifndef CTCLSUBSCRIPTION_H
#define CTCLSUBSCRIPTION_H

#include <TCLObjectProcessor.h>
#include <map>
#include <zmq.hpp>
#include <tcl.h>
#include <CStatusSubscription.h>

class CTCLInterpreter;
class CTCLObject;


/**
 *  @class CTCLSubscription
 *    Provides Tcl bindings to the C++ CStatusSubscription class.  See
 *    the internal NSCL link:
 *
 *    https://swdev-redmine.nscl.msu.edu/projects/sfnscldaq/wiki/Subscription_API#TclBindings
 *
 * For a description of how this all works.
 */
class CTCLSubscription : public CTCLObjectProcessor
{
private:
    class SubscriptionInstance;
    typedef std::map<std::string, SubscriptionInstance*> Registry;
private:
    Registry        m_registry;
    static unsigned m_sequence;
    static zmq::context_t& m_zmqContext;

    
public:
    CTCLSubscription(
        CTCLInterpreter& interp, const char* cmd = "statusSubscription"
    );
    virtual ~CTCLSubscription();
    
public:
    int operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
private:
    void create(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void destroy(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);

    // Nested instance class:
private:
    class SubscriptionInstance : public CTCLObjectProcessor
    {
    private:
        typedef struct _ThreadParameter {
            zmq::socket_t*        s_pSocket;
            SubscriptionInstance* s_pInstance;
            Tcl_ThreadId          s_NotifyMe;
        } ThreadParameter, *pThreadParameter;
        
        typedef struct _NotificationEvent {
            Tcl_Event             s_tclEvent;
            SubscriptionInstance* s_pInstance;
            
        } NotificationEvent, *pNotificationEvent;
    private:
        zmq::socket_t&       m_socket;
        CStatusSubscription& m_Subscription;
        std::string          m_script;
        bool                 m_dispatching;
        bool                 m_requestEndToDispatching;
        Tcl_ThreadId         m_pollThreadId;
        Tcl_Mutex           m_socketLock;
        Tcl_Condition       m_condition;
    public:
        SubscriptionInstance(
            CTCLInterpreter& interp, const char* cmd, zmq::socket_t& sock,
            CTCLObject& subdef
        );
        virtual ~SubscriptionInstance();
    public:
        int operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    private:
        void receive(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
        void onMessage(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
        void test(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
        
    private:
        Tcl_Obj* receiveMessage();
        
        void createSubscriptions(CTCLInterpreter& interp, CTCLObject& obj);
        void subscribe(CTCLInterpreter& interp, CTCLObject& sub);
        std::vector<std::string> objListToStringVector(
            CTCLInterpreter& interp, CTCLObject& obj
        );
        CStatusSubscription::RequestedTypes stringsToTypes(
            const std::vector<std::string>& strings
        );
        CStatusSubscription::RequestedSeverities stringsToSeverities(
            const std::vector<std::string>& strings
        );
        
        void startPollThread();
        void stopPollThread();
        void flushEvents();
        
        static int  eventHandler(Tcl_Event* pEvent, int flags);
        static void pollThread(ClientData pInfo);
    };
};

#endif
