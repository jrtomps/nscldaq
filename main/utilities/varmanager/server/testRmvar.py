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
# @file   testRmvar.py
# @brief  tests for the server RMVAR operation.
# @author <fox@nscl.msu.edu>




import testBase
import unittest
import tempfile
import os, os.path
import signal

import nscldaq.vardb.vardb
import nscldaq.vardb.dirtree
import nscldaq.vardb.variable
import nscldaq.vardb.statemachine

import zmq

class TestVarlist(testBase.TestBase):
    # Get a temp file name..
    def setUp(self):
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

        self._requestService = 'vardb-request'
        self._pubService     = 'vardb-changes'
        self.setup0mq()
        #self._dbName = 'test.db'
        self._db = nscldaq.vardb.vardb.VarDb(self._dbName)
        self.setdb(self._db)
    ##
    #  We need to:
    #   *  Delete any temporary file we may have created (database).
    #   *  Kill off the server if it's running (non Null self._pid)
    #   *  close any pipe fd's that might have also been created.
    #
    def tearDown(self):
        if self._pid is not None:
            os.kill(self._pid, signal.SIGKILL)
            os.waitpid(self._pid, 0)
        if self._stdout is not None:
            self._stdout.close()
        if os.path.isfile(self._dbName):
            os.unlink(self._dbName)
            pass
        self._zmq.destroy()
        
    #-------------------------------------------------------------------------
    #  The tests.
    #
    
    
    #  I can remove a variable that I created:
    
    def test_RmOk(self):
        self.makevar('/avar', 'integer', '1234')
        self._req.send('RMVAR:/avar:')
        reply =  self._req.recv()
        result = reply.split(':')
        self.assertEquals('OK', result[0])
        
        #  Flush any publication
        
        
        self._sub.recv()
        
        # Check that the varible is gone:
        
        
        dir = nscldaq.vardb.dirtree.DirTree(self._db)
        vars = nscldaq.vardb.variable.ls(self._db, dir)
        self.assertEquals(0, len(vars))
        
    #  I can remove the correct one of several:
    
    def test_CorrectOne(self):
        self.makevar('/avar', 'integer', '1234')
        self.makevar('/bvar', 'string', 'hello world')
        self.makevar('/cvar', 'real', '3.1415926539')
        
        self._req.send('RMVAR:/bvar:')
        reply = self._req.recv()
        result = reply.split(':')
        self.assertEquals('OK', result[0])
        
        self._sub.recv()
        
        dir = nscldaq.vardb.dirtree.DirTree(self._db)
        vars = nscldaq.vardb.variable.ls(self._db, dir)
        self.assertEquals(2, len(vars))
        
        # Figure out the set of names:
        
        names = set()
        for var in vars:
            names.add(var['name'])
        
        
        self.assertFalse('bvar' in names)
        
    def test_nosuch(self):
        self._req.send('RMVAR:/nosuch:')
        reply = self._req.recv()
        result = reply.split(':')
        self.assertEquals('FAIL', result[0])
        

    def test_notify(self):
        self.makevar('/avar', 'integer', '1234')
        self.makevar('/bvar', 'string', 'hello world')
        self.makevar('/cvar', 'real', '3.1415926539')
        
        self._req.send('RMVAR:/bvar:')
        reply = self._req.recv()
        result = reply.split(':')
        self.assertEquals('OK', result[0])
        
        notify = self._sub.recv()
        notification = notify.split(':')
        self.assertEquals('/bvar', notification[0])
        self.assertEquals('RMVAR', notification[1])
        self.assertEquals('', notification[2])
    

if __name__ == '__main__':
    unittest.main()

