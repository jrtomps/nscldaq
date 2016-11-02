##
# @file   statussub.py
# @brief  Test python encapsulation of CStatusSubscription
# @author <fox@nscl.msu.edu>



import unittest
from nscldaq.status import statusmessages
import time
import struct
import string

port = 29000

class TestStatusSubscription(unittest.TestCase):
    def setUp(self):
        global port
        self._pubUri = 'tcp://*:%d' % port
        self._subUri  = 'tcp://localhost:%d' %port
        statusmessages.enableTest()
        self._log     = statusmessages.LogMessage(self._pubUri, 'TestLogger')
        self._sub     = statusmessages.Subscription(self._subUri)
    def tearDown(self):
        global port
        self._sub = None
        self._log = None
        statusmessages.disableTest()
        port = port - 1
    
    #  This just tests setup/teardown.
    
    def test_empty(self):
        pass

    def test_suball(self):
        self._sub.subscribe([])
        
        self._log.Log(statusmessages.SeverityLevels.INFO, 'A test')
        parts = self._sub.receive()
        self.assertEqual(2, len(parts))
    
    # Only should get messages for the correct types:
    
    def test_subType(self):
        self._sub.subscribe([statusmessages.MessageTypes.STATE_CHANGE])
        self._log.Log(statusmessages.SeverityLevels.INFO, 'A test') #Should not receive.
        self._sub.subscribe([statusmessages.MessageTypes.LOG_MESSAGE])
        time.sleep(0.1)                       # Wait for sub to propagate.
        self._log.Log(statusmessages.SeverityLevels.DEBUG, 'Another test') # Should get.
        
        parts = self._sub.receive()
        hdr   = parts[0]
        header = struct.unpack('ii32s128s', hdr)
        self.assertEquals(statusmessages.SeverityLevels.DEBUG, header[1])
        
    def test_subSeverity(self):
        self._sub.subscribe(
            [statusmessages.MessageTypes.LOG_MESSAGE],
            [statusmessages.SeverityLevels.INFO]
        )

        self._log.Log(statusmessages.SeverityLevels.DEBUG, "Won't receive this")
        self._log.Log(statusmessages.SeverityLevels.INFO, "Should receive this")
        
        parts = self._sub.receive()
        hdr   = parts[0]
        header = struct.unpack('ii32s128s', hdr)
        self.assertEquals(statusmessages.SeverityLevels.INFO, header[1])
    
    def test_subAppName(self):
        self._sub.subscribe(
            [statusmessages.MessageTypes.LOG_MESSAGE], [], 'WrongApp'
        )
        self._log.Log(statusmessages.SeverityLevels.DEBUG, "Won't receive this")
        self._sub.subscribe(
            [statusmessages.MessageTypes.LOG_MESSAGE], [], 'TestLogger'
        )
        time.sleep(0.1)
        self._log.Log(statusmessages.SeverityLevels.INFO, 'Should receive this')
        
        parts = self._sub.receive()
        hdr   = parts[0]
        header = struct.unpack('ii32s128s', hdr)
        
        app = string.rstrip(header[2], " \0")
        self.assertEquals('TestLogger', app)
        self.assertEquals(statusmessages.SeverityLevels.INFO, header[1])
        
        
if __name__ == '__main__':
    unittest.main()

     