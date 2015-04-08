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
# @file   pyNotifierTests.py
# @brief  Tests of the notifier module.
# @author <fox@nscl.msu.edu>



import unittest
import nscldaq.vardb.notifier
import nscldaq.vardb.varmgr
import testBase

import unittest
import os
import os.path
import subprocess
import tempfile
import signal
import time
import getpass


class NotifierTest(testBase.TestBase):
    def setUp(self):
        # Get a temp file name..
        
        myVarDb = tempfile.NamedTemporaryFile()
        self._dbName = myVarDb.name
        myVarDb.close()                    # Unix specfic.
        
        #  Locate the server (it's in $BINDIR/vardbServer)
        #  This is normally a shell script that runs the server
        #  passing parameters to it.
        
        bindir = os.getenv('BINDIR')
        self._server = os.path.join(bindir, 'vardbServer')
        #
        #  Now initialize variables that startServer might create
        #  to None
        #
        self._pid    = None
        self._stdout = None
        #
        #  Start the server unconditionally
        #
        self.startServer(['--database', self._dbName, '--create-ok', 'yes'])
        p = self.getport('vardb-request')       # Wait for server to publish services.
        self._api = nscldaq.vardb.varmgr.Api('tcp://localhost:%d' % p)
        
    def tearDown(self):
        if self._pid is not None:
            os.kill(self._pid, signal.SIGKILL)
        if self._stdout is not None:
            self._stdout.close()
        if os.path.isfile(self._dbName):
            os.unlink(self._dbName)
    #
    #  Create a notifier object (success)
    #
    def test_create(self):
        n = nscldaq.vardb.notifier.Notifier('tcp://localhost')
        
    #  Tests for subscriptions
    
    def test_subscribeok(self):
        n = nscldaq.vardb.notifier.Notifier('tcp://localhost')
        n.subscribe('/test')
        
    def test_subscribedup(self):
        n = nscldaq.vardb.notifier.Notifier('tcp://localhost')
        n.subscribe('/test')
        self.assertRaises(nscldaq.vardb.notifier.error, n.subscribe, '/test')
    
    #  Tests for unsubscribe

    def test_unsubscribeok(self):
        n = nscldaq.vardb.notifier.Notifier('tcp://localhost')
        n.subscribe('/test')
        n.unsubscribe('/test')
    
    def test_unsubscribenosuch(self):
        n = nscldaq.vardb.notifier.Notifier('tcp://localhost')
        self.assertRaises(nscldaq.vardb.notifier.error, n.unsubscribe, '/test')

    # Tests for waitmsg
    
    def test_waitWaited(self):
        n  = nscldaq.vardb.notifier.Notifier('tcp://localhost')
        n.subscribe('/test')
        self._api.mkdir('/test/testing')     # should give us  message:
        
        self.assertTrue(n.waitmsg(100))
            
    def test_wait_timeout(self):
        n  = nscldaq.vardb.notifier.Notifier('tcp://localhost')
        n.subscribe('/test')
        self._api.mkdir('/another/dir')   # should not give a message
                                          #
        self.assertFalse(n.waitmsg(100))
        
    
    # Tests for readable
    
    def test_readable(self):
        n  = nscldaq.vardb.notifier.Notifier('tcp://localhost')
        n.subscribe('/test')
        self._api.mkdir('/test/testing')     # should give us  message:
        
        if n.waitmsg(100):
            self.assertTrue(n.readable())
        else:
            self.fail('waitmsg should be true')
            
    def test_readableNot(self):
        n  = nscldaq.vardb.notifier.Notifier('tcp://localhost')
        n.subscribe('/test')
        self._api.mkdir('/another/dir')   # should not give a message
                                          #
        self.assertFalse(n.waitmsg(100))
        self.assertFalse(n.readable())
    
    # Tests for read
    
    def test_read(self):
        n  = nscldaq.vardb.notifier.Notifier('tcp://localhost')
        n.subscribe('/test')
        self._api.mkdir('/test/dir')   # should not give a message
        
        self.assertDictEqual(
            {'path': '/test', 'op': 'MKDIR', 'data': 'dir'},
            n.read()
        )
    
    
if __name__ == '__main__':
    unittest.main()