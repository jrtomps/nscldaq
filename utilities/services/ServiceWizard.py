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
# @file   ServiceWizard.py
# @brief  Wizard to set up the service data base for the standard services.
# @author <fox@nscl.msu.edu>
import sys
from PyQt4 import QtGui, QtCore
import os, os.path
import sys
from nscldaq.vardb import services

##
#  @class WizardStep
#     Base class for wizard steps.  This just defines the
#     interface required by the concrete step classes.
#            
class WizardStep(QtGui.QWidget):
    ##
    # setupUi
    #   Setup the ui and lay it out inside the parent.  The parent is the wizard
    #   widget.
    #
    # @param parent -  parent in which to layout the widget.
    # 
    def setupUi(self, parent):
        pass
    ##
    #  apply
    #    Apply the current settings to the application code.
    #
    # @param parent - the wizard.
    #
    def apply(self, parent):
        pass

##
# @class Wizard
#
#   This class represents a sequence of 'screens'  Each screen lives in a
#   frame and there are a trio of buttons below the frames.  The buttons are:
#   -  Next - at the lower right advances to the next screen in the sequence
#             or completes the sequence (exiting).
#   -  Back - At the lower left, returns to the prior screen.
#   -  Exit - Exits the wizard prior to the last screen.
#
#  Note the first screen only has Next and Exit.  The last screen
#  only shows Back and Exit.
#
#  'screens' should be derived from 'WizardStep and must override:
#   *  setupUi     - create the User interface...return the outer widget created.
#   *  apply       - Apply communicate the UI settings to the underlying
#                    application
#
class Wizard(QtGui.QWidget):
    
    def __init__(self):
        super(Wizard, self).__init__()
        self.setupUi()
        self._pageWidgets = list()
            
    
    #--------------------------------------------------------------------------
    #  Event handlers:
    #
    
    
    def _NextPage(self):
        
        # Figure out and display the next page.
        
        n  = self._pages.count()
        i  = self._pages.currentIndex()
        
        self._pageWidgets[i].apply(self)
        
        if i < n-1:
            i = i+1
        self._pages.setCurrentIndex(i)
        
        # Figure out which buttons to show:
        
        if i == n-1:
            self._next.hide()
        else:
            self._next.show()
        if i > 0:
            self._back.show()
        
    
    def _PreviousPage(self):
        
        #  Figure out/display prior page.
        
        n  = self._pages.count()
        i  = self._pages.currentIndex()
        self._pageWidgets[i].apply(self)
        
        if i > 0:
            i = i -1
        self._pages.setCurrentIndex(i)
    
        # Figure out which buttons to show:
        
        self._next.show()
        if i == 0:
            self._back.hide()
        else:
            self._back.show()
    
    def _Exit(self):
        i = self._pages.currentIndex()
        self._pageWidgets[i].apply(self)    # Before exiting, apply the data.
        QtCore.QCoreApplication.instance().quit()
        
    #-------------------------------------------------------------------------
    # Public methods
    
    ##
    # setupUi
    #    Create the generic widget layout.
    #
    def setupUi(self):
        self.setWindowTitle('Services Wizard')
        self.move(300, 300)
        
        #   We want a QStackedWidget on top and a frame for the buttons on the
        #   bottom:
        
        self._pages   = QtGui.QStackedWidget(self)
        self._pages.show()
        
        self._buttons = QtGui.QFrame(self)
        
        
        #  Create the buttons - we'll use show/hide to indicate which buttons
        #  are viewable.  We are going to initialize for page 1.
        #
        
        self._next = QtGui.QPushButton("Next>", self)
        self._next.clicked.connect(self._NextPage)
        
        self._back = QtGui.QPushButton("<Back", self)
        self._back.clicked.connect(self._PreviousPage)
        
        self._exit = QtGui.QPushButton("Exit",  self)
        self._exit.clicked.connect(self._Exit)
        
        # Layout the buttons and hide the back button:
        
        buttonLayout = QtGui.QHBoxLayout()
        buttonLayout.addWidget(self._back)
        buttonLayout.addWidget(self._exit)
        buttonLayout.addWidget(self._next)
        self._back.hide()
        self._buttons.setLayout(buttonLayout)
        
        # Layout the widgets in the top level widget.
        
        windowLayout = QtGui.QVBoxLayout()
        windowLayout.addWidget(self._pages)
        windowLayout.addWidget(self._buttons)
        self.setLayout(windowLayout)
        
        
        
    ##
    #  addPage
    #   Add a widget to the layout.  The widget is appended to the set of wizard
    #   pages that are defined.  Pages must be derived from, or implement the interface of
    #   WizardStep.
    #
    # @param widget -the widgt to add to the page set.
    def addPage(self, widget):
        self._pageWidgets.append(widget)
        inner = widget.setupUi(self._pages)
        self._pages.addWidget(inner)
        self._pages.setCurrentIndex(0)
        
    

##
# @class DAQBINProgram
#   This will prompt for where to run a program that is stored
#   in the DAQBIN directory.
#
class DAQBINProgram(WizardStep):
    
    ##
    # __init__
    #   Construct the step:
    #   - Store the API object for later.
    #   - Load information about the program as we know it now.
    #
    # @param name    - name of the program.
    # @param program - program the path we should use.
    # @param db      - API object for services.
    #
    def __init__(self, name, program, db):
        super(DAQBINProgram, self).__init__()
        self._name    = name
        self._program = program
        self._db      = db
        self._programToFullPath()
        self._loadInfo()
    
    
    ##
    # _programToFullPath
    #   Prepends  self._program with the value of the DAQBIN env variable.
    #   If that env does not exist, then we complain and exit.
    #
    def _programToFullPath(self):
        
        # Get the DAQBIN env var.
        daqbin = os.getenv('DAQBIN')
        if daqbin == None:
            print >>        \
                sys.stderr, \
                'You must run daqsetup.bash for this version of NSCLDAQ first',
            sys.exit(os.EX_CONFIG)
        
        #  Build the self._program variable with DAQBIN:
        
        self._program = os.path.join(daqbin, self._program)
        
    
    ##
    # _loadInfo
    #   Load information about the current program.
    #   -  If needed create the directory structure for the services.
    #   - If the service has not yet been defined, define it with an empty string
    #     for the host.
    #   - If the service exists but uses a different program, change the program.
    #   - So our main purpose is to get the host from the service if it exists.
    #
    def _loadInfo(self):
        #  If needed create the database.
        
        if not self._db.exists():
            self._db.create()
        #
        #  Get info on program if it exists or create it if needed:
        #
        try:
            info = self._db.listProgram(self._name)
        except:                                # Program does not exist.
            self._db.createProgram(self._name, self._program, '')
            info = [self._program, '']
        
        # If needed modify the program:
        
        if info[0] != self._program:
            self._db.setProgram(self._name, self._program)
            
        # Save the host:
        
        self._host = info[1]
        
    ##
    # setupUi
    #   We setup a frame.  The frame will have layout managed by a grid.
    #     col, row
    #   - 0,0 - The name of the program.
    #   - 1,0 - The program path for the program (parenthesized).
    #   - 0,1 - Text entry for hostname.
    #   - 1,1 - Label 'Host name runs in'.
    #
    # @param parent   - parent for the frame.
    # @return QWidget - frame containing all the widgets.
    #
    def setupUi(self, parent):
        top             = QtGui.QFrame(parent)
        
        name            = QtGui.QLabel(self._name, top)
        path            = QtGui.QLabel('(%s)' % self._program, top)
        self._hostEntry = QtGui.QLineEdit(self._host, top)
        self._hostEntry.setFixedWidth(150)
        entrylbl        = QtGui.QLabel('Host %s runs in' % self._name, parent)
        
        # Layout the widgets in the top frame.
        # Note coordinates for addItem are row, col.
        #
        layout = QtGui.QGridLayout()
        layout.addWidget(name, 0,0)
        layout.addWidget(path, 0, 1)
        layout.addWidget(self._hostEntry, 1, 0)
        layout.addWidget(entrylbl, 1,1)
        
        top.setLayout(layout)
        
        return top
    ##
    # Called when we need to update the database from our data:
    #
    # @param parent - the wizard (not using it)
    #
    def apply(self, parent):
        newHost = self._hostEntry.text()
        self._db.setHost(self._name, str(newHost))
        self._host= newHost                      # Since we could be called back.

##
# usage
#  Output program usage and exit.
#
def usage():
    print('Usage')
    print('  svcwiz dbUri')
    print('Where:')
    print('   dbUri - is the URI that specifies a database connection')
    sys.exit(os.EX_USAGE)

        
##
# getSvcAPi
#   Return an API for the services database.
#   - Ensure there's an argv[1]
#   - Use it as the URI to create an nscldaq.vardb.servicesApi
def getSvcApi():
    if len(sys.argv) != 2:
        usage()
    return services.Api(sys.argv[1])
    
        
def main():
    db  = getSvcApi()
    app = QtGui.QApplication(sys.argv)
    ex  = Wizard()
    ex.addPage(DAQBINProgram('VARDBServer', 'vardbServer', db))
    ex.addPage(DAQBINProgram('BootManager', 'bootmanager', db))
    ex.show()
    
    sys.exit(app.exec_())

if __name__ == '__main__':
    main()