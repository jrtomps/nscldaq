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
# @file   CTCLSubscription.cpp
# @brief  Implement the subscription command ensemble and the instance command class.
# @author <fox@nscl.msu.edu>
*/

#include "CTCLSubscriptions.h"
#include <TCLInterpreter.h>
#include <TCLObject.h>
#include <CStatusSubscription.h>
#include <stdexcept>
#include <system_error>
#include <Exception.h>
#include <sstream>
#include <errno.h>
#include <iostream>
#include "TclUtilities.h"

/** Static members */

unsigned        CTCLSubscription::m_sequence(0);
zmq::context_t&  CTCLSubscription::m_zmqContext(*(new zmq::context_t(1)));


/*=============================================================================
 *  Outer class implementation
*/

/**
 * constructor (outer class)
 *
 * @param interp  - The interpreter on which the command is being registered.
 * @param command - The name of the command.
 */
CTCLSubscription::CTCLSubscription(CTCLInterpreter& interp, const char* command) :
    CTCLObjectProcessor(interp, command, true)
{
    
}
/**
 * destructor (outer class)
 *    Must iterate through the registery destroying instance objects.
 *    Each instance object is responsible for shutting down any internal threads
 *    it may have active.
 */
CTCLSubscription::~CTCLSubscription()
{
    for (auto p = m_registry.begin(); p != m_registry.end(); p++) {
        delete p->second;
    }
    // The registry will take care of deleting itself.
}

/**
 * operator() (outer class).
 *   Gets control when the command executes.  We require a subcommand and
 *   dispatch to the appropriate handler for the subcommand.  We also provide
 *   an outer try/catch handler to do centralized/simplified error management.
 *
 *  @param interp - The interpreter that is executing the command.
 *  @param objv   - The command line words.
 */
int
CTCLSubscription::operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    bindAll(interp, objv);
    try {
        requireAtLeast(objv, 2, "Command ensembles require a subcommand");
        std::string subcommand = objv[1];
        
        if (subcommand == "create") {
            create(interp, objv);
        } else if (subcommand == "destroy") {
            destroy(interp, objv);
        } else {
            throw std::invalid_argument("Unrecognized subcommand keyword.");
        }
    }
    catch(std::exception& e) {
        interp.setResult(e.what());
        return TCL_ERROR;
    }
    catch(CException& e) {
        interp.setResult(e.ReasonText());
        return TCL_ERROR;
    }
    catch (std::string msg) {
        interp.setResult(msg);
        return TCL_ERROR;
    }
    catch (const char* msg) {
        interp.setResult(msg);
        return TCL_ERROR;
    }
    catch (...) {
        interp.setResult("Unanticipated exception type caught.");
        return TCL_ERROR;
    }
    
    return TCL_OK;
}
/*----------------------------------------------------------------------------
 *  Private members.
 */

/**
 * create
 *    Process the 'create' subcommand.  This subcommand creates a new instance of
 *    a subscription.  The command requires a URI which specifies the status
 *    publisher.  It also requires a word that contains the desired subscription
 *    definitions lists.  The list must be present, even if it is empty (which
 *    implies all messages are desired).
 *
 *  @param interp - interpreter executing the subcommand.
 *  @param objv   - The command words.
 */
void
CTCLSubscription::create(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    requireExactly(objv, 4, "Sub-command requires publisher URI and subscription list");
    std::string uri = objv[2];
    
    // Create the socket as a sub socket and connect it to the URI:
    
    zmq::socket_t& sock = *(new zmq::socket_t(m_zmqContext, ZMQ_SUB));
    sock.connect(uri.c_str());
    
    // Create the instance object and add it to the registry:
    
    std::stringstream instanceName;
    instanceName << "statusSubscription_" << m_sequence++;
    
    SubscriptionInstance* pInstance = new SubscriptionInstance(
        interp, instanceName.str().c_str(), sock, objv[3]
    );
    m_registry[instanceName.str()] = pInstance;
    
    // Return the registration.
    
    interp.setResult(instanceName.str());
}
/**
 * destroy
 *    Destroy an instance of a subscription.  This requires a single parameter
 *    which is the id of the instance.  The id the key in the map that holds
 *    the instance object.
 *
 *    @param interp - Interpreter that is executing the command.
 *    @param objv   - The command words.
 *  
 */
void
CTCLSubscription::destroy(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    requireExactly(objv, 3, "Sub-command requires the command to destroy");
    std::string cmd = objv[2];
    
    auto p = m_registry.find(cmd);
    if (p != m_registry.end()) {
        delete p->second;
        m_registry.erase(p);
    } else {
        throw std::invalid_argument("The command you are trying to delete is not a subscripton instance");
    }
}
/*=============================================================================
 *  Implementation of the SubscriptionInterface inner class.
 */

/**
 * construction (innerclass);
 *   @param interp - the interpreter on which the new command is being registered.
 *   @param command - name of the command.
 *   @param sock    - ZMQ socket used to communicate.
 *   @param subs    - CTCLObject that specifies the list of subscription definitions.
 */
CTCLSubscription::SubscriptionInstance::SubscriptionInstance(
    CTCLInterpreter& interp, const char* command, zmq::socket_t& sock,
    CTCLObject& obj
) :
    CTCLObjectProcessor(interp, command, true),
    m_socket(sock),
    m_Subscription(*(new CStatusSubscription(sock))),
    m_script(""),
    m_dispatching(false),
    m_requestEndToDispatching(false),
    m_socketLock(nullptr),
    m_condition(nullptr)
{
    createSubscriptions(interp, obj);        
}
/**
 * destructor
 *    - If the polling thread is active, force it to exit and wait for it.
 *    Race condition I don't know how to fix:  If an event has been queued for
 *    us that we have not yet processed, after destroying this object, there's likely
 *    to be a catastrophic problem.  We'll try to get around this by processing
 *    all of the events in the event queue here before exiting.  Not 100% sure
 *    this will fix things but we can try,  Note that the intent to destroy
 *    implies that the client is no longer interested in script level notifications.
 *  
 */

CTCLSubscription::SubscriptionInstance::~SubscriptionInstance()
{
    m_script = "";            // No longer execute scripts.
    
    stopPollThread();         // no new events but there might be some queued.
    
    flushEvents();
    
    Tcl_MutexFinalize(&m_socketLock);
    Tcl_ConditionFinalize(&m_condition);
    try {
        delete &m_Subscription;
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
    delete &m_socket;
    
}    
/**
 *  operator()
 *     Execute a subcommand.  We require that there be a subcommand, decode
 *     it and execute the appropriate command procedure.  This method also provides
 *     an outermost try/catch blocks that centralize error processing.
 *
 *  @param interp - interpreter executing the command.
 *  @param objv   - Command words.
 */
int
CTCLSubscription::SubscriptionInstance::operator()(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    bindAll(interp, objv);
    try {
        requireAtLeast(objv, 2, "Command ensembles require a subcommand");
        std::string sub = objv[1];
        if(sub == "receive") {
            receive(interp, objv);
        } else if (sub == "onMessage") {
            onMessage(interp, objv);
        } else {
            throw std::invalid_argument("Invalid subcommand");
        }
    }
    catch(std::exception& e) {
        interp.setResult(e.what());
        return TCL_ERROR;
    }
    catch(CException& e) {
        interp.setResult(e.ReasonText());
        return TCL_ERROR;
    }
    catch (std::string msg) {
        interp.setResult(msg);
        return TCL_ERROR;
    }
    catch (const char* msg) {
        interp.setResult(msg);
        return TCL_ERROR;
    }
    catch (...) {
        interp.setResult("Unanticipated exception type caught.");
        return TCL_ERROR;
    }
    
    
    return TCL_OK;
}
/**
 * receive
 *    Synchronously receive events.  Note that this implicitly ends any
 *    event notification in progress.  The event queue is flushed and any
 *    pending events are dispatched.
 *
 * @param interp   - interpreter running the command.  The result will be
 *                   a list of binary message parts.
 * @param objv     - The command words - no further command words are allowed.
 */
void
CTCLSubscription::SubscriptionInstance::receive(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    if (m_dispatching) {
        stopPollThread();
        flushEvents();
        m_script = "";
    }
    // Read all the messages into a Tcl List and set the result:
    
    interp.setResult(receiveMessage());

}
/**
 * onMessage
 *   Establish, stop or get the message handler. An optional script parameter
 *   can be supplied:
 *   - If not supplied the current dispatch script is provided.  It is an empty
 *     string if there is no active dispatcher.
 *   - If the string is empty ("") and dispatching is active it is halted.
 *   - If the string is not empty, dispatching to that script is initiated.
 *     
 *  @note If dispatching is active it will always be halted/flushed first to
 *        eliminate race conditions that may occur due to uncontrolled access
 *        to the m_script variable.
 *
 *  @param interp - interpreter that is processing this command.
 *  @param objv   - command words.
 */
void
CTCLSubscription::SubscriptionInstance::onMessage(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    requireAtMost(objv, 3, "Subcommand can take only an additional script parameter");
    
    // There are three cases represented by the if chain above... see the
    // method headers for detailed descriptions of the cases:
    
    if (objv.size() == 2) {
        // Just want the curernt script value (case 1).
        
        interp.setResult(m_script);
    } else {
        // Script changes case 2,3.
        
        std::string newScript = objv[2];
        if(m_dispatching) {
            stopPollThread();
            flushEvents();
        }
        m_script = newScript;
        if (m_script != "") {
            // We have a script worth starting stuff for (case 3).
            
            startPollThread();
        }
    }
    
}
/*----------------------------------------------------------------------------
 *  Private utilities.
 */

 /**
  * receiveMessage
  *    Receives the parts of a ZMQ message from the socket and place them as
  *    binary objects into a list of Tcl objects.  The resulting Tcl_Obj*
  *    is returned.
  */
 Tcl_Obj*
 CTCLSubscription::SubscriptionInstance::receiveMessage()
 {
    Tcl_MutexLock(&m_socketLock);
    try {
        Tcl_Obj* result = Tcl_NewListObj(0, nullptr);
        uint64_t haveMore(0);
        size_t   n;
        Tcl_Interp* pInterp = getInterpreter()->getInterpreter();
        // Get the first message segment (could block)
        
        do {
            zmq::message_t part;
            m_socket.recv(&part, 0);
            Tcl_Obj* partObj = Tcl_NewByteArrayObj(
                reinterpret_cast<const unsigned char*>(part.data()), part.size()
            );
            Tcl_ListObjAppendElement(pInterp, result, partObj);
            
            m_socket.getsockopt(ZMQ_RCVMORE, &haveMore, &n);
        } while (haveMore);
        Tcl_MutexUnlock(&m_socketLock);
        return result;
    }
    catch (...) {
        Tcl_MutexUnlock(&m_socketLock);
        throw;
    }

 }
 /**
  * createSubscriptions
  *    Decodes the list of subscripton specifications and turns them
  *    into calls to subscribe on m_Subscription.
  *     See : https://swdev-redmine.nscl.msu.edu/projects/sfnscldaq/wiki/Subscription_API#TclBindings
  *     for more information.
  *  @param interp - interpreter to use when decoding the subscription lists.
  *  @param obj    - Tcl object containing subscription descriptions.  This is
  *                  a list that contains sublists.  Each sublist describes a
  *                  subscription we desire on m_Subscription.  The sublists
  *                  contain:
  *                  - A possibly empty list of message types wanted (symoblic)
  *                  - A possibliy empty list of severities wanted (symbolic)
  *                  - An optional application name of interest.
  *                  - An option source host of interest.
  */
 void
 CTCLSubscription::SubscriptionInstance::createSubscriptions(
    CTCLInterpreter& interp, CTCLObject& obj
 )
 {
    // If length of the list is 0, this is a special case where we want
    // to subscribe to everything:
    
    if(obj.llength() == 0) {
        CStatusSubscription::RequestedTypes      allTypes;
        CStatusSubscription::RequestedSeverities allSeverities;
        m_Subscription.subscribe(allTypes, allSeverities);
    } else {
        for (int i =0; i < obj.llength(); i++) {
            CTCLObject aSubscription = obj.lindex(i);
            subscribe(interp, aSubscription);
        }
    }
 }
 /**
  * subscribe
  *    Analyzes a single subscription description list (see createSubscriptons)
  *    and creates the corresponding subscription request.
  *    
  *  @param interp - The interpreter used to analyze the subscription object.
  *  @param sub    - The subscription description.
  */
 void
 CTCLSubscription::SubscriptionInstance::subscribe(
    CTCLInterpreter& interp, CTCLObject& obj
 )
{
    // There must be at least two elements in the obj list and it cannot
    // have more than four:
    
    size_t nElements = obj.llength();
    if ((nElements < 2) || (nElements > 4)) {
        throw std::invalid_argument(
            "Subscription descriptions are lists between two and four elements long"
        );
    }
    
     // Create the requested types and severities lists:
     CTCLObject typeList = obj.lindex(0);
     CTCLObject sevList  = obj.lindex(1);
     CStatusSubscription::RequestedTypes types =
        stringsToTypes(objListToStringVector(interp, typeList));
    CStatusSubscription::RequestedSeverities sevs =
        stringsToSeverities(objListToStringVector(interp, sevList));
    
    // Application and source may be null:
    
    const char* app(nullptr);
    const char* src(nullptr);
    std::string appString;
    std::string srcString;
    
    if (nElements >2) {
        appString = std::string(obj.lindex(2));
        app = appString.c_str();
    }
    if (nElements == 4) {
        srcString = std::string(obj.lindex(3));
        src  = srcString.c_str();
    }
    
    m_Subscription.subscribe(types, sevs, app, src);
 }
 /**
  * objListToStringVector
  *    Given a TCL Object that contains a list, converts it to
  *    an std::vector<std::string>
  *
  * @param interp - interpreter used to process the list and its elements.
  * @param obj    - The list object.
  */
 std::vector<std::string>
 CTCLSubscription::SubscriptionInstance::objListToStringVector(
    CTCLInterpreter& interp, CTCLObject& obj
 )
 {
    obj.Bind(interp);
    std::vector<std::string> result;
    
    for (int i = 0; i < obj.llength(); i++) {
        result.push_back(std::string(obj.lindex(i)));    
    }
    
    return result;
 }
 /**
  * stringsToTypes
  *    Convert a vector of strings to a list of types.
  * @param strings - Input strings.
  * @return CStatusSubscription::RequestedTypes - corresponding type list.
  */
CStatusSubscription::RequestedTypes
CTCLSubscription::SubscriptionInstance::stringsToTypes(
    const std::vector<std::string>& strings
)
{
    
    // Use a lambda to build up the result via for_each:
    
    CStatusSubscription::RequestedTypes result;
    for_each(strings.begin(), strings.end(),
        [&result](const std::string& s)  {
                result.push_back(TclMessageUtilities::stringToMessageType(s.c_str()));
        }
    );
    return result;
}
/**
 * stringsToSeverities
 *    Convert a vector of strings to a list of severities
 *  @param strings
 *  @return CStatusSubscription::RequestedSeverities - Corresponding type list.
 */
CStatusSubscription::RequestedSeverities
CTCLSubscription::SubscriptionInstance::stringsToSeverities(
    const std::vector<std::string>& strings
)
{
       
    CStatusSubscription::RequestedSeverities result;
    for_each(strings.begin(), strings.end(),
        [&result](const std::string& s) {
            result.push_back(TclMessageUtilities::stringToSeverity(s.c_str()));
        }
    );
    return result;
}
/**
 * startPollThread
 *     Starts an instance of the polling thread that pumps notifications
 *     of ZMQ message receipts to scripts.
 *     -  If the polling thread is already active, we just return without
 *        starting anything new..though in theory this is a defect.
 *     -  Create and marshall a thread parameter block containing
 *        the socket, our instance and _our_ thread id so the thread knows
 *        where to post message.
 *     -  Use the Tcl thread library to start the thread:
 *        *   m_dispatching -> true
 *        *   m_requestEndToDispatching -> false
 *        *   m_pollThreadId <- Tcl_CreateThread
 *    @note we use the Tcl threading rather than C++ because we don't want to make
 *          the assumption that the Tcl library can get a meaningful thread id
 *          for a thread started in an arbitrary way (pthread, or std::thread e.g.).
 */
void
CTCLSubscription::SubscriptionInstance::startPollThread()
{
    if (!m_dispatching) {
        
        // Marshall the thread parameter block:
        
        pThreadParameter param = new ThreadParameter;
        param->s_pSocket   = &m_socket;
        param->s_pInstance = this;
        param->s_NotifyMe  = Tcl_GetCurrentThread();
        
        // Thread book keeping so that the caller knows the thread is running
        // and the thread knows it's not supposed to stop:
        
        m_dispatching             = true;
        m_requestEndToDispatching = false;
        
        // Start the thread:
        
        if (Tcl_CreateThread(
                &m_pollThreadId, pollThread, param,
                TCL_THREAD_STACK_DEFAULT, TCL_THREAD_JOINABLE
            ) != TCL_OK
        ) {
            throw std::runtime_error("Unable to start a ZMQ subscription poll thread");   
        }
    }
}
/**
 * stopPollThread
 *   This method
 *  #    Requests any running poll thread exit.
 *  #    joins to the polling thread,
 *  #    m_dispatching -> false.
 *
 *  @note - Calls to this when polling is inactive are silently ignored though
 *          with the outer logic I've tried to create that should represent an
 *          error.
 *  @note - It is possible that at the time the thread actually exits, the event
 *          queue holds events posted by the thread.  Therefore, after this
 *          method exits, the user should invoke flushEvents either with
 *          the script left alone (if it's desired to process those events through
 *          the user script), or the script made empty (if the events should not
 *          be processed).
 */
void
CTCLSubscription::SubscriptionInstance::stopPollThread()
{
    int status;
    if (m_dispatching) {
        m_requestEndToDispatching= true;
        Tcl_JoinThread(m_pollThreadId, &status);     // Wait if needed for thread exit.
        m_dispatching = false;            // Dispatching is done/
    }
}
/**
 * flushEvents
 *    Flush the Tcl event queue for the calling thread.
 */
void
CTCLSubscription::SubscriptionInstance::flushEvents()
{
    while(Tcl_DoOneEvent(TCL_DONT_WAIT | TCL_ALL_EVENTS))
        ;
}
/**
 * eventHandler
 *    This static method runs in the main thread. It is invoked from the
 *    Tcl event loop when the poll thread indicates the ZMQ socket is readable.
 *    -  Read all message segments.
 *    -  invoke the script associated with message delivery - if it's not empty.
 *
 *  @param pEvent - Pointer to the event data.
 *  @param flags  - The event flags.
 *  @return 1     - Indicating the event can be removed from the queue and
 *                  deallocated.
 */
int
CTCLSubscription::SubscriptionInstance::eventHandler(Tcl_Event* pEvent, int flags)
{
    pNotificationEvent pNotification = reinterpret_cast<pNotificationEvent>(pEvent);
    SubscriptionInstance* pInstance = pNotification->s_pInstance;
    
    // Read the zmq data:
    
    Tcl_Obj*  messageList = pInstance->receiveMessage();
    Tcl_MutexLock(&(pInstance->m_socketLock));
    
    
    // What we do next depends on if there's a script:
    
    if (pInstance->m_script != "") {
        Tcl_Interp* interp = pInstance->getInterpreter()->getInterpreter();
        CTCLObject script;
        script.Bind(*pInstance->getInterpreter());
        script  = pInstance->m_script;
        
        CTCLObject msg(messageList);
        msg.Bind(*pInstance->getInterpreter());
        
        script   += msg;
        
        int status = Tcl_GlobalEvalObj(
            interp, script.getObject()
        );
        Tcl_ConditionNotify(&(pInstance->m_condition));
        Tcl_MutexUnlock(&(pInstance->m_socketLock));
        if (status != TCL_OK) {
            //  Turn off script handling.
            
            pInstance->m_script = "";
            Tcl_AddErrorInfo(interp, "Error processing status subscription script");
            Tcl_BackgroundError(interp);
        }
    }
    
    return 1;
}
/**
 * pollThread
 *    This static method runs in its very own thread.  It alternately polls
 *    the zmq socket for input and the m_requestEndToDispatching variable
 *    for thread exit.
 *  @param pInfo - really a pointer to a dynamically allocated ThreadParameter
 *                 block.
*/
void
CTCLSubscription::SubscriptionInstance::pollThread(ClientData pInfo)
{
    pThreadParameter      params       = reinterpret_cast<pThreadParameter>(pInfo);
    zmq::socket_t*        sock         = params->s_pSocket;
    SubscriptionInstance* pInstance    = params->s_pInstance;
    Tcl_ThreadId          targetThread = params->s_NotifyMe;
    
    while (!pInstance->m_requestEndToDispatching) {
        Tcl_MutexLock(&(pInstance->m_socketLock));
        zmq_pollitem_t item =
            {(void*)(*sock), -1, ZMQ_POLLIN|ZMQ_POLLERR, 0};
        if(zmq_poll(&item, 1, 1000) == 1) {
            pNotificationEvent pEvent =
                reinterpret_cast<pNotificationEvent>(
                    Tcl_Alloc(sizeof(NotificationEvent))
                );
            pEvent->s_pInstance = pInstance;
            pEvent->s_tclEvent.proc = eventHandler;
            pEvent->s_tclEvent.nextPtr = nullptr;
            Tcl_ThreadQueueEvent(
                targetThread, reinterpret_cast<Tcl_Event*>(pEvent),
                TCL_QUEUE_TAIL
            );
            Tcl_ThreadAlert(targetThread);
            Tcl_ConditionWait(
                &(pInstance->m_condition), &(pInstance->m_socketLock), nullptr
            );
        }
        Tcl_MutexUnlock(&(pInstance->m_socketLock));
    }
}