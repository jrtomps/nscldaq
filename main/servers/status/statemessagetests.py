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
# @file   statusmessagetests.py
# @brief  Test python encapsulation of CStatusDefinitions::StatusMessage
# @author <fox@nscl.msu.edu>



import unittest
from nscldaq.status import statusmessages
import zmq
import struct
import socket
import string

port = 29000

class TestStatusMessage(unittest.TestCase):
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
    #  Tests:
    
    def test_construction(self):
        obj = statusmessages.StateChange(self._uri, 'MyTestApp')
        
    def test_change(self):
        app = 'MytestApp'
        prior = 'NotReady'
        current = 'Readying'
        
        obj = statusmessages.StateChange(self._uri, app)
        obj.logChange(prior, current)
        
        frames = self._receiver.recv_multipart()
        self.assertEqual(2, len(frames))     # Header and body.
        
        #  Analyze header contents:
        
        hdr = frames[0]
        header = struct.unpack('ii32s128s', hdr)
        self.assertEqual(statusmessages.MessageTypes.STATE_CHANGE , header[0])     # Type.
        self.assertEqual(statusmessages.SeverityLevels.INFO, header[1])     # Severity  == info
        self.assertEqual(app, string.rstrip(header[2], "\0"))
        self.assertEqual(socket.getfqdn(), string.rstrip(header[3], "\0"))
        
        # Analyze the state change body:
        
        sc  = frames[1]
        body = struct.unpack('q32s32s', sc)
        self.assertEqual(prior, string.rstrip(body[1], "\0"))
        self.assertEqual(current, string.rstrip(body[2], "\0"))
        
        
if __name__ == '__main__':
    unittest.main()

        