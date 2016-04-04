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
from PyQt4 import QtGui, QtCore

##
# @class QNSCLBootShutdown
#
#  Provides a stacked widget that holds a pair of buttons; 'Boot' and 'Shutdown'
#  'Boot' Boots is intended to boot up the state sensitive programs. It should
#   be active when the state is NotReady.  Shutdown is intended to shutdown
#   state sensitive programs.  It should be active when the state is anything
#   but 'NotReady' or '0Initial'.  Neither button is visible in 0Initial.
#
class QNSCLBootShutdown(QtGui.QStackedWidget):
    boot     = QtCore.pyqtSignal()
    shutdown = QtCore.pyqtSignal()
    ##
    # __init__
    #   Construct.  We setup a stacked widget with the two buttons
    #   described above.  We then handle visibility in accordance with the
    #   state passed in to us.
    #
    # @param state - The state of the system (defaults to 0Initial).
    # @param parent - Our parent defaults to None
    def __init__(self, state='0Initial', parent=None):
        super(QNSCLBootShutdown, self).__init__(parent)
        self._bootButton     =QtGui.QPushButton('Boot', self)
        self._shutdownButton = QtGui.QPushButton('Shutdown', self)
        
        self.addWidget(self._bootButton)
        self.addWidget(self._shutdownButton)
        
        #  Connect the signals to our relays:
        
        self._bootButton.clicked.connect(self._bootRelay)
        self._shutdownButton.clicked.connect(self._shutdownRelay)
        
        #  Select the proper thing to display:
        
        self.setState(state)
        
    ##
    # setState
    #   Set the proper widget state:
    #
    # @param state - Current state:
    #                * 0Initial hide self.
    #                * NotReady - show self, select boot button.
    #                * Other - show self, select Shutdown button.
    #
    def setState(self, state):
        if state == '0Initial':
            self.hide()
        elif state == 'NotReady':
            self.show()
            self.setCurrentIndex(0)
        else:
            self.show()
            self.setCurrentIndex(1)
    
    #-------------------------------------------------------------------------
    # Private methods.
    
    ##
    # _bootRelay
    #   relays the boot button's onclick signal to our boot signal:
    #
    def _bootRelay(self):
        self.boot.emit()
        
    ##
    # _shutdownRelay
    #    rlays the shutdown button's onclick signal to our shutdown signal:
    #
    def _shutdownRelay(self):
        self.shutdown.emit()
        
        

##
# @class QNSCLRunControl
#   Provides a set of buttons that can control the run, and a unified signal
#   stateChange  that is emitted when a button is pushed.  The state is also
#   displayed.
#   Each button has a set of states in which it is displayed.  These are maintained
#   in a dict indexed by the button's widget.
#
class QNSCLRunControl(QtGui.QFrame):
    stateChange = QtCore.pyqtSignal([str])

    
    ##
    # __init__
    #   Construct the class:
    #   Centered above all buttons will be a label with the current states.
    #   Below that on the left is a Begin button and in the same position  an End
    #   button (but only one is visible at a time).
    #   Over to the right of the Begin/End button pair are overlaid Pause/Resume
    #   buttons of which only at most one is visible at a time.
    #
    def __init__(self, state='0Initial', parent=0):
        super(QNSCLRunControl, self).__init__(parent)
        
        self._state = QtGui.QLabel(state, self)
        self._prebegin = QtGui.QPushButton("PreBegin", self)
        self._begin = QtGui.QPushButton("Begin", self)
        self._end   = QtGui.QPushButton("End", self)
        self._pause = QtGui.QPushButton("Pause", self)
        self._resume= QtGui.QPushButton("Resume", self)
        
        #  Use a grid layout:
        
        layout =  QtGui.QGridLayout(self)
        layout.addWidget(self._state, 0, 1)
        
        layout.addWidget(self._prebegin, 1, 0)
        layout.addWidget(self._begin, 1, 0)
        layout.addWidget(self._end,   1, 0)      # Intentionally overlaid.
        
        layout.addWidget(self._pause, 1, 3)
        layout.addWidget(self._resume, 1, 3)     # Intentionally overlaid.
        
        self.setLayout(layout)
        
        #   Hook the buttons to their simple button handlers.
        #   They will translate the button into the appropriate stateChange sig.
        
        self._prebegin.clicked.connect(self._preBegin)
        self._begin.clicked.connect(self._beginRun)
        self._end.clicked.connect(self._endRun)
        self._pause.clicked.connect(self._pauseRun)
        self._resume.clicked.connect(self._resumeRun)
        
        
        #  Define the set of states for which each button is visible:
        #  This is a dict indexed by button whose values are the set of
        #  states for which that button is visible on the UI.
        
        self._visibility = dict()
        self._visibility[self._prebegin] = set(['Ready'])
        self._visibility[self._begin] = set(['Beginning'])
        self._visibility[self._end]   = set(['Active', 'Paused'])
        self._visibility[self._pause] = set(['Active'])
        self._visibility[self._resume] = set(['Paused'])
        
        # Make the appropriate buttons visible/invisible.
        
        self._currentState = state
        self.setState(state)
        
    # Public methods:
    
    ##
    #  Set the button visibilty based on the requesed state and the
    #  visibility dict.
    #
    #  @param state - The state of the system.
    #
    def setState(self, state):
        for button in [self._prebegin, self._begin, self._pause, self._resume, self._end]:
            if state in self._visibility[button]:
                button.show()
            else:
                button.hide()
                
        self._state.setText(state)
        self._currentState = state
    
    #   Private methods:

    ## _preBegin
    #   Handle the prebegin button by emitting a state change to beginning
    #
    def _preBegin(self):
        self.stateChange.emit('Beginning')
        
    ## _beginRun
    #   Handle the Begin button click. If the state is Ready, emit begining
    #   otherwise emit active
    #
    #
    def _beginRun(self):
        if self._currentState == 'Ready':
            self.stateChange.emit('Beginning')
        else:
            self.stateChange.emit('Active')
            
    ##
    # _endRun
    #    Handle the End button click by emitting a state change to
    #    'Ending'
    #
    def _endRun(self):
        self.stateChange.emit('Ending')
    ##
    #  _pauseRun
    #     Handle the Pause button click by emitting a state change to
    #    'Pausing'
    #
    def _pauseRun(self):
        self.stateChange.emit('Pausing')
    ##
    # _resumeRun
    #    Handle a click of the Resume button by emitting a state change to
    #    'Resuming'
    #
    def _resumeRun(self):
        self.stateChange.emit('Resuming')

        
        
##
# @class QNSCLRecording
#   Recording state.  This is a stacked widget with a checkbox when active
#   and a label when not.  The label will be either 'Recording' or 'Not Recording'
#   depending on the state.  The checkbox is checked or not depending on the state.
#
#   valueChanged is signaled when the state of the recording checkbox has changed
#   in the user interface.
#
class QNSCLRecording(QtGui.QStackedWidget):
    valueChanged = QtCore.pyqtSignal([bool])
    ##
    # __init__
    #  Construction.
    #
    # @param state - initial state of the widget.
    #
    def __init__(self, state=False, parent=0):
        super(QNSCLRecording, self).__init__(parent)
        self._cb = QtGui.QCheckBox('Recording', parent)
        self._label=QtGui.QLabel('Recording', parent)
        
        self.addWidget(self._cb)
        self.addWidget(self._label)
        
        self.setValue(state)
        self.setCurrentIndex(0)
        
        self._cb.stateChanged.connect(self._stateChanged)
    
    ##
    # enable
    #   Show the checkbox.
    #
    def enable(self):
        self.setCurrentIndex(0)
    
    ##
    # disable
    #   Show the checkbox.
    #
    def disable(self):
        self.setCurrentIndex(1)
        
    ##
    # value
    #   return bool value of checkbutton:
    #
    # @return bool.
    #
    def value(self):
        v = self._cb.checkState()
        if v == Qt.Core.Checked:
            return True
        else:
            return False
    
    
    
    ##
    # setValue
    #    Set a new value to the checkbox.
    #
    # @param value - New state (bool).
    def setValue(self, value):
        if value:
            state = QtCore.Qt.Checked
            text  = 'Recording'
        else:
            state = QtCore.Qt.Unchecked
            text  = 'Not Recording'
        
        self._cb.setCheckState(state)
        self._label.setText(text)
    
    ##
    # _stateChanged
    #   Slot that reacts to the state changing in the check button.
    #   The checkbox and  the label get set.
    #
    # @param n - non zero for checked. zero otherwizse.
    def _stateChanged(self, n):
        self.setValue(n != 0)
        self.valueChanged.emit(n != 0)
    

##
# @class QNSCLRunNumber
#
#    Run number.  This is actually a frame containing a stacked widget that has
#    a spinbox and a label which can be selected.  When the widget is 'enabled'
#    the spinbox shows.  When the widget is 'disabled' the label shows.
#    Signal/Slots are used to ensure that the label and spinbox always have
#    identical values.  In addition, the valueChanged signal is emitted here
#    whenever the value of the spinbox changes.
#    The spinbox limits are set to [0, maxint]
class QNSCLRunNumber(QtGui.QFrame):
    valueChanged = QtCore.pyqtSignal([int])
    
    ##
    # Create the widget:
    #
    #  @param n  - Initial value.
    #  @param parent - Widget parent.
    #
    def __init__(self, n = 0, parent=0):
        super(QNSCLRunNumber, self).__init__(parent)
        

        
        # Create the widgets
        
        self._sel = QtGui.QStackedWidget(self)
        self._run = QtGui.QSpinBox(parent)
        self._label = QtGui.QLabel(parent)
        self._fixed = QtGui.QLabel('Run: ', parent)
        
        # Stock the stacked widget:
        
        self._sel.addWidget(self._run)
        self._sel.addWidget(self._label)
        self._sel.setCurrentIndex(0)
        
        # Layout using a horizontal box layout:
        
        layout = QtGui.QHBoxLayout()
        layout.addWidget(self._fixed)
        layout.addWidget(self._sel)
        self.setLayout(layout)
        
        # Set spinbox attributes:
        
        self._run.setMinimum(0)
        self._run.setMaximum(9999999)
        self.setValue(n)
        
        self.setMaximumHeight(40)

        
        self._run.valueChanged.connect(self._update)
    
    ##
    # enable
    #   Enable editing (display the spinbox).
    #
    def enable(self):
        self._sel.setCurrentIndex(0)
    
    ##
    # disable
    #   Disable editing (display the label).
    #
    def disable(self ):
        self._sel.setCurrentIndex(1)
    
    ##
    #  value
    #    Return the current value (from the spinbox).
    #
    def value(self):
        return self._run.value()
    ##
    # setValue
    #  Set the value of the spinbox and the label
    #
    def setValue(self, n):
        self._run.setValue(n)
        self._label.setText('%d' % n)
    
    
    
    def _update(self, n):
        self._label.setText('%d' % self._run.value())
        self.valueChanged.emit(n)
        
##
# @class QNSCLTitle
#
#   New title entry.  This is actually a frame containing a stacked widget containing
#   a QLabel and a QLineEntry.  Disabling the widget displays the
#   label, while enabling it displays the line entry.
#
#  * The textChanged signal from the QLineEntry is exposed as our signal
#    so external actors can know the value of the line entry changed.
#  * External actors can also set the text which sets the text for both the
#    line entry and the label.
#
#
class QNSCLTitle(QtGui.QFrame):
    textChanged = QtCore.pyqtSignal([QtCore.QString])
    ##
    #  Create the widget.
    #  @param intialText -text to put in both widgets.
    #  @param parent     -parent of the stacked widget.
    #
    def __init__(self, initialText='', parent=0):
        super(QNSCLTitle, self).__init__(parent)
        self._sel   = QtGui.QStackedWidget(self)
        self._entry = QtGui.QLineEdit(initialText, self._sel)
        self._label = QtGui.QLabel(initialText, self._sel)
        self._fixed = QtGui.QLabel('Title: ', self)
        
        # Add  entry,label to the selector:
        
        self._sel.addWidget(self._entry)
        self._sel.addWidget(self._label)
        self._sel.setCurrentIndex(0);               # Initially enabled:
        
        
        # Layout the fixed text and stacked widget:
        
        layout = QtGui.QHBoxLayout()
        layout.addWidget(self._fixed)
        layout.addWidget(self._sel)
        self.setLayout(layout)
        
        # Connect the textChanged signal of the line editor to update:
        
        self._entry.textEdited.connect(self._update)
        #
        #  Limit the height to about one line worth
        #
        self.setMaximumHeight(40)
        self._entry.setFixedWidth(200)

    
    ##
    #
    #  enable
    #    Select the entry in the stacked widget:
    #
    def enable(self):
        self._sel.setCurrentIndex(0)
    ##
    #  disable
    #    Set the label entry in the stacked widget:
    #
    def disable(self):
        self._sel.setCurrentIndex(1)
    
    ##
    # setText
    #   Set the text in the label and the entry.  I'm not sure the textChanged
    #   signal gets or should get emitted.
    #
    # @param text - new text value.
    # 
    def setText(self, text):
        self._entry.setText(text)
        self._label.setText(text)
    ##
    # text
    #   Return the text.  The entry and label are both supposed to have the
    #   same values so we're just going to return the label's text.
    #
    # @return str - The contents of the label as a string (not QString)
    def text(self):
        return str(self._label.text())
    
    ##
    # _update
    #    Update the QLabel string from the QLineEdit and
    #    resignal the our textChanged signal.
    #
    def _update(self, newString):
        self._label.setText(newString)
        self.textChanged.emit(newString)
        
        
        

##
# @class QNSCLStateControllerWidget
#
#    This widget provides a controller for the global state manager
#    The user interface allows you to manipulate most of the variables
#    in the run state database:
#    *  Recording - instructs loggers whether or not to record a run.
#    *  RunNumber - Run number to associated with the next started run.
#    *  State     - The run state (this is how runs start).
#    *  Timeout   - Number of seconds to wait for timeouts.
#    *  Title     - Title that can be associated with a run.
#
#  The user interface looks something like:
#
#   +------------------------------------------------------+
#   | [ title block        ]  [ Run number]   [ ] recording|
#   |    < run control buttons  >                          |
#   |    <boot/kill>                                       |
#   |  [ timeout]                                          |
#   +------------------------------------------------------+
#
#  A bit about the elements of this UI:
#   *  The title, run number and recording controls are actually in stacked
#       widget containers.  When the run is not halted, the title becomes a
#       label rather than an entry, as does the run number and the
#       Recording checkbutton becomes a label that either says 'Recording'
#       or 'Not Recording'.
#   *  The Run control buttons visible depend on the actual run state.
#      For state prior to Ready, no buttons are visible.  For
#      Ready, a Begin button is shown.  For Active an End and Pause button
#      are shown.  For Paused, an End and Resume button are shown.
#      for all states that don't show a button the state name is shown.
#
#  Note, this app cleanly separates the GUI from the application.
#  There is no knowledge hear about how state transitions work etc.
#
class QNSCLStateControllerWidget(QtGui.QFrame):
    stateChanged    = QtCore.pyqtSignal([str])
    titleChanged    = QtCore.pyqtSignal([str])
    runNumberChanged= QtCore.pyqtSignal([int])
    recordingChanged= QtCore.pyqtSignal([bool])
    boot            = QtCore.pyqtSignal()
    shutdown        = QtCore.pyqtSignal()
    
    ##
    # __init__
    #   Constructs the UI.
    #
    #
    
    def __init__(self, parent=None):
        super(QNSCLStateControllerWidget, self).__init__(parent)
       
        # Set up the UI
        
        QNSCLStateControllerWidget._setupUi(self)    # Probably overidden by actual.
     
     
    #--------------------------------------------------------------------------
    #   Widget attribute management:
    #
    
    ##
    #  setState
    #    Communicate a state change to the UI
    #
    def setState(self, newState):
        #  The buttons know how to handle themselves:
        
        self._runcontrol.setState(newState)
        self._bootblock.setState(newState)
        
        #  The other boxes need to be told explicitly that they are enabled
        #  or disabled
        
        if newState in ['Active', 'Paused', 'Beginning', 'Ending', 'Pausing', 'Resuming']:
            self._titleBlock.disable()
            self._runBlock.disable()
            self._recording.disable()
        else:
            self._titleBlock.enable()
            self._runBlock.enable()
            self._recording.enable()
        
    ##
    # setRun
    #    Communicate an external change of the run number to the UI
    #
    # @param run - new run number.
    #
    def setRun(self, run):
        self._runBlock.setValue(run)
        
    ##
    # run
    #  Get the run number from the title block
    #
    # @return [int] run number in the title block.  This may or may  not be the
    #               same as the actual run number in RunState
    #
    def run(self):
        return self._runBlock.value()
    
    ##
    # setTitle
    #     Communicate an external title change to the UI.
    #
    #  @param newTitle
    #
    def setTitle(self, newTitle):
        self._titleBlock.setText(newTitle)
    ##
    # title
    #    Return the title from the title block.
    #
    # @return [str] - the title in the title block.#
    #
    def title(self):
        return self._titleBlock.text()
    
    ##
    # setRecording
    #   Communicate an external change to the recording state:
    #
    # @param recording - boolean new recording state.
    #
    def setRecording(self, recording):
        self._recording.setValue(recording)

    ##
    # recording
    #   Returnt he recording state from he recording block
    #
    # @return [boolean] - the current recording state in the block.
    #
    def recording(self):
        return self._recording.value()
    
    #--------------------------------------------------------------------------
    #  Private methods:
        
    ##
    # _setupUi
    #
    #  Set up the ui elements.
    def _setupUi(self):
        
        self._titleBlock = QNSCLTitle('unknown', self)
        self._runBlock   = QNSCLRunNumber(0, self)
        self._recording  = QNSCLRecording(False, self)
        self._runcontrol = QNSCLRunControl('0Initial', self)
        self._bootblock  = QNSCLBootShutdown('0Initial', self)

        
        layout = QtGui.QGridLayout(self)
        layout.addWidget(self._titleBlock, 0, 0)
        layout.addWidget(self._runBlock, 0, 1)
        layout.addWidget(self._recording, 0, 2)
        layout.addWidget(self._runcontrol, 1, 0, 1, 3)
        layout.addWidget(self._bootblock, 2, 0)
    
        self.setLayout(layout)
        
        # Connect the componet signals to our relays:
        
        self._runcontrol.stateChange.connect(self._stateRelay)
        self._titleBlock.textChanged.connect(self._titleRelay)
        self._runBlock.valueChanged.connect(self._runChangedRelay)
        self._recording.valueChanged.connect(self._recordingRelay)
        self._bootblock.boot.connect(self._bootRelay)
        self._bootblock.shutdown.connect(self._shutdownRelay)
        
        self.show()
    
    ##
    # _stateRelay
    #    Slot for the state controller stateChange  signal.
    #    this  just relays the signal to our state change signal.
    #
    # @param state - new requested state.
    #
    def _stateRelay(self, state):
        
        self.stateChanged.emit(state)
    ##
    # _titleRelay
    #   Slot to relay the title block's textChanged signal to our
    #   titleChanged signal.
    #
    def _titleRelay(self, newTitle):
        self.titleChanged.emit(newTitle)
    ##
    # _runChangedRelay
    #   Slot for the run control block's value changed signal
    #   This is mapped to our runNumberChanged signal
    #
    def _runChangedRelay(self, runNumber):
        self.runNumberChanged.emit(runNumber)
        pass
    
    ##
    # _recordingRelay
    #    Relays the valuechanged signal of the recording block to our
    #    recordingChanged signal.
    #
    def _recordingRelay(self, newState):
        self.recordingChanged.emit(newState)
    
    ##
    # _bootRelay
    #    Relay the boot block's boot signal to ours.
    #
    def _bootRelay(self):
        self.boot.emit()
    ##
    # _shutdownRelay
    #   Relay the boot block's shutdown signal to ours.
    #
    def _shutdownRelay(self):
        self.shutdown.emit()
        
def main():

    app = QtGui.QApplication(sys.argv)
    ex  = QNSCLStateControllerWidget()
    ex.show()
    sys.exit(app.exec_())

if __name__ == '__main__':
    main()