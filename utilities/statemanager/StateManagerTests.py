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
# @file   StateManagerTests.py
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

import nscldaq.vardb.statemanager
import testBase

class StateManagerTests(testBase.TestBase):
    
    def dbUri(self):
        uri = 'file://' + self._dbName
        return uri
    
    def stockDatabase(self):
        nscldaq.vardb.vardb.create(self._dbName)
        api = nscldaq.vardb.varmgr.Api(self.dbUri())
        
        # Run state:
        
        api.defineEnum('boolean', ('true', 'false'))
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
        api.declare("/RunState/Recording", "boolean", "false")
        api.declare('/RunState/Timeout', 'integer')
        api.declare('/RunState/ReadoutParentDir', 'string')
        api.declare('/RunState/State', 'RunStateMachine')
        
        # Make the test program:
        
        api.mkdir("/RunState/test")
        api.declare('/RunState/test/standalone', 'boolean')
        api.declare('/RunState/test/enable', 'boolean')
        api.declare('/RunState/test/path', 'string')
        api.declare('/RunState/test/host', 'string')
        api.declare('/RunState/test/outring', 'string')
        api.declare('/RunState/test/inring', 'string')
        api.declare('/RunState/test/State', 'RunStateMachine', '0Initial')
        
        
    def setUp(self):
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

    def tearDown(self):
        if self._pid is not None:
            os.kill(self._pid, signal.SIGKILL)
            self.waitPortGone('vardb-request')
        if self._stdout is not None:
            self._stdout.close()
        if os.path.isfile(self._dbName):
            os.unlink(self._dbName)
    
    def test_creation(self):
        api = nscldaq.vardb.statemanager.Api(
            'tcp://localhost', 'tcp://localhost'
        )
    def test_getprogramparentdir_initial(self):
        api = nscldaq.vardb.statemanager.Api(
            'tcp://localhost', 'tcp://localhost'
        )
        self.assertEqual('/RunState', api.getProgramParentDir())
        
    def test_getprogramparentdir_changed(self):
        self._api.mkdir('/programs')
        self._api.set('/RunState/ReadoutParentDir', '/programs')
        api = nscldaq.vardb.statemanager.Api(
            'tcp://localhost', 'tcp://localhost'
        )
        self.assertEqual('/programs', api.getProgramParentDir())
        
    def test_getprogramparentdir_argchecks(self):
        api = nscldaq.vardb.statemanager.Api(
                'tcp://localhost', 'tcp://localhost'
        )
        with self.assertRaises(nscldaq.vardb.statemanager.error):
            api.getProgramParentDir('abcde')
            
    def test_setprogramparentdir(self):
        self._api.mkdir('/programs')
        api = nscldaq.vardb.statemanager.Api(
                'tcp://localhost', 'tcp://localhost'
        )
        api.setProgramParentDir('/programs')
        self.assertEqual('/programs', api.getProgramParentDir())
    def test_setprogramparentdir_argchecks(self):
        api = nscldaq.vardb.statemanager.Api(
                'tcp://localhost', 'tcp://localhost'
        )
        with self.assertRaises(nscldaq.vardb.statemanager.error):
            api.setProgramParentDir()         # too few
        with self.assertRaises(nscldaq.vardb.statemanager.error):
            api.setProgramParentDir('/a', '/b') # too many
            
    def test_addProgram_ok(self):     # Fully specified program:
        program = {
            'enabled' : True, 'standalone' : False,
            'path' : '/users/fox/test', 'host' : 'charlie.nscl.msu.edu',
            'outring' : 'output', 'inring' : 'tcp://localhost/george'
        }
        api = nscldaq.vardb.statemanager.Api(
                'tcp://localhost', 'tcp://localhost'
        )
        api.addProgram('myprogram', program)
        self.assertEqual(('myprogram', 'test'), self._api.ls('/RunState'))
        self.assertEqual('true', self._api.get('/RunState/myprogram/enable'))
        self.assertEqual('false', self._api.get('/RunState/myprogram/standalone'))
        self.assertEqual('/users/fox/test', self._api.get('/RunState/myprogram/path'))
        self.assertEqual('charlie.nscl.msu.edu', self._api.get('/RunState/myprogram/host'))
        self.assertEqual('output', self._api.get('/RunState/myprogram/outring'))
        self.assertEqual('tcp://localhost/george',self._api.get('/RunState/myprogram/inring'))
        
        
    def test_addProgram_defaults(self):    # Partially speced gives defaults.
        program = {
            'path' : '/users/fox/test', 'host' : 'charlie.nscl.msu.edu',
        }                         # Minimal def.
        api = nscldaq.vardb.statemanager.Api(
                'tcp://localhost', 'tcp://localhost'
        )
        api.addProgram('myprogram', program)
        self.assertEqual(('myprogram', 'test'), self._api.ls('/RunState'))
        self.assertEqual('true', self._api.get('/RunState/myprogram/enable'))
        self.assertEqual('false', self._api.get('/RunState/myprogram/standalone'))
        self.assertEqual('/users/fox/test', self._api.get('/RunState/myprogram/path'))
        self.assertEqual('charlie.nscl.msu.edu', self._api.get('/RunState/myprogram/host'))
        self.assertEqual('', self._api.get('/RunState/myprogram/outring'))
        self.assertEqual('',self._api.get('/RunState/myprogram/inring'))
                
    def test_addProgram_argcheck(self):
        api = nscldaq.vardb.statemanager.Api(
                'tcp://localhost', 'tcp://localhost'
        )
        with self.assertRaises(nscldaq.vardb.statemanager.error):
            api.addProgram('myprogram')
        program = {
            'path' : '/users/fox/test', 'host' : 'charlie.nscl.msu.edu',
        }  
        with self.assertRaises(nscldaq.vardb.statemanager.error):
            api.addProgram('myprogram', program, 'junk')
        with self.assertRaises(nscldaq.vardb.statemanager.error):
            api.addProgram('myprogram', 'junk')   # program def mb dict.
            
    def test_addProgram_dupname(self):
        program = {
            'path' : '/users/fox/test', 'host' : 'charlie.nscl.msu.edu',
        }                         # Minimal def.
        api = nscldaq.vardb.statemanager.Api(
                'tcp://localhost', 'tcp://localhost'
        )
        api.addProgram('myprogram', program)
        with self.assertRaises(nscldaq.vardb.statemanager.error):
            api.addProgram('myprogram', program)
            
    def test_addProgram_underspecified(self):
        program = {
             'host' : 'charlie.nscl.msu.edu',
        }
        api = nscldaq.vardb.statemanager.Api(
                'tcp://localhost', 'tcp://localhost'
        )
        with self.assertRaises(nscldaq.vardb.statemanager.error):
            api.addProgram('myprogram', program)
         
        program = { 'path' : '/user/fox/junk' }
        with self.assertRaises(nscldaq.vardb.statemanager.error):
            api.addProgram('myprogram', program)

    def test_getProgramDef_ok(self):
        api = nscldaq.vardb.statemanager.Api(
                'tcp://localhost', 'tcp://localhost'
        )
        self.assertEqual(
            {'standalone' : True, 'enabled' : True, 'path' : '', 'host' : '',
             'outring' : '', 'inring' : ''},
            api.getProgramDefinition('test'))
    def test_getProgramDef_badName(self):
        api = nscldaq.vardb.statemanager.Api(
                'tcp://localhost', 'tcp://localhost'
        )
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            api.getProgramDefinition('junk')
            
    def test_getProgramDef_argcheck(self):
        api = nscldaq.vardb.statemanager.Api(
                'tcp://localhost', 'tcp://localhost'
        )
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            api.getProgramDefinition()
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            api.getProgramDefinition('test', 'junk')
            
    def test_modifyProgram_ok(self):
        api = nscldaq.vardb.statemanager.Api(
                'tcp://localhost', 'tcp://localhost'
        )
        progDef = api.getProgramDefinition('test')
        #
        #  Make some mods:
        #
        progDef['path'] = '/home/fox/stuff'
        progDef['host'] = 'spdaq20.nscl.msu.edu'
    
        api.modifyProgram('test', progDef)
        
        #  Check that they took.
        
        self.assertEqual(progDef, api.getProgramDefinition('test'))
        
        
    def test_modifyProgram_nosuch(self):
        api = nscldaq.vardb.statemanager.Api(
                'tcp://localhost', 'tcp://localhost'
        )
        progDef = api.getProgramDefinition('test')
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            api.modifyProgram('junk', progDef)    # No such program.
        
    def test_modifyProgram_checkargs(self):
        api = nscldaq.vardb.statemanager.Api(
                'tcp://localhost', 'tcp://localhost'
        )
        progDef = api.getProgramDefinition('test')
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            api.modifyProgram('test')     # need def.
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            api.modifyProgram('test', progDef, 'junk')   #extra stuff
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            api.modifyProgram('test', 'junk')    # def not dict.
        
    def test_enableProgram_ok(self):
        self._api.set('/RunState/test/enable', 'false')   # first disable.
        api = nscldaq.vardb.statemanager.Api(
                'tcp://localhost', 'tcp://localhost'
        )
        api.enableProgram('test')
        self.assertEqual('true', self._api.get('/RunState/test/enable'))
        
    def test_enableProgram_nosuch(self):
        api = nscldaq.vardb.statemanager.Api(
                'tcp://localhost', 'tcp://localhost'
        )
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            api.enableProgram('junk')
    def test_enableProgram_argCheck(self):
        api = nscldaq.vardb.statemanager.Api(
                'tcp://localhost', 'tcp://localhost'
        )
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            api.enableProgram()            # need name.
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            api.enableProgram('test', 'junk') # Extra param
   
    def test_disableProgram_Ok(self):
        self._api.set('/RunState/test/enable', 'true')   # first enable.
        api = nscldaq.vardb.statemanager.Api(
                'tcp://localhost', 'tcp://localhost'
        )
        api.disableProgram('test')
        self.assertEqual('false', self._api.get('/RunState/test/enable'))
        
    def test_disableProgram_nosuch(self):
        api = nscldaq.vardb.statemanager.Api(
                'tcp://localhost', 'tcp://localhost'
        )
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            api.disableProgram('junk')
        
    def test_disablProgram_argcheck(self):
        api = nscldaq.vardb.statemanager.Api(
                'tcp://localhost', 'tcp://localhost'
        )
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            api.disableProgram()
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            api.disableProgram('test', 'junk')
            
    def test_isProgramEnabled_ok(self):
        api = nscldaq.vardb.statemanager.Api(
                'tcp://localhost', 'tcp://localhost'
        )
        api.enableProgram('test')
        self.assertTrue(api.isProgramEnabled('test'))
        
        api.disableProgram('test')
        self.assertFalse(api.isProgramEnabled('test'))
        
    def test_isProgramEnabled_nosuch(self):
        api = nscldaq.vardb.statemanager.Api(
                'tcp://localhost', 'tcp://localhost'
        )
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            api.isProgramEnabled('junk')
        
    def test_isProgramEnabled_argcheck(self):
        api = nscldaq.vardb.statemanager.Api(
                'tcp://localhost', 'tcp://localhost'
        )
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            api.isProgramEnabled()
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            api.isProgramEnabled('test', 'junk')
            
    def test_setStandalone_ok(self):
        self._api.set('/RunState/test/standalone', 'false')  #ensure false first.
        api = nscldaq.vardb.statemanager.Api(
                'tcp://localhost', 'tcp://localhost'
        )
        api.setProgramStandalone('test')
        self.assertEqual('true', self._api.get('/RunState/test/standalone'))
        
    def test_setStandalone_nosuch(self):
        api = nscldaq.vardb.statemanager.Api(
                'tcp://localhost', 'tcp://localhost'
        )
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            api.setProgramStandalone('junk')
            
    def test_setStandalone_argcheck(self):
        api = nscldaq.vardb.statemanager.Api(
                'tcp://localhost', 'tcp://localhost'
        )
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            api.setProgramStandalone()
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            api.setProgramStandalone('test', 'junk')
            
    def test_setNostandalone_ok(self):
        self._api.set('/RunState/test/standalone', 'true')  #ensure true
        api = nscldaq.vardb.statemanager.Api(
                'tcp://localhost', 'tcp://localhost'
        )
        api.setProgramNoStandalone('test')
        self.assertEqual('false', self._api.get('/RunState/test/standalone'))
        
    def test_setNostandalone_nosuch(self):
        api = nscldaq.vardb.statemanager.Api(
                'tcp://localhost', 'tcp://localhost'
        )
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            api.setProgramNoStandalone('junk')
            
    def test_setNostandalone_argcheck(self):
        api = nscldaq.vardb.statemanager.Api(
                'tcp://localhost', 'tcp://localhost'
        )
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            api.setProgramNoStandalone()
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            api.setProgramNoStandalone('test', 'junk')
            
    def test_isStandalone_ok(self):
        api = nscldaq.vardb.statemanager.Api(
                'tcp://localhost', 'tcp://localhost'
        )
        api.setProgramStandalone('test')
        self.assertTrue(api.isProgramStandalone('test'))
        
        api.setProgramNoStandalone('test')
        self.assertFalse(api.isProgramStandalone('test'))
        
    def test_isStandalone_nosuch(self):
        api = nscldaq.vardb.statemanager.Api(
                'tcp://localhost', 'tcp://localhost'
        )
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            api.isProgramStandalone('junk')
            
    def test_isStandalone_argcheck(self):
        api = nscldaq.vardb.statemanager.Api(
                'tcp://localhost', 'tcp://localhost'
        )
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            api.isProgramStandalone()
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            api.isProgramStandalone('test', 'junk')
    
# Run the tests if this is main:


if __name__ == '__main__':
    unittest.main()
