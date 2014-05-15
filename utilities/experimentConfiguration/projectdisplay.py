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

from PyQt4 import QtGui, QtCore
from nscldaq.expconfiguration import state, project


##
# @class
#   HostTable
#     This class is derived from a QTableWidget and is used to
#     *  Display information about the set of hosts known by the project and
#     *  Interact with that information in a way that allows you to
#        maintain the host table.
# @note
#    For the editing functions to operate, the user must have connected to a
#    project.
#
class HostTable(QtGui.QTableWidget):
    ##
    # __init__
    #    Construct us as a derivation of a QTableWidget.
    #
    # @param parent - the parent widget.
    #
    def __init__(self, parent):
        super(HostTable, self).__init__(parent)
        self.setSortingEnabled(True)
        self.setContextMenuPolicy(QtCore.Qt.CustomContextMenu)
        self.customContextMenuRequested.connect(self._contextMenu)
        self.cellChanged.connect(self._changed)

    def _contextMenu(self, point):
        row = self.currentRow()
        idItem = self.item(row,0)

        # Can't delete the one that has no id:
        
        if str(idItem.text()) != '':   
            globalPosition = self.mapToGlobal(point)
            contextMenu = QtGui.QMenu()
            contextMenu.addAction('Delete...')
            action = contextMenu.exec_(globalPosition)
            if action and (action.text() == 'Delete...'):
                confirm = QtGui.QMessageBox.question(
                    contextMenu, 'Delete? ',
                    'Are you sure you want to delete this host?',
                    QtGui.QMessageBox.Ok, QtGui.QMessageBox.Cancel
                )
                if confirm == QtGui.QMessageBox.Ok:
                    id = int(idItem.text())
                    ##
                    # TODO: - ensure there are no references to the host
                    #
                    
                    hosts = project.Hosts(state.State.project)
                    hosts.delete(id)
                    self.populate(hosts.list())
            
    
            
    ##
    # _changed
    #   Called when a cell contents change
    #   * If the host is empty the assumption is you didn't want to do anything.
    #   * If this is the row with no id, a new item is added to the host table.
    #     (however if the host already exists, that's an error).
    #   * If this is a row with an id that row will be modified (again it's an
    #     error to duplicate a host).
    #
    def _changed(self, row, col):
        print("changed")
        idItem   = self.item(row, 0)
        hostItem = self.item(row, 1)
        
        hosts   = project.Hosts(state.State.project)
        
        id   = str(idItem.text())
        host = str(hostItem.text())
        
        if host != '':     
            if id == '':
                if hosts.exists(host):
                    
                    QtGui.QMessageBox.critical(
                        self, 'Duplicate Host',
                        '%s is already defined as a host.' % host
                    )
                else:
                    hosts.add(host)
            else:
                if hosts.exists(host):
                    QtGui.QMessageBox.critical(
                       self, 'Duplicate Host',
                       '%s is already defined as a host.' % host 
                    )
                else:
                    hosts.modify(int(id), host)                   
            self.populate(hosts.list())
        
        
    #-------------------------------------------------------------------------
    #  Public methods
    
    ##
    # populate
    #   Clear the data in the current table and populate it with new data
    #
    # @param hosts - This is a tuple/list of dicts each dict contains the keys:
    #                -   id - The id of a host.
    #                -   host_name - the name of that host.
    # @note The hosts parameter is suitable for the outpt from the
    #       project.Hosts.list method.
    #
    def populate(self, hosts):
        # population will fire change signals so we first want to disconnect that:
        
        self.cellChanged.disconnect(self._changed)
        
        # start out with an empty table and set the header titles:
        
        
        self.clear()        
       

        self.setColumnCount(2)
        self.setRowCount(1 + len(hosts))    # 1 extra for new items.
        
        self.setHorizontalHeaderLabels(['id', 'Host Name'])
        
        #  Now populate the rows and columns from the list dict.
        #  Seems like the headers count as rows:
        
        row = 0
        for host in hosts:
            id = QtGui.QTableWidgetItem(str(host['id']))
            id.setFlags(id.flags() & (~(QtCore.Qt.ItemIsEditable)))
            self.setItem(row, 0, id)
            self.setItem(row, 1, QtGui.QTableWidgetItem(host['host_name']))
            row = row+1
        emptyId = QtGui.QTableWidgetItem('')
        emptyId.setFlags(emptyId.flags() & (~(QtCore.Qt.ItemIsEditable)))
        self.setItem(row, 0, emptyId)
            
        # From now on we want to know when cells get changed:
        
        self.cellChanged.connect(self._changed)
        
    
##
# @class ProjectDisplay
#
#   This is a megawidget that is meant to be set as the CentralWidget of the
#   main window of the editor.  This would be a tabbed notebook with pages
#   for each of the views into the project.
#


class ProjectDisplay(QtGui.QFrame):
    ##
    # __init__
    #
    # @param parent - parent widget which is assumed to be a MainWindow
    #
    def __init__(self, parent):
        super(ProjectDisplay, self).__init__(parent)
        parent.setCentralWidget(self)
        self._setupUi()
    
    ##
    #  _setupUi
    #    Setup our user interface widgets (the contents of the frame).
    #
    def _setupUi(self):
        
        self._layoutManager = QtGui.QVBoxLayout()
        self.setLayout(self._layoutManager)
        
        #  Add the host table:
        
        self._hostTable = HostTable(self)
        self._layoutManager.addWidget(self._hostTable)
    
        
    
    #--------------------------------------------------------------------
    # Public methods
    ##
    # clear
    #    Clear the project (called after  a new project has been created)
    #
    def clear(self):
        self._hostTable.clear()
        pass
    
    ##
    # populate
    #   Populate the UI if a project has just been opened.
    #
    def populate(self):
        self.clear()
        
        # Populate the host table:
        
        hosts = project.Hosts(state.State.project)
        
        self._hostTable.populate(hosts.list())
        pass               # More to come.
