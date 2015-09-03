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
import os
from PyQt4 import QtGui
from nscldaq.rctl  import RunControl, StateMonitorWidget
from nscldaq.vardb import statemanager

class QNSCLReadoutGui(QtGui.QTabWidget):
    ##
    # __init__
    #    Construct the application widget.
    #
    # @param api    - State APi that is needed by the state monitor widget.
    # @param requri - Request URI needed by the state monitor widget.
    # @param suburi - Subscriptino URI needed by the RunControl widget.
    # @param parent - object that will be the parent of this widget.
    #   
    def __init__(self, api, requri, suburi, parent = None):
        super(QNSCLReadoutGui, self).__init__(parent)
        self._api    = api
        self._requri = requri
        self._suburi = suburi
        self.setupUi()
        
    def setupUi(self):
        self.setWindowTitle('Readout GUI')
        self._rctl = RunControl.RunControl(self._requri, self._suburi, self)
        self.addTab(self._rctl, "Run Control")

        self._mon = StateMonitorWidget.QNSCLStateMonitor(self._api, 1.0, self)
        self.addTab(self._mon, "Program states")
        
        self.show()
        
        
def main():
    if len(sys.argv) != 3:
        print >> stderr, "Usage:"
        print >> stderr, "   ReadoutGui requri suburi"
        print >> stderr, "Where:"
        print >> stderr, "   requri   - The vardbserver request URI"
        print >> stderr, "   suburi   - The vardbserver subscription URI"
        sys.exit(os.EX_USAGE)
        
    requests      = sys.argv[1]
    subscriptions = sys.argv[2]
    api           = statemanager.Api(requests, subscriptions)
    
    app = QtGui.QApplication(sys.argv)
    ex  = QNSCLReadoutGui(api, requests, subscriptions)
    sys.exit(app.exec_())

if __name__ == '__main__':
    main()