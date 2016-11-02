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
# @file   readoutstatisticstests.py
# @brief  Test python bindings to CReadoutDefinitions::ReadoutStatistics
# @author <fox@nscl.msu.edu>

import unittest
from nscldaq.status import statusmessages
import zmq
import struct
import socket
import string

port = 29000

class TestReadoutStatistics(unittest.TestCase):
    def setUp(self):
        global port
        self._ctx = zmq.Context(1)
        self._uri = 'tcp://localhost:%d' % port
        self._receiver = self._ctx.socket(zmq.PULL)
        self._receiver.bind('tcp://*:%d' % port) 

    def tearDown(self):
        global port
        self._receiver.close()
        self._ctx.destroy()
        port = port - 1
        
    #-------------------------------------------------------------------------
    #  The tests:
    
    def test_construction(self):
        obj = statusmessages.ReadoutStatistics(self._uri)
    def test_construction_withApp(self):
        obj = statusmessages.ReadoutStatistics(self._uri, 'MyReadoutProgram')
     
    #  test begin run method produces header and run identification:
    
    def test_beginRun(self):
        title = 'This is the title of the run.'
        obj = statusmessages.ReadoutStatistics(self._uri, 'MyReadoutProgram')
        obj.beginRun(123, title)
        
        frames = self._receiver.recv_multipart()
        
        # There should be two frames;  the header and the title:
        
        self.assertEqual(2, len(frames))
        
        # Check the header contents:
        
        hdr = frames[0]
        header = struct.unpack('ii32s128s', hdr)
        self.assertEqual(statusmessages.MessageTypes.READOUT_STATISTICS, header[0])     # Type.
        self.assertEqual(statusmessages.SeverityLevels.INFO, header[1])     # Severity  == info
        self.assertEqual('MyReadoutProgram', string.rstrip(header[2], "\0"))
        self.assertEqual(socket.getfqdn(), string.rstrip(header[3], "\0"))
    
        # check contents of the ReadoutStatRunInfo:
        
        info = struct.unpack('qI84s', frames[1])  #end of struct is aligned to 64bits.
        self.assertEqual(123, info[1])        # Ignore the tod.
        self.assertEqual(title, string.rstrip(info[2], "\0"))
        
    def test_ReadoutStatistics(self):
        title = 'This is the title of the run.'
        obj = statusmessages.ReadoutStatistics(self._uri, 'MyReadoutProgram')
        obj.beginRun(123, title)
        
        # Get the messages from this thing.
        
        self._receiver.recv_multipart()             # We already know this is ok.
        
        obj.emitStatistics(100, 200, 12345)
        msg = self._receiver.recv_multipart()      # header, run id and stats:
        self.assertEquals(3, len(msg))
        
        stats = msg[2]
        statistics = struct.unpack('qQQQQ', stats)
        self.assertEqual(100, statistics[2])      # Skip tod, and elapsed time.
        self.assertEqual(200, statistics[3])
        self.assertEqual(12345, statistics[4])
        
if __name__ == '__main__':
    unittest.main()


