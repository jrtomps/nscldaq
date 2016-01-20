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
# @file   StateClientTests.py
# @brief  Tests for the statemachine data type bindings
# @author <fox@nscl.msu.edu>

import unittest
import tempfile
import os
import signal
import subprocess
import time


import nscldaq.vardb.vardb
import nscldaq.vardb.varmgr

import nscldaq.vardb.stateclient
import testBase

class StateClientTests(testBase.TestBase):
    
    def dbUri(self):
        uri = 'file://' + self._dbName
        return uri
    
    def stockDatabase(self):
        nscldaq.vardb.vardb.create(self._dbName)
        api = nscldaq.vardb.varmgr.Api(self.dbUri())
        
        # Run state:
        
        
        api.defineStateMachine(
            'RunStateMachine',
            {
                '0Initial' : ('NotReady',),
                'NotReady' : ('Readying', '0Initial'),
                'Readying' : ('Ready', 'NotReady'),
                'Ready'    : ('NotReady',)
            }
        )                # Don't really need the full state machine for tests.
        
        api.mkdir("/RunState")
        api.declare("/RunState/RunNumber", "integer")
        api.declare("/RunState/Title", "string")
        api.declare("/RunState/Recording", "bool", "false")
        api.declare('/RunState/Timeout', 'integer')
        api.declare('/RunState/ReadoutParentDir', 'string')
        api.declare('/RunState/State', 'RunStateMachine')
        
        # Make the test program:
        
        api.mkdir("/RunState/test")
        api.declare('/RunState/test/standalone', 'bool')
        api.declare('/RunState/test/enable', 'bool')
        api.declare('/RunState/test/path', 'string')
        api.declare('/RunState/test/host', 'string')
        api.declare('/RunState/test/outring', 'string')
        api.declare('/RunState/test/inring', 'string')
        api.declare('/RunState/test/State', 'RunStateMachine', '0Initial')
        
        
    def setUp(self):
        print("Setup")
        # Get a temp file name..
    
        myVarDb = tempfile.NamedTemporaryFile()
        self._dbName = myVarDb.name
        myVarDb.close()                    # Unix specfic.
        
        # Create the stuff in the database:
        
        self.stockDatabase()
        
        #  Locate the server (it's in $BINDIR/vardbServer)
        #  This is normally a shell script that runs the server
        #  passing parameters to it.
        
        bindir = os.getenv('DAQBIN')
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
        print("completed setup")
    def tearDown(self):
        print("Teardown")
        if self._pid is not None:
            print("Killing server")
            os.kill(self._pid, signal.SIGKILL)
            print("Killed")
            self.waitPortGone('vardb-request')
            print("Port gone")
        if self._stdout is not None:
            self._stdout.close()
        if os.path.isfile(self._dbName):
            os.unlink(self._dbName)
        print("Teardown complete")

    # Construction of an API object.
    
    def test_creation_ok(self):
        api = nscldaq.vardb.stateclient.Api(
            'tcp://localhost', 'tcp://localhost', 'test'
        )
    def test_creation_badprog(self):
        with self.assertRaises(nscldaq.vardb.stateclient.error):
            api = nscldaq.vardb.stateclient.Api(
                'tcp://localhost', 'tcp://localhost', 'junk'
            )
    # getstate tests
    
    def test_getstateInitial(self ):
        api = nscldaq.vardb.stateclient.Api(
            'tcp://localhost', 'tcp://localhost', 'test'
        )
        
        self.assertEqual('0Initial', api.getstate())
    
    def test_getstateChange(self):
        self._api.set('/RunState/test/State', 'NotReady')
        api = nscldaq.vardb.stateclient.Api(
            'tcp://localhost', 'tcp://localhost', 'test'
        )
        self.assertEqual('NotReady', api.getstate())
        
    def test_setstate_needarg(self):
        api = nscldaq.vardb.stateclient.Api(
            'tcp://localhost', 'tcp://localhost', 'test'
        )
        with self.assertRaises(nscldaq.vardb.stateclient.error):
            api.setstate()
            
    def test_setstate(self):
        api = nscldaq.vardb.stateclient.Api(
            'tcp://localhost', 'tcp://localhost', 'test'
        )
        # Ready the program:
        
        api.setstate('NotReady')
        api.setstate('Readying')
        api.setstate('Ready')
        
        time.sleep(1)         # Let messages update the state.
        
        self.assertEquals('Ready', api.getstate())
        
    def test_isenabled_yes(self):
        api = nscldaq.vardb.stateclient.Api(
            'tcp://localhost', 'tcp://localhost', 'test'
        )
        self.assertTrue(api.isenabled())
    def test_isenabled_no(self):
        api = nscldaq.vardb.stateclient.Api(
            'tcp://localhost', 'tcp://localhost', 'test'
        )
        self._api.set('/RunState/test/enable', 'false')
        self.assertFalse(api.isenabled())


    def test_isstandalone_no(self):
        self._api.set('/RunState/test/standalone', 'false')
        api = nscldaq.vardb.stateclient.Api(
            'tcp://localhost', 'tcp://localhost', 'test'
        )
        self.assertFalse(api.isstandalone())
        
    def test_isstandalone_yes(self):
        api = nscldaq.vardb.stateclient.Api(
            'tcp://localhost', 'tcp://localhost', 'test'
        )
        self._api.set('/RunState/test/standalone', 'true')
        self.assertTrue(api.isstandalone())
        
    # Note programs can only get the title not set it:
     
    def test_title_empty(self):
        api = nscldaq.vardb.stateclient.Api(
            'tcp://localhost', 'tcp://localhost', 'test'
        )
        self.assertEquals('', api.gettitle())
    
    def test_title_set(self):
        title = 'title has been set'
        self._api.set('/RunState/Title', title)
        api = nscldaq.vardb.stateclient.Api(
            'tcp://localhost', 'tcp://localhost', 'test'
        )
        self.assertEquals(title, api.gettitle())
        
    def test_runnum_initial(self):
        api = nscldaq.vardb.stateclient.Api(
            'tcp://localhost', 'tcp://localhost', 'test'
        )
        self.assertEquals(0, api.runnumber())
        
    def test_runnum_changed(self):
       self._api.set("/RunState/RunNumber", "1234") 
       api = nscldaq.vardb.stateclient.Api(
            'tcp://localhost', 'tcp://localhost', 'test'
       )
       self.assertEquals(1234, api.runnumber())
       
    def test_recording_initial(self):
        api = nscldaq.vardb.stateclient.Api(
             'tcp://localhost', 'tcp://localhost', 'test'
        )
        self.assertFalse(api.recording())
       
    def test_recording_changed(self):
        self._api.set('/RunState/Recording', 'true')
        api = nscldaq.vardb.stateclient.Api(
            'tcp://localhost', 'tcp://localhost', 'test'
        )
        self.assertTrue(api.recording())
       
       
    def test_outring_initial(self):
        api = nscldaq.vardb.stateclient.Api(
            'tcp://localhost', 'tcp://localhost', 'test'
        )
        self.assertEqual('', api.outring())
        
    def test_outring_changed(self):
        self._api.set('/RunState/test/outring', 'output')
        api = nscldaq.vardb.stateclient.Api(
            'tcp://localhost', 'tcp://localhost', 'test'
        )
        self.assertEqual('output', api.outring())
    
    def test_inring_initial(self):
        api = nscldaq.vardb.stateclient.Api(
            'tcp://localhost', 'tcp://localhost', 'test'
        )
        self.assertEqual('', api.inring())    
    
    def test_intring_changed(self):
        inring = 'tcp://localhost/fox'
        self._api.set('/RunState/test/inring', inring)
        api = nscldaq.vardb.stateclient.Api(
            'tcp://localhost', 'tcp://localhost', 'test'
        )
        self.assertEqual(inring, api.inring())
        
    def test_waittransition_timeout(self):
        api = nscldaq.vardb.stateclient.Api(
            'tcp://localhost', 'tcp://localhost', 'test'
        )
        result = api.waitTransition(0)
        self.assertFalse(result['changed'])
        
    def test_waittransition_invalidtimeout(self):
        api = nscldaq.vardb.stateclient.Api(
            'tcp://localhost', 'tcp://localhost', 'test'
        )
        with self.assertRaises(nscldaq.vardb.stateclient.error):
            api.waitTransition(-2)
            
    def test_waittransition_occured(self):
        api = nscldaq.vardb.stateclient.Api(
            'tcp://localhost', 'tcp://localhost', 'test'
        )
        self._api.set('/RunState/test/standalone', 'false')
        self._api.set('/RunState/State', 'NotReady')
        
        result = api.waitTransition(2000)             # infinite timeout.
        self.assertTrue(result['changed'])
        self.assertEqual('NotReady', result['state'])
        
    def test_waittransition_standalone(self):
        api = nscldaq.vardb.stateclient.Api(
            'tcp://localhost', 'tcp://localhost', 'test'
        )
        self._api.set('/RunState/test/standalone', 'true')
        self._api.set('/RunState/test/State', 'NotReady')
        
        result = api.waitTransition(2000)             # infinite timeout.
        self.assertTrue(result['changed'])
        self.assertEqual('NotReady', result['state'])
        
# Run the tests if this is main:


if __name__ == '__main__':
    unittest.main(verbosity=2)