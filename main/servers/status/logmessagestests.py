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
# @file   logmessagetests.py
# @brief  Test the python wrapper for the CStatusDefinitions::LogMessage class.
# @author <fox@nscl.msu.edu>


import unittest
from nscldaq.status import statusmessages
import zmq
import struct
import socket
import string

port = 29000


class TestRingStatistics(unittest.TestCase):
    def setUp(self):
        global port
        self._ctx = zmq.Context(1)
        self._uri = 'tcp://localhost:%d' % port
        self._receiver = self._ctx.socket(zmq.PULL)
        self._receiver.bind('tcp://*:%d' % port) 
        statusmessages.enableTest()
    def tearDown(self):
        global port
        statusmessages.disableTest()
        self._receiver.close()
        self._ctx.destroy()
        port = port - 1                # In case the server socket lingers.
    
    #-------------------------------------------------------------------------
    #  The tests:
    
    def test_construct(self):
        objv = statusmessages.LogMessage(self._uri, 'MyApplication')
        


if __name__ == '__main__':
    unittest.main()