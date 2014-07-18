#!/usr/bin/env python



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
# @file   StateMonitor.py
# @brief  State Monitoring class framework.
# @author <fox@nscl.msu.edu>

import zmq
from nscldaq.statemanager import Utilities



##
# @class StateMonitorBase
#
#   Simplest possible state manager base class.  Initial state publication is
#   via call to the overridable initialState method and transitions to the
#   overridable Transition method.
#
#  An initializer callout allows additional poll targets to be registered.
#
#   We also provide ways to make a request of the state manager server.
#
class StateMonitorBase(object):
    ##
    # __init__
    #    Constructor
    #  @param transitionRequest - URI for the transition request port.
    #  @param statePublisher    - URI for the state publisher port.
    #  @param initializer       - Callable to call  just before exiting the
    #                             constructor.  This is passed self.
    #                              from which other stuff can be gotten.
    #
    def __init__(self, transitionRequest, statePublisher, initializer = None):
        super(StateMonitorBase, self).__init__()
        self.zmqContext         = zmq.Context()
        #
        #  Make the transition request socket and connect it to the
        #  state manager
        #
        self._requestSocket  = self.zmqContext.socket(zmq.REQ)
        self._requestSocket.connect(transitionRequest)
        
        # Make the subscription socket and subscribe to transitions
        # and to state publications.
        
        self._stateSocket = self.zmqContext.socket(zmq.SUB)
        self._stateSocket.setsockopt(zmq.SUBSCRIBE, 'STATE:')
        self._stateSocket.setsockopt(zmq.SUBSCRIBE, 'TRANSITION:')
        self._stateSocket.connect(statePublisher)
        
        #  Make a poller and add our subscription to it.
        #
        self.poller = Utilities.ZmqFileEventLoop()
        self.poller.register(self._stateSocket, zmq.POLLIN, self._StateMessage)
        self._state = None                 # we don't know the state yet.
        
        #  Now we can call the client's initializer callback:
        
        if initializer != None:
            initializer(self)
    
    def __del__(self):
        self._requestSocket.close()
        self._stateSocket.close()
        
    #-------------------------------------------------------------------------
    # private/protected utilities
    
    ##
    # _StateMessage
    #    Called when the state manager publishes a new STATE or TRANSITION  message.
    #
    # @param  poller - The poller object that fired us off.
    # @param  object - The object that became readable/writable (the socket).
    # @param  mask   - Mask of events on the object.
    #
    def _StateMessage(self, poller, object, mask):
        msg          = object.recv()
        messageParts = msg.split(':')
        msgType      = messageParts[0]
        msgBody      = messageParts[1]
        
        # If the type was a STATE or TRANSITION request,
        # and the current state was None, update the state,
        # invoke InitialState and remove the 'STATE:' subscription.
        
        
        
        # STATE and TRANSITION types containe the new/current state as the body:
        
        if msgType == 'STATE' :
            self.initialState(msgBody)
        elif msgType == 'TRANSITION':
            self.transition(msgBody)
        else:
            raise RuntimeError('Invalid message type %s' % (msgType))
    
    #---------------------------------------------------------------------------
    #  Public entries.
    #
    
    
    ##
    # requestTransition
    #     Called to request a state transition.
    #
    # @param transition - The state transition request string (e.g. FAIL)
    #
    # @return string - the status of the request from the server.
    #
    def requestTransition(self, transition):
        self._requestSocket.send(transition)
        reply = self._requestSocket.recv()
        return reply
    
    ##
    # initialState
    #   Called to set the initial state of the system.  This is normally
    #   a callback from _StateMessage but is left public so that it can
    #   be overridden by subclasses (which still should call us):
    #   * self_state is set.
    #   * We unsubscribe from the STATE: messages.
    #
    # @param state - New value for the state.
    #
    def initialState(self, state):
        self._state = state
        self._stateSocket.setsockopt(zmq.UNSUBSCRIBE, 'STATE:')
        
    ##
    # transition
    #    Called when a state transition is detected.  Note that if our current
    #    state is None we treat this as an initialState as well
    #    actual classes override this normally but should call us.
    # @param state -- the new state.  Note that self._state contains the prior
    #                 state.
    #
    def transition(self, state):
        if self._state == 'None':
            self.initialState(state)
        else:
            self._state  = state
    
    ##
    # run
    #    Run the state machine.
    #     TODO:  We could continue to take STATE messages and use timeouts
    #            to watch for the state manager exit.
    def run(self):
        while True:
            self.poller.pollForever(10)

##
# @class StateMonitor
#    This is the class most people should use.  With it users can specify
#    callbacks for specific state transitions.  Those callbacks get
#    the knowledge of the from/to states as well as us as a parameter.
#
class StateMonitor(StateMonitorBase):
    ##
    # __init__
    #   Constructor.  initialize the base class and set up the
    #   dispatch map as empty.
    #  @param transitionRequest - URI for the transition request port.
    #  @param statePublisher    - URI for the state publisher port.
    #  @param initializer       - Callable to call  just before exiting the
    #                             constructor.  This is passed self.
    #                              from which other stuff can be gotten.
    #
    def __init__(self, transitionRequest, statePublisher, initializer = None):
        super(StateMonitor, self).__init__(
            transitionRequest, statePublisher, initializer
        )
        self._dispatch = dict()
        
    #--------------------------------------------------------------------------
    # Public methods.
    
    ##
    # register
    #   Declares a desire to know about transitions into a specific state.
    #
    # @param state    - The state we want transitions to.
    # @param callable - A callable invoked when we transition into that state.
    #                    The callable, when called is parameterized by:
    #                    *  This object.
    #                    *  The from state - if None, this is an initial
    #                       state indicating we just joined the state machine.
    #                    *  The to state.
    #                    *  An arbitrary parameter passed in (see cbarg below).
    # @param cbarg    - An parameter that is passed without interperetation to
    #                   the callable.
    #
    def register(self, state, callable, cbarg):
        self._dispatch[state.upper()] = [callable, cbarg]
    ##
    # unregister
    #   Removes the registration of a handler for a state transition.
    #
    # @param state - The target state we no longer want handled.
    #
    def unregister(self, state):
        del self._dispatch[state.upper()]
    ##
    # initialState
    #   Override of base class initial stae.
    #   In addition to all of the actions of the base class's initialState
    #   method, if there is a handler for the state it will be invoked with a
    #   prior state as None
    #
    # @param state - New state.
    #
    def initialState(self, state):
        super(StateMonitor, self).initialState(state)
        state = state.upper()
        if state in self._dispatch.keys():
            callable = self._dispatch[state][0]
            param    = self._dispatch[state][1]
            
            callable(self, None, state, param)
    ##
    # transition
    #  Overrides the base class transition method.
    #  * save the prior state.#
    #  * run the base class method.
    #  * If the prior state was not None, invoke
    #    any handler the target state has.
    #
    # @param state - new state name.
    #
    def transition(self, state):
        priorState = self._state.upper()                # In base class...
        state = state.upper()
        super(StateMonitor, self).transition(state)
        if priorState != None and state in self._dispatch.keys():
            callable = self._dispatch[state][0]
            param    = self._dispatch[state][1]
            callable(self, priorState, state, param)
            
        