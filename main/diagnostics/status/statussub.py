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
    
    def test_decodeLogMessage(self):
        self._sub.subscribe([statusmessages.MessageTypes.LOG_MESSAGE], [])
        self._log.Log(statusmessages.SeverityLevels.WARNING, "Some sort of warning")
        parts = self._sub.receive()
        decoded = statusmessages.decode(parts)
        self.assertEqual(2, len(decoded))
        header = decoded[0]
        self.assertEqual(statusmessages.MessageTypes.LOG_MESSAGE, header['type'])
        self.assertEqual(statusmessages.SeverityLevels.WARNING, header['severity'])
        self.assertEqual('TestLogger', header['app'])
        self.assertEqual(socket.getfqdn(), header['src'])
        
        body   = decoded[1]
        self.assertEqual('Some sort of warning', body['message'])
    
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
        ringSource.addProducer( ["a","producer", "program"], 100, 1234, 10)
        ringSource.addConsumer(["a", "consumer"], 123, 4567, 666, 20)
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
        self.assertEqual(0,  producer['backlog'])
        self.assertEqual(10, producer['pid'])
        self.assertEqual(["a","producer", "program"], producer['command'])
        
        self.assertEqual(123, consumer['operations'])
        self.assertEqual(4567, consumer['bytes'])
        self.assertEqual(666, consumer['backlog'])
        self.assertEqual(20, consumer['pid'])
        self.assertEqual(False, consumer['producer'])
        self.assertEqual(["a", "consumer"], consumer['command'])
    
    def test_decode_readoutstats_minimal(self):
        self._log = None                        # Get rid of old source.
        time.sleep(1)                           # takes time in some implementations.
        
        self._readout = statusmessages.ReadoutStatistics(self._pubUri, 'readout')
        self._sub     = statusmessages.Subscription(self._subUri)  # renew connection
        self._sub.subscribe(
            [statusmessages.MessageTypes.READOUT_STATISTICS],
            []
        )
        
        # Send the minimal message:
        
        self._readout.beginRun(1234, 'This is a test title')
        rawMessage = self._sub.receive()
        
        # Now decode and check:
        
        decodedMessage = statusmessages.decode(rawMessage)
        self.assertEqual(2, len(decodedMessage))    # 2 message parts.
    
        #  Check header type, application, severity and source:
        
        header = decodedMessage[0]
        self.assertEqual(statusmessages.MessageTypes.READOUT_STATISTICS, header['type'])
        self.assertEqual(statusmessages.SeverityLevels.INFO, header['severity'])
        self.assertEqual('readout', header['app'])
        self.assertEqual(socket.getfqdn(), header['src'])
        
        #  Check run and title in runid - we're  not checking the starttime
        
        runId = decodedMessage[1]
        self.assertEqual(1234, runId['run'])
        self.assertEqual('This is a test title', runId['title'])
    
    def test_decode_readoutstats_full(self):
        # Setup and set the run id info by using test_decode_readoutstats_minimal:
        
        self.test_decode_readoutstats_minimal()
        
        #  Now we can emit statistics and receive them:
        
        self._readout.emitStatistics(100, 50, 1000)
        rawMessage = self._sub.receive()
        self.assertEqual(3, len(rawMessage))
        
        #  Assume the header and body are already correct (common code)
        #  just check the statistics:
        
        decodedMessage = statusmessages.decode(rawMessage)
        stats = decodedMessage[2]
        
        self.assertEqual(100, stats['triggers'])
        self.assertEqual(50, stats['events'])
        self.assertEqual(1000, stats['bytes'])
    
    def test_decode_statechange(self):
        self._log = None                        # Get rid of old source.
        time.sleep(1)                           # takes time in some implementations.
        
        sc = statusmessages.StateChange(self._pubUri, 'TestApp')
        self._sub     = statusmessages.Subscription(self._subUri)  # renew connection
        self._sub.subscribe(
            [statusmessages.MessageTypes.STATE_CHANGE],
            []
        )
        
        sc.logChange('Begining', 'Active')
        rawMessage = self._sub.receive()
        self.assertEqual(2, len(rawMessage))   # Header & body.
        
        decodedMessage = statusmessages.decode(rawMessage)
        header = decodedMessage[0]
        body   = decodedMessage[1]
        
        #  Check header fields:
        
        self.assertEqual(statusmessages.MessageTypes.STATE_CHANGE, header['type'])
        self.assertEqual(statusmessages.SeverityLevels.INFO, header['severity'])
        self.assertEqual('TestApp', header['app'])
        self.assertEqual(socket.getfqdn(), header['src'])
        
        # Check body fields:
        
        self.assertEqual('Begining', body['leaving'])
        self.assertEqual('Active', body['entering'])
        
        
if __name__ == '__main__':
    unittest.main()

     