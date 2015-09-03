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
# @file   <filename>
# @brief  <short description>
# @author <fox@nscl.msu.edu>
import sys
from os import path
from PyQt4 import QtGui, QtCore
import StateControllerWidget
import nscldaq.vardb.statemanager
import nscldaq.vardb.notifier

##
# @class RunControl
#    This class extends the QNSCLStateControllerWidget with the functionality
#    required to actually control the NSCLDAQ via the global state machine
#    in the variable database.
#    This separation allows the QNSCLStateControllerWidget to be a relatively
#    mechanism independent user interface element for controlling runs.
#
class RunControl(StateControllerWidget.QNSCLStateControllerWidget):
    ##
    #  __init__
    #   Construct the 'widget'.
    #   - Set up the GUI and signal connections to our slots.
    #   - setup our hooks into the NSCLDAQ.
    #
    def __init__(self,  reqUri, subUri, parent = None):
        super(RunControl, self).__init__()
        
        self._setupUi()
        self._setupStateManager(reqUri, subUri)
        self._setupSubscriptions(subUri)
    
        # Connect UI signals:
        
        self.stateChanged.connect(self._requestTransition)
        self.titleChanged.connect(self._titleChanged)
        self.runNumberChanged.connect(self._runChanged)
        self.recordingChanged.connect(self._recordingChanged)
        self.boot.connect(self._bootSystem)
        self.shutdown.connect(self._shutdownSystem)
        
        # Timer and flag used to handle runnumber change timeouts.
        #  See _runChanged comments for more.  No need to start
        #  the timer here.
        #
        
        self._runNumberTimeout = QtCore.QTimer(self);
        self._runNumberTimeout.setSingleShot(False)
        self._runNumberTimeout.setInterval(200)
        self._runNumberTimeout.timeout.connect(self._setRunNumber)
        self._runNumberChanged = False
        
    
    ##
    # _setupUi
    #    Just set the window title and let the top window of our tree be
    #    visible.
    #
    def _setupUi(self):
        
        self.setWindowTitle('NSCLMulti-Run Control')
        self.show()
    ##
    # _setupStateManager
    #
    #  Given the URIs for the state  manager:
    #   - Create a state manager.
    #   - Populate the UI with the current state.
    #
    # @param reqUri - Request URI to the vardb Server.
    # @param subUri - Subscription URI to the server.
    #
    def _setupStateManager(self, reqUri, subUri):
        self._stateManager =  nscldaq.vardb.statemanager.Api(reqUri, subUri)
        self.setState(self._stateManager.getGlobalState())
        self.setRun(self._stateManager.getRunNumber())
        self.setTitle(self._stateManager.getTitle())
        self.setRecording(self._stateManager.isRecording())
        
        
        
    ##
    # _setupSubscriptions
    #
    #   Setup a subscription on /RunControl  and a timer to process
    #   publication messages and use them to update the UI.
    #
    # @param subUri  - URI specifying the subscription port of the vardbServer.
    def _setupSubscriptions(self, subUri):
        self._subscriptions = nscldaq.vardb.notifier.Notifier(subUri)
        self._subscriptions.subscribe('/RunState/')
        
        self._timer = QtCore.QTimer(self)
        self._timer.setInterval(1 * 1000)    # timers are in milliseconds
        self._timer.setSingleShot(False)
        self._timer.timeout.connect(self._checkSubscriptions)
        self._timer.start()
        
        # Make the set of variables we care about:
        
        self._guiVars = set([
            '/RunState/Recording', '/RunState/RunNumber', '/RunState/State',
            '/RunState/Timeout', '/RunState/Title'
        ])
        
        
    def _IgnoreMessages(self, api, notDict, arg):
        pass
    ##
    # _checkSubscriptions
    #    - flush the state manager message queue.
    #    - process messages in our notifier queue that we care about.
    #
    def _checkSubscriptions(self):

        self._stateManager.processMessages(self._IgnoreMessages, None)
        
        while self._subscriptions.readable() :
            msg = self._subscriptions.read()
            
            #  We only care about assignments -- and only to the vars that
            #  affect the UI
            #
            
            if (msg['op'] == 'ASSIGN') and (msg['path'] in self._guiVars) :
                thepath  = msg['path']    # Warning path.split.path - uses module.
                value = msg['data']
                split = path.split(thepath)
                varname = path.split(thepath)[1]
    
                # Don't really have a good way to do this other than this:
                
                if varname == 'Recording':
                    self.setRecording(True if value == 'true' else False)
                elif varname == 'RunNumber':
                    self.setRun(int(value))
                elif varname == 'State':
                    self.setState(value)
                elif varname == 'Title':
                    self.setTitle(value)
                    
                # TODO:  need to add the timeout to the GUI and here.
    
    def _stateTransitionProgress(self, Api, program, state, arg):
        QtGui.QApplication.processEvents()
        
    ##
    # _requestTransition
    #   Invoked when the GUI requests a state transiton
    #
    # @param state - requested State.
    #
    def _requestTransition(self, state):
        self._stateManager.setGlobalState(str(state))
        #
        #  Wait on the transition:
        #  TODO: maybe push this into a thread?
        #  so that we can have the UI live during the transition.
        #
        result = self._stateManager.waitTransition(self._stateTransitionProgress, None)
        if not result:
            #
            #   Report failure.
            #
            self._stateManager.setGlobalState('NotReady')
            self._stateManager.waitTransition(self._stateTransitionProgress, None)
            
    ##
    # _titleChanged
    #    Process changes in the title UI
    # @param title - New title to set in the state system.
    # 
    def _titleChanged(self, title):
        title = str(title)
        self._stateManager.setTitle(title)
    ##
    # _bootSystem
    #   Set the state to readying in response to a boot click.
    #
    def _bootSystem(self):
        self._stateManager.setGlobalState('Readying');
        
    ##
    # _shutdownSystem
    #   Set the state to NotReady in response to a shutdown click.
    #
    def _shutdownSystem(self):
        self._stateManager.setGlobalState('NotReady')

    
    ##
    # _runChanged
    #   Process changes in the run number UI.
    #   This is really tricky.  Here's the problem we need to solve:
    #   If we just set the run number in the database, that will notify us.
    #   If we get another change in the UI before the notification arrives, the
    #   notification will try to undo our change in the UI which in turn will try
    #   to undo our change in the database.  We can't just use the editDone
    #   signal like we did with the title since this is a spinbox so the user
    #   expects reasonable latencies on a spin (which does not affect editDone)
    #
    #   What we are therefore going to do is just start a periodic timer
    #   (200ms is probably) sufficient, and set a 'changed' flag.  When the
    #   timer expires; If the changed flag was set, reset it.  If the changed
    #   flag was not set, kill off the timer and update the database...if the
    #   UI value is different than the database value.
    #
    # @param run - new run number to set in the state system.
    # @note  We change the run that sends us a subscription event if we then change
    #        the run to that we wind up in an event infinite looop.
    #        Therefore only change the state run number if it's different
    #        from what we've been tasked with.
    #
    def _runChanged(self, run):
        self._runChanged = True               # Flag the run number changed.
        if not self._runNumberTimeout.isActive():
            self._runNumberTimeout.start()    # Start the timer if needed.
    ##
    # _setRunNumber
    #    Called when the timer started by _runChanged expires.  see _runChanged
    #    for detailed comments.
    def _setRunNumber(self):
        if self._runChanged:
            self._runChanged = False
        else:
            self._runNumberTimeout.stop()
            requested = self._runBlock.value()
            actual    = self._stateManager.getRunNumber()
            if requested != actual:
                self._stateManager.setRunNumber(requested)
    ##
    # _recordingChanged
    #    Called when the recording UI element changed.  Just directly update
    #    the database.
    #
    # @param state - new state of recording flag.
    def _recordingChanged(self, state):
        if state != self._stateManager.isRecording():
            self._stateManager.setRecording(state)
def main():
    requests      = sys.argv[1]
    subscriptions = sys.argv[2]
    
    app = QtGui.QApplication(sys.argv)
    ex  = RunControl(requests, subscriptions)
    sys.exit(app.exec_())

if __name__ == '__main__':
    main()