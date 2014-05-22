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
# @file   programspec.py
# @brief  Forms and dialogs to specify various types of programs.
# @author <fox@nscl.msu.edu>

from PyQt4 import QtGui
from nscldaq.expconfiguration import formdialog
import sys
import os

##
# @class ProgramSpec
#     Contains a prompter that defines a program.
#     A program consists of the following elements:
#     *   A file system path to the program (file chooser).
#     *   A working directory  in which the program runs (directory chooser).
#     *   The id of the host in which the program runs (combobox with host names).
#
# LAYOUT:
#    +-------------------------------------------------+
#    | Program Path: [                 ] (Browse...)   |
#    | Working dir:  [                 ] (Browse...)   |
#    | Host:         [Host combobox    ]               |
#    +-------------------------------------------------+
#
# METHODS:
#   *   setPath   - Set the value of the path.
#   *   path      - Get the path value.
#   *   setWd     - Set the value of the working directory.
#   *   wd        - Get the value of the working directory.
#   *   setHosts  - Set the legal hosts.
#   *   setHost   - Set the value of the host box.
#   *   host      - Get the host value.
#
class ProgramSpec(QtGui.QFrame):
    
        def __init__(self, parent = None):
            super(ProgramSpec, self).__init__(parent)
            
            #  Use a vertical box layout for the form:
            
            layout = QtGui.QGridLayout()
            self.setLayout(layout)
            
            #  Instantiate and layout the widgets
            
            layout.addWidget(QtGui.QLabel('* Program Path:', self), 0, 0)
            self._path = QtGui.QLineEdit(self)
            self._path.setFixedWidth(300)
            layout.addWidget(self._path, 0, 1)
            self._pathBrowse = QtGui.QPushButton('Browse...')
            layout.addWidget(self._pathBrowse, 0, 2)
            
            layout.addWidget(QtGui.QLabel('* Working dir:', self), 1, 0)
            self._wd = QtGui.QLineEdit(self)
            self._wd.setFixedWidth(300)
            layout.addWidget(self._wd, 1, 1)
            self._wdBrowse = QtGui.QPushButton('Browse...')
            layout.addWidget(self._wdBrowse, 1, 2)
            
            layout.addWidget(QtGui.QLabel('Host: ', self), 2, 0)
            self._host = QtGui.QComboBox(self)
            layout.addWidget(self._host, 2,1)
            
            #  Connect widget signals we want to react to (browse buttons).
            
            self._pathBrowse.clicked.connect(self._browseProgramPath)
            self._wdBrowse.clicked.connect(self._browseWd)
            
        ##
        # _browseProgramPath
        #   Respond to a click of the path's browse button by getting a filename
        #   fromt he user via a filesystem browser and setting that filename in the
        #   path widget.'
        #
        def _browseProgramPath(self, checked):
            fname = QtGui.QFileDialog.getOpenFileName(
                self, 'OpenFile', os.getcwd()
            )
            if fname.size() > 0:
                self._path.setText(fname)
                
        def _browseWd(self, checked):
            dirname = QtGui.QFileDialog.getExistingDirectory(
                self, 'Choose Wd', os.getcwd()
            )
            if dirname.size() > 0:
                self._wd.setText(dirname)

            
        #----------------------------------------------------------------------
        # Public methods:
        
        ##
        # setPath
        #    Set the value of the path name QLineEdit
        #
        # @param path - new path value.
        #
        def setPath(self, path):
            self._path.setText(path)
        ##
        # path
        #    Return the current path value.
        # 
        def path(self):
            self._path.text()
            
        ##
        # setWd
        #   Set the value of the working directory line editor.
        #
        # @param path  the new path.
        def setWd(self, path):
            self.wd.setText(path)
            
        def wd(self):
            pass
        def setHosts(self, hostList):
            pass
        def setHost(self, host):
            pass
        def host(self):
            pass
            
##
# main is provided for testing:
#

if __name__ == '__main__':
    app = QtGui.QApplication(sys.argv)
    w   = ProgramSpec()
    w.show()
    
    sys.exit(app.exec_())