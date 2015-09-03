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
# @file   StateMonitorWidget.py
# @brief  Provide a widget that monitors the state of all state sensitive programs.
# @author <fox@nscl.msu.edu>
import sys
from PyQt4 import QtGui, QtCore
import nscldaq.vardb.statemanager


##
# @class QNSCLStateMonitor
#    This is an autonomous widget that can monitor the state of programs that
#    are registered as state sensitive.   Periodicaly the widget will
#    *  Get the list of programs.
#    *  Get the list of enabled programs.
#    *  Get the list of standalone programs.
#    *  Get the states of all programs
#
#  This information is then used to update information about all programs.
#  Each program is represented by a row in the table widget that contains (L to R):
#  *   Program name
#  *   Program individual state.
#  *   Enabled (Yes/no).
#  *   Standalone (Yes/no).
#
#
#
class QNSCLStateMonitor(QtGui.QTableWidget):
    ##
    # __init__
    #   Construct the item.
    #   @param api - An nscldaq.vardb.statemanager.Api object used to
    #                update the data in the widget.
    #   @param updateRate - Number of seconds between updates (defaults to 1).
    #                This is floating point seconds with a resolution of 1ms.
    #   @param parent - THe parent widget, defaults to None.
    #
    def __init__(self, api, updateRate = 1.0, parent = None):
        super(QNSCLStateMonitor, self).__init__(parent)
        self._api = api
        self._updateRate = updateRate

        self.setColumnCount(4)
        
        
        # Setup for periodic updates:
        
        self._timer = QtCore.QTimer(parent)
        self._timer.setInterval(int(updateRate*1000))
        self._timer.setSingleShot(False)
        self._timer.timeout.connect(self._update)
        self._timer.start()
        
        # Initial table setup:
        
        self.setMinimumWidth(410)
        self.setHorizontalHeaderItem(0, QtGui.QTableWidgetItem('Program'))
        self.setHorizontalHeaderItem(1, QtGui.QTableWidgetItem('State '))
        self.setHorizontalHeaderItem(2, QtGui.QTableWidgetItem('Enabled'))
        self.setHorizontalHeaderItem(3, QtGui.QTableWidgetItem('Standalone'))
        self._update()
    
    
    ##
    # _setCell
    #   Set the text of a cell
    #
    #  @param row - row number
    #  @param col - column number.
    #  @param text - New text value.
    #
    def _setCell(self, row, col, text):
        item = self.item(row, col)
        if item == None:
            item = QtGui.QTableWidgetItem()
            item.setFlags(QtCore.Qt.NoItemFlags)
            self.setItem(row, col, item)
        item.setText(text)
     
    ##
    # _setBooleanCell
    #
    #    Set the contents of a cell from a boolean
    #
    # @param row - row to set.
    # @param col - Column to set.
    # @param flag - Flag to set from.
    #
    def _setBooleanCell(self, row, col, flag):
        if flag:
            text = 'Yes'
        else:
            text = 'No'
        self._setCell(row, col, text)
     
    ##
    # _updateRow
    #   Update a row of the table:
    #  @param row     - row number.
    #  @param name    - program name.
    #  @param state   - Program state text
    #  @param enabled - Enable flag (boolean)
    #  @param standalone - Standalone flag (boolean).
    #
    def _updateRow(self, row, name, state, enabled, standalone):
        self._setCell(row, 0, name)
        self._setCell(row, 1, state)
        self._setBooleanCell(row, 2, enabled)
        self._setBooleanCell(row, 3, standalone)
       
    ##
    #  update
    #
    #  Update the table with the program names, states enabled and standalone
    #  flags.
    #
    def _update(self):
        
        #  note that the database _can_ be locked in which case one of these
        # operations can fail so:
        
        try :
            programs   = self._api.listPrograms()
            enabled    = self._api.listEnabledPrograms()
            standalone = self._api.listStandalonePrograms()
            
            self.setRowCount(len(programs))
            row=0
            for program in programs:
                state        = self._api.getProgramState(program)
                isEnabled    = program in enabled
                isStandalone = program in standalone
                self._updateRow(row, program, state, isEnabled, isStandalone)
                row = row+1
        except:
            print("Update failed...we'll catch it next time")
            pass                 # Catch it next upate interval
    
        
        
        
        
def main():
    reqUri = sys.argv[1]
    subUri = sys.argv[2]
    api = nscldaq.vardb.statemanager.Api(reqUri, subUri)
    app = QtGui.QApplication(sys.argv)
    ex  = QNSCLStateMonitor(api)
    ex.show()
    sys.exit(app.exec_())

if __name__ == '__main__':
    main()