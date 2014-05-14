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
# @file   projectdisplay.py
# @brief  Contains the code needed to render displays of a project on the UI
# @author <fox@nscl.msu.edu>

from PyQt4 import QtGui

##
# @class ProjectDisplay
#
#   This is a megawidget that is meant to be set as the CentralWidget of the
#   main window of the editor.  This would be a tabbed notebook with pages
#   for each of the views into the project.
#

###  Stub for now.

class ProjectDisplay(QtGui.QFrame):
    ##
    # __init__
    #
    # @param parent - parent widget which is assumed to be a MainWindow
    #
    def __init__(self, parent):
        super(ProjectDisplay, self).__init__(parent)
        parent.setCentralWidget(self)
        
    ##
    # clear
    #    Clear the project (called after  a new project has been created)
    #
    def clear(self):
        print("ProjectDisplay.clear")
        pass
    
    ##
    # populate
    #   Populate the UI if a project has just been opened.
    #
    def populate(self):
        print("ProjectDisplay.populate")
        self.clear()
        pass               # More to come.
