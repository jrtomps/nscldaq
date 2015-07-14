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
        api.declare('/RunState/Timeout', 'integer', '60')
        api.declare('/RunState/ReadoutParentDir', 'string')
        api.declare('/RunState/State', 'RunStateMachine')
        
        # Make the test program:
        
        api.mkdir("/RunState/test")
        api.declare('/RunState/test/standalone', 'boolean', 'false')
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

class Creation(StateManagerTests):
    def test_creation(self):
        api = nscldaq.vardb.statemanager.Api(
            'tcp://localhost', 'tcp://localhost'
        )
        
class ProgramParentDir(StateManagerTests):
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

class ProgramDef(StateManagerTests):
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
            {'standalone' : False, 'enabled' : True, 'path' : '', 'host' : '',
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

class ProgramParticipation(StateManagerTests):        
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
 
class ProgramListing(StateManagerTests):   
    def test_listprograms_1(self):
        api = nscldaq.vardb.statemanager.Api(
                'tcp://localhost', 'tcp://localhost'
        )
        self.assertEquals(['test'], api.listPrograms())
    def test_listprograms_a_few(self):
        api = nscldaq.vardb.statemanager.Api(
                'tcp://localhost', 'tcp://localhost'
        )
        # Add some programs:
        
        program = {
            'enabled' : True, 'standalone' : False,
            'path' : '/users/fox/test', 'host' : 'charlie.nscl.msu.edu',
            'outring' : 'output', 'inring' : 'tcp://localhost/george'
        }
        api.addProgram('atest', program)
        api.addProgram('btest', program)
        api.addProgram('ztest', program)
        
        self.assertEquals(
            ['atest', 'btest', 'test', 'ztest'],
            api.listPrograms()
        )
    def test_listprograms_none(self):
        self._api.mkdir("/programs")         # empty.
        api = nscldaq.vardb.statemanager.Api(
                'tcp://localhost', 'tcp://localhost'
        )
        api.setProgramParentDir('/programs')
        self.assertEquals([], api.listPrograms())
    
    def test_listprograms_argcheck(self):
        api = nscldaq.vardb.statemanager.Api(
                'tcp://localhost', 'tcp://localhost'
        )
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            api.listPrograms('junk')
        
    def test_listenabled_1(self):
        api = nscldaq.vardb.statemanager.Api(
                'tcp://localhost', 'tcp://localhost'
        )
        self.assertEquals(['test'], api.listEnabledPrograms())
        
    def test_listenabled_multiple(self):
        api = nscldaq.vardb.statemanager.Api(
                'tcp://localhost', 'tcp://localhost'
        )
        # Add some programs (all enabled):
        
        program = {
            'enabled' : True, 'standalone' : False,
            'path' : '/users/fox/test', 'host' : 'charlie.nscl.msu.edu',
            'outring' : 'output', 'inring' : 'tcp://localhost/george'
        }
        api.addProgram('atest', program)
        api.addProgram('btest', program)
        api.addProgram('ztest', program)
        
        self.assertEquals(
            ['atest', 'btest', 'test', 'ztest'],
            api.listEnabledPrograms()
        )
    def test_listenabled_none(self):
        api = nscldaq.vardb.statemanager.Api(
                'tcp://localhost', 'tcp://localhost'
        )
        # disable test:
        
        api.disableProgram('test')
        
        self.assertEquals([], api.listEnabledPrograms())
        
    def test_listenabled_argcheck(self):
        api = nscldaq.vardb.statemanager.Api(
                'tcp://localhost', 'tcp://localhost'
        )
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            api.listEnabledPrograms('junk')
    
    def test_liststandalone_none(self) :
        api = nscldaq.vardb.statemanager.Api(
                'tcp://localhost', 'tcp://localhost'
        )
        self.assertEquals([], api.listStandalonePrograms())
        
    def test_liststandalone_1(self) :
        api = nscldaq.vardb.statemanager.Api(
                'tcp://localhost', 'tcp://localhost'
        )
        api.setProgramStandalone('test')
        self.assertEquals(['test'], api.listStandalonePrograms())
        
    def test_liststandalone_many(self):
        api = nscldaq.vardb.statemanager.Api(
                'tcp://localhost', 'tcp://localhost'
        )
        program = {
            'enabled' : True, 'standalone' : True,
            'path' : '/users/fox/test', 'host' : 'charlie.nscl.msu.edu',
            'outring' : 'output', 'inring' : 'tcp://localhost/george'
        }
        api.addProgram('atest', program)
        api.addProgram('btest', program)
        api.addProgram('ztest', program)
        
        self.assertEquals(
            ['atest', 'btest', 'ztest'],
            api.listStandalonePrograms()
        )
    def test_liststandalone_argcheck(self):
        api = nscldaq.vardb.statemanager.Api(
                'tcp://localhost', 'tcp://localhost'
        )
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            api.listStandalonePrograms('junk')
    
    # Active programs are those that are enabled and not standalone.
    
    def test_listinactive_none(self):
        api = nscldaq.vardb.statemanager.Api(
                'tcp://localhost', 'tcp://localhost'
        )
        self.assertEquals([], api.listInactivePrograms())
        
    def test_listinactive_1(self):
        api = nscldaq.vardb.statemanager.Api(
                'tcp://localhost', 'tcp://localhost'
        )
        api.disableProgram('test')
        self.assertEquals(['test'], api.listInactivePrograms())
        
        api.enableProgram('test')
        api.setProgramStandalone('test')
        self.assertEquals(['test'], api.listInactivePrograms())
        
        
    def test_listinactive_several(self):
        api = nscldaq.vardb.statemanager.Api(
                'tcp://localhost', 'tcp://localhost'
        )
        program = {
            'enabled' : True, 'standalone' : True,
            'path' : '/users/fox/test', 'host' : 'charlie.nscl.msu.edu',
            'outring' : 'output', 'inring' : 'tcp://localhost/george'
        }
        api.addProgram('atest', program)
        
        program['standalone'] = False
        program['enabled']    = False
        api.addProgram('btest', program)
        
        program['standalone'] = True
        api.addProgram('ztest', program)
        
        self.assertEquals(
            ['atest', 'btest', 'ztest'], api.listInactivePrograms()
        )
    def test_listinactive_argcheck(self):
        api = nscldaq.vardb.statemanager.Api(
                'tcp://localhost', 'tcp://localhost'
        )
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            api.listInactivePrograms('junk')
            
    def test_listactive_1(self):
        api = nscldaq.vardb.statemanager.Api(
                'tcp://localhost', 'tcp://localhost'
        )
        self.assertEquals(['test'], api.listActivePrograms())
        
    def test_listactive_0(self):
        api = nscldaq.vardb.statemanager.Api(
                'tcp://localhost', 'tcp://localhost'
        )
        api.disableProgram('test')
        self.assertEquals([], api.listActivePrograms())
        
        api.enableProgram('test')
        api.setProgramStandalone('test')
        self.assertEquals([], api.listActivePrograms())
        
    def test_listactive_many(self):
        api = nscldaq.vardb.statemanager.Api(
                'tcp://localhost', 'tcp://localhost'
        )
        program = {
            'enabled' : True, 'standalone' : True,
            'path' : '/users/fox/test', 'host' : 'charlie.nscl.msu.edu',
            'outring' : 'output', 'inring' : 'tcp://localhost/george'
        }
        api.addProgram('atest', program)
        
        program['standalone'] = False
        api.addProgram('btest', program)
        
        program ['enabled'] = False
        api.addProgram('ztest', program)
        
        self.assertEqual(['btest', 'test'], api.listActivePrograms())
        
    def test_listactive_argcheck(self) :
        api = nscldaq.vardb.statemanager.Api(
                'tcp://localhost', 'tcp://localhost'
        )
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            api.listActivePrograms('junk')
            
class DeleteProgram(StateManagerTests):
    def test_deleteprog_ok(self):
        api = nscldaq.vardb.statemanager.Api(
                'tcp://localhost', 'tcp://localhost'
        )
        api.deleteProgram('test')
        self.assertEqual([], api.listPrograms())
        
    def test_deleteprog_nox(self):
        api = nscldaq.vardb.statemanager.Api(
                'tcp://localhost', 'tcp://localhost'
        )
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            api.deleteProgram('nosuch')
    def test_deleteprog_argcheck(self):
        api = nscldaq.vardb.statemanager.Api(
                'tcp://localhost', 'tcp://localhost'
        )
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            api.deleteProgram()
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            api.deleteProgram('test', 'junk')

class StateS(StateManagerTests):
    def test_setglobalstate_ok(self):
        api = nscldaq.vardb.statemanager.Api(
                'tcp://localhost', 'tcp://localhost'
        )
        api.setGlobalState('NotReady')
        self.assertEquals('NotReady', self._api.get('/RunState/State'))
        
    def test_setglobalstate_badstate(self) :
        api = nscldaq.vardb.statemanager.Api(
                'tcp://localhost', 'tcp://localhost'
        )
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            api.setGlobalState('Readying')        # Inv transtion.
        
    def test_setglobalstate_argcheck(self) :
        api = nscldaq.vardb.statemanager.Api(
                'tcp://localhost', 'tcp://localhost'
        )
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            api.setGlobalState()        # Missing param.
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            api.setGlobalState('NotReady', 'junk')
            
    def test_getglobalstate_initial(self):
        api = nscldaq.vardb.statemanager.Api(
                'tcp://localhost', 'tcp://localhost'
        )
        self.assertEqual('0Initial', api.getGlobalState())
    def test_getglobalstate_changed(self):
        api = nscldaq.vardb.statemanager.Api(
                'tcp://localhost', 'tcp://localhost'
        )
        api.setGlobalState('NotReady')
        self.assertEqual('NotReady', api.getGlobalState())
    def test_getglobalstate_argcheck(self):
        api = nscldaq.vardb.statemanager.Api(
                'tcp://localhost', 'tcp://localhost'
        )
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            api.getGlobalState('junk')
    
    def test_getpartstates_initial(self):
        api = nscldaq.vardb.statemanager.Api(
                'tcp://localhost', 'tcp://localhost'
        )
        self.assertEqual(
            {'test' : '0Initial'}, api.getParticipantStates()
        )
    def test_getpartstates_seversl(self):
        api = nscldaq.vardb.statemanager.Api(
                'tcp://localhost', 'tcp://localhost'
        )
        programDef = {
        'enabled' : True, 'standalone' : False,
            'path' : '/users/fox/test', 'host' : 'charlie.nscl.msu.edu',
            'outring' : 'output', 'inring' : 'tcp://localhost/george'
        }
        api.addProgram('atest', programDef)
        api.addProgram('ztest', programDef)
        
        self.assertEqual(
            {'atest' : '0Initial', 'test': '0Initial', 'ztest' : '0Initial'},
            api.getParticipantStates()
        )
    def test_getpartstates_modified(self):
        api = nscldaq.vardb.statemanager.Api(
                'tcp://localhost', 'tcp://localhost'
        )
        programDef = {
        'enabled' : True, 'standalone' : False,
            'path' : '/users/fox/test', 'host' : 'charlie.nscl.msu.edu',
            'outring' : 'output', 'inring' : 'tcp://localhost/george'
        }
        api.addProgram('atest', programDef)
        api.addProgram('ztest', programDef)
        
        #  Diddle some states:
        
        self._api.set('/RunState/test/State', 'NotReady')
        self._api.set('/RunState/test/State', 'Readying')
        
        self._api.set('/RunState/ztest/State', 'NotReady')
        self._api.set('/RunState/atest/State', 'NotReady')
        
        self.assertEqual(
            {'atest' : 'NotReady', 'test' : 'Readying', 'ztest': 'NotReady'},
            api.getParticipantStates()
        )
    def test_getpartstates_argcheck(self):
        api = nscldaq.vardb.statemanager.Api(
                'tcp://localhost', 'tcp://localhost'
        )
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            api.getParticipantStates(0)
            
class RunParameters(StateManagerTests):   
    def test_gettitle_initial(self):
        api = nscldaq.vardb.statemanager.Api(
                'tcp://localhost', 'tcp://localhost'
        )
        self.assertEqual('', api.getTitle())

    def test_gettitle_modified(self):
        title = 'A new title'
        
        self._api.set('/RunState/Title', title)
        api = nscldaq.vardb.statemanager.Api(
                'tcp://localhost', 'tcp://localhost'
        )
        self.assertEqual(title, api.getTitle())
        
    def test_gettitle_argcheck(self):
        api = nscldaq.vardb.statemanager.Api(
                'tcp://localhost', 'tcp://localhost'
        )
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            api.getTitle(0)
            
    def test_settitle_ok(self):
        title = 'This is a test title'
        api = nscldaq.vardb.statemanager.Api(
                'tcp://localhost', 'tcp://localhost'
        )
        api.setTitle(title)
        self.assertEqual(title, api.getTitle())
        
    def test_settitle_argcheck(self):
        api = nscldaq.vardb.statemanager.Api(
                'tcp://localhost', 'tcp://localhost'
        )
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            api.setTitle()           # Need parameter.
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            api.setTitle('abcde', 0)   # extra param.
 
    def test_gettimeout_initial(self):
        api = nscldaq.vardb.statemanager.Api(
                'tcp://localhost', 'tcp://localhost'
        )
        self.assertEqual(60, api.getTimeout())
        
    def test_gettimeout_modified(self):
        self._api.set("/RunState/Timeout", '45')
        api = nscldaq.vardb.statemanager.Api(
                'tcp://localhost', 'tcp://localhost'
        )
        self.assertEqual(45, api.getTimeout())
        
    def test_gettimeout_argcheck(self):
        api = nscldaq.vardb.statemanager.Api(
                'tcp://localhost', 'tcp://localhost'
        )
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            api.getTimeout(0)
    
    def test_settimeout_ok(self):
        api = nscldaq.vardb.statemanager.Api(
                'tcp://localhost', 'tcp://localhost'
        )
        api.setTimeout(1234)
        self.assertEqual(1234, api.getTimeout())
        
    def test_settimeout_argcheck(self):
        api = nscldaq.vardb.statemanager.Api(
                'tcp://localhost', 'tcp://localhost'
        )
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            api.setTimeout()    # missing arg.
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            api.setTimeout('bad')   # Bad time.
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            api.setTimeout(1234, 42)   #Extra args.
            
    def test_isrecording_initial(self):
        api = nscldaq.vardb.statemanager.Api(
                'tcp://localhost', 'tcp://localhost'
        )
        self.assertFalse(api.isRecording())
    def test_isrecording_changed(self):
        self._api.set('/RunState/Recording', 'false')
        api = nscldaq.vardb.statemanager.Api(
                'tcp://localhost', 'tcp://localhost'
        )
        self.assertFalse(api.isRecording())
        
    def test_isrecording_argcheck(self):
        api = nscldaq.vardb.statemanager.Api(
                'tcp://localhost', 'tcp://localhost'
        )
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            api.isRecording(0)
    
    def test_setrecording_ok(self):
        api = nscldaq.vardb.statemanager.Api(
                'tcp://localhost', 'tcp://localhost'
        )
        api.setRecording(True)
        self.assertTrue(api.isRecording())
        
        api.setRecording(False)
        self.assertFalse(api.isRecording())
        
    def test_setrecording_argcheck(self):
        api = nscldaq.vardb.statemanager.Api(
                'tcp://localhost', 'tcp://localhost'
        )
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            api.setRecording()      # Missing parameter.
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            api.setRecording(True, 0)   # Extra param.
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            api.setRecording('1234')      # not a bool.
   
    def test_getrunnum_initial(self):
        api = nscldaq.vardb.statemanager.Api(
                'tcp://localhost', 'tcp://localhost'
        )
        self.assertEqual(0, api.getRunNumber())
    def test_getrunnum_changed(self):
        self._api.set("/RunState/RunNumber", "1234")
        api = nscldaq.vardb.statemanager.Api(
                'tcp://localhost', 'tcp://localhost'
        )
        self.assertEqual(1234, api.getRunNumber())
    def test_getrunnum_argcheck(self):
        api = nscldaq.vardb.statemanager.Api(
                'tcp://localhost', 'tcp://localhost'
        )
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            api.getRunNumber(0)
            
    def test_setrunnum_ok(self):
        api = nscldaq.vardb.statemanager.Api(
                'tcp://localhost', 'tcp://localhost'
        )
        api.setRunNumber(1234)
        self.assertEqual(1234, api.getRunNumber())
        
    def test_setrunnum_argcheck(self):
        api = nscldaq.vardb.statemanager.Api(
                'tcp://localhost', 'tcp://localhost'
        )
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            api.setRunNumber()
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            api.setRunNumber(1234, 0)
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            api.setRunNumber('abcd')
            

class WaitTransitionTests(StateManagerTests):
    def __init__(self, *args, **kwargs):
        super(WaitTransitionTests, self).__init__(*args, **kwargs)
        self._callbacklist = None
    
    def StateTransitionCallback(self, api, program, state, cd):
        self._callbacklist.append([program, state, cd])
    
    def test_waittransition_timeout(self):
        api = nscldaq.vardb.statemanager.Api(
                'tcp://localhost', 'tcp://localhost'
        )
        api.setTimeout(1)     # 1 second shortest timeout possible.
        api.setGlobalState('NotReady')
        self.assertFalse(api.waitTransition())
        
    def test_waittransition_onestep(self):
        api = nscldaq.vardb.statemanager.Api(
                'tcp://localhost', 'tcp://localhost'
        )
        api.setGlobalState('NotReady')
        self._api.set('/RunState/test/State', 'NotReady')
        self.assertTrue(api.waitTransition())
        
    def test_waittransition_multistep(self):
        self._api.set('/RunState/State', 'NotReady')
        self._api.set('/RunState/test/State', 'NotReady')
        time.sleep(1)       # Let the pubs settle.
        
        # Both global and 'test' state is 'NotReady'.
        
        api = nscldaq.vardb.statemanager.Api(
                'tcp://localhost', 'tcp://localhost'
        )
        api.setGlobalState('Readying')
        self._api.set('/RunState/test/State', 'Readying')
        self._api.set('/RunState/test/State', 'Ready')
        self.assertTrue(api.waitTransition())
        
        # Global state should have also advanced:
        
        self.assertEqual('Ready', api.getGlobalState())
        
    def test_waittransition_callbacks(self):
        self._callbacklist = []
        api = nscldaq.vardb.statemanager.Api(
                'tcp://localhost', 'tcp://localhost'
        )
        api.setGlobalState('NotReady')
        self._api.set('/RunState/test/State', 'NotReady')
        self.assertTrue(api.waitTransition(self.StateTransitionCallback, 1))
        self.assertEqual(
            [['test', 'NotReady', 1],], self._callbacklist
        )
        
    def test_waittransition_argcheck(self):
        api = nscldaq.vardb.statemanager.Api(
                'tcp://localhost', 'tcp://localhost'
        )
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            api.waitTransition(1, 0)    # Not callable!
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            api.waitTransition(
                self.test_waittransition_argcheck, 0, 1
            )                          # too many params.
            
class ProcessMessageTests(StateManagerTests):
    def __init__(self, *args, **kwargs):
        super(ProcessMessageTests, self).__init__(*args, **kwargs)
        self._callbacklist = None
    
    def setUp(self) :
        super(ProcessMessageTests, self).setUp()
        self._callbacklist = []
        
        
    def Callback(self, api, Notification, cd):
        self._callbacklist.append([Notification, cd])
    
    def test_processMessages_noCallbacks(self):
        api = nscldaq.vardb.statemanager.Api(
                'tcp://localhost', 'tcp://localhost'
        )
        api.processMessages(self.Callback, 1)
        self.assertEquals(
            [], self._callbacklist
        )
        
    def test_processMessages_gblchange(self):
        api = nscldaq.vardb.statemanager.Api(
                'tcp://localhost', 'tcp://localhost'
        )
        api.setGlobalState('NotReady')
        time.sleep(1)        # Wait for the publications.
        api.processMessages(self.Callback, 2)
        self.assertEquals(
            [[{'type' : "GlobalStateChange", "state" : "NotReady"}, 2]],
            self._callbacklist
        )
    def test_processMessage_programchange(self):
        api = nscldaq.vardb.statemanager.Api(
                'tcp://localhost', 'tcp://localhost'
        )
        self._api.set('/RunState/test/State', 'NotReady')
        time.sleep(1)
        api.processMessages(self.Callback, 3)
        self.assertEquals(
            [[{'type' : "ProgramStateChange",
               "state" : "NotReady",
               'program' : 'test'}, 3]],
            self._callbacklist
        )
        
    def test_processMessage_joins(self):
        program = {
            'enabled' : True, 'standalone' : False,
            'path' : '/users/fox/test', 'host' : 'charlie.nscl.msu.edu',
            'outring' : 'output', 'inring' : 'tcp://localhost/george'
        }
        api = nscldaq.vardb.statemanager.Api(
                'tcp://localhost', 'tcp://localhost'
        )
        api.addProgram('newprogram', program)
        time.sleep(1)
        api.processMessages(self.Callback, 4)
        self.assertEquals(
            [[{'type' : 'ProgramJoins', 'program': 'newprogram'}, 4]],
            self._callbacklist
        )
        
    def test_processMessage_leaves(self):
        api = nscldaq.vardb.statemanager.Api(
                'tcp://localhost', 'tcp://localhost'
        )
        api.deleteProgram('test')
        time.sleep(1)
        api.processMessages(self.Callback, 5)
        self.assertEquals(
            [[{'type' : 'ProgramLeaves', 'program' : 'test'}, 5]],
            self._callbacklist
        )
        
    def test_processMessages_argcheck(self):
        api = nscldaq.vardb.statemanager.Api(
                'tcp://localhost', 'tcp://localhost'
        )
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            api.processMessages()
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            api.processMessages(1)
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            api.processMessages(self.Callback, 1, 2)
    

class ProgramStates(StateManagerTests):
    def test_isactive_ok(self):
        api = nscldaq.vardb.statemanager.Api(
                'tcp://localhost', 'tcp://localhost'
        )
        self.assertTrue(api.isActive('test'))
        
        api.disableProgram('test')
        self.assertFalse(api.isActive('test'))
        
    def test_isactive_nosuch(self):
        api = nscldaq.vardb.statemanager.Api(
                'tcp://localhost', 'tcp://localhost'
        )
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            api.isActive('no-such-program')
            
    def test_isactive_argcheck(self):
        api = nscldaq.vardb.statemanager.Api(
                'tcp://localhost', 'tcp://localhost'
        )
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            api.isActive()           # Need program.
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            api.isActive('test', 'junk') # Extra param.

    def test_setprogstate_ok(self):
        api = nscldaq.vardb.statemanager.Api(
                'tcp://localhost', 'tcp://localhost'
        )
        api.setProgramState('test', 'NotReady')
        self.assertEquals(
            {'test' : 'NotReady'}, api.getParticipantStates()
        )
    def test_setprogstate_nosuch(self):
        api = nscldaq.vardb.statemanager.Api(
                'tcp://localhost', 'tcp://localhost'
        )
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            api.setProgramState('nosuch', 'NotReady')
        
    def test_setprogstate_badstate(self):
        api = nscldaq.vardb.statemanager.Api(
                'tcp://localhost', 'tcp://localhost'
        )
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            api.setProgramState('test', 'Ready')
            
    def test_setprogstate_argcheck(self):
        api = nscldaq.vardb.statemanager.Api(
                'tcp://localhost', 'tcp://localhost'
        )
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            api.setProgramState('test')  # Too few args.
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            api.setProgramState('test', 'NotReady', 0) #Too many
    
    def test_getprogstate_ok(self):
        api = nscldaq.vardb.statemanager.Api(
                'tcp://localhost', 'tcp://localhost'
        )
        self.assertEqual('0Initial', api.getProgramState('test'))
        api.setProgramState('test', 'NotReady')
        self.assertEqual('NotReady', api.getProgramState('test'))
        
    def test_getprogstate_argcheck(self):
        api = nscldaq.vardb.statemanager.Api(
                'tcp://localhost', 'tcp://localhost'
        )
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            api.getProgramState()    # Need program.
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            api.getProgramState('test', 0) # Extra param.
    
    
# Run the tests if this is main:


if __name__ == '__main__':
    unittest.main()
