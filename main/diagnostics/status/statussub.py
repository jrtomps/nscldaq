##
# @file   statussub.py
# @brief  Test python encapsulation of CStatusSubscription
# @author <fox@nscl.msu.edu>



import unittest
from nscldaq.status import statusmessages
import time
import struct
import string
import socket

serial = 0
class TestStatusSubscription(unittest.TestCase):
    def setUp(self):
        global serial
        self._pubUri = 'inproc://subtest%d' % serial 
        self._subUri  = 'inproc://subtest%d' % serial
        statusmessages.enableTest()
        self._log     = statusmessages.LogMessage(self._pubUri, 'TestLogger')
        self._sub     = statusmessages.Subscription(self._subUri)
    def tearDown(self):
        global serial
        del self._sub 
        del self._log 
        statusmessages.disableTest()
        serial = serial + 1
        
        
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
        self._sub.subscribe([statusmessages.MessageTypes.STATE_CHANGE],
            [statusmessages.SeverityLevels.INFO])
        self._log.Log(statusmessages.SeverityLevels.INFO, 'A test') #Should not receive.
        self._sub.subscribe([statusmessages.MessageTypes.LOG_MESSAGE],
                [statusmessages.SeverityLevels.DEBUG])
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
            [statusmessages.MessageTypes.LOG_MESSAGE],
            [statusmessages.SeverityLevels.DEBUG], 'WrongApp'
        )
        
        self._log.Log(statusmessages.SeverityLevels.DEBUG, "Won't receive this")
        time.sleep(1.0)
        
        self._sub.subscribe(
            [statusmessages.MessageTypes.LOG_MESSAGE],
            [statusmessages.SeverityLevels.INFO], 'TestLogger'
        )
        
        self._log.Log(statusmessages.SeverityLevels.INFO, 'Should receive this')
        
        parts = self._sub.receive()
        hdr   = parts[0]
        header = struct.unpack('ii32s128s', hdr)
        
        app = string.rstrip(header[2], " \0")
        self.assertEquals('TestLogger', app)
        self.assertEquals(statusmessages.SeverityLevels.INFO, header[1])
    
    def test_decodeRingStats(self):
        self._log = None
        time.sleep(1)
        ringSource = statusmessages.RingStatistics(self._pubUri, 'Ringer')
        
        self._sub     = statusmessages.Subscription(self._subUri)  # renew connection
        self._sub.subscribe(
            [statusmessages.MessageTypes.RING_STATISTICS],
            []
        )
        
        
        ringSource.startMessage("aring")
        ringSource.addProducer( ["a","producer", "program"], 100, 1234)
        ringSource.addConsumer(["a", "consumer"], 123, 4567)
        ringSource.endMessage()
        
        message = self._sub.receive()
        decodedMessage = statusmessages.decode(message)
        self.assertEqual(4, len(decodedMessage))
        
        header = decodedMessage[0]
        ringid = decodedMessage[1]
        producer = decodedMessage[2]
        consumer = decodedMessage[3]
        
        self.assertEqual(statusmessages.MessageTypes.RING_STATISTICS, header['type'])
        self.assertEqual(statusmessages.SeverityLevels.INFO, header['severity'])
        self.assertEqual('Ringer', header['app'])
        self.assertEqual(socket.getfqdn(), header['src'])
        
        self.assertEqual('aring', ringid['name'])
        
        self.assertEqual(100, producer['operations'])
        self.assertEqual(1234, producer['bytes'])
        self.assertEqual(True, producer['producer'])
        self.assertEqual(["a","producer", "program"], producer['command'])
        
        self.assertEqual(123, consumer['operations'])
        self.assertEqual(4567, consumer['bytes'])
        self.assertEqual(False, consumer['producer'])
        self.assertEqual(["a", "consumer"], consumer['command'])
        
if __name__ == '__main__':
    unittest.main()

     