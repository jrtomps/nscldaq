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
# @file   mkdirTests.py
# @brief  Test server's functionality on the mkdir operation.
# @author <fox@nscl.msu.edu>

import unittest
import os
import os.path
import subprocess
import tempfile
import signal
import time
import getpass
import zmq
import nscldaq.vardb.vardb
import nscldaq.vardb.dirtree

import testBase


from nscldaq.portmanager import PortManager

class mkdirTests(testBase.TestBase):
    #------------------------- Initialization/teardown 
    ##
    #  We need to:
    #    *  Create a new temporary file name for the database.
    #    *  Locate the server and save it's location in member data.
    #
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
        
        self._requestService = 'vardb-request'
        self._pubService     = 'vardb-changes'
        self.setup0mq()
        
    ##
    #  We need to:
    #   *  Delete any temporary file we may have created (database).
    #   *  Kill off the server if it's running (non Null self._pid)
    #   *  close any pipe fd's that might have also been created.
    #
    def tearDown(self):
        if self._pid is not None:
            os.kill(self._pid, signal.SIGKILL)
            os.waitpid(self._pid,0)
        if self._stdout is not None:
            self._stdout.close()
        if os.path.isfile(self._dbName):
            os.unlink(self._dbName)
        self._zmq.destroy()
            
    
    #------------------- The tests ---------------
    
    ##
    # Test that we can make a directory
    # *  - must get an ok response.
    # *  - must get a publication of the creation.
    # *  - must see the directory in the database.
    #
    def test_mkdirok(self):
        message = 'MKDIR:/test:'
        self._req.send(message)
        
        reply = self._req.recv()
        self.assertEqual('OK:', reply)
        
        notification = self._sub.recv()
        self.assertEqual('/:MKDIR:test', notification)
        
        db = nscldaq.vardb.vardb.VarDb(self._dbName)
        dir = nscldaq.vardb.dirtree.DirTree(db)
        subdirs = dir.ls()
        self.assertEqual(1, len(subdirs))
        self.assertEqual('test', subdirs[0])
    ##
    #   Creating a duplicate directory should result in an error
    #   message to that effect.  There should also not be anything
    #   sent to the pub/sub system:
    
    def test_mkdirdup(self):
        self.test_mkdirok()             # Makes /test
        message = 'MKDIR:/test:'
        self._req.send(message)
        reply = self._req.recv()
        self.assertEqual('FAIL:Attempted to create a duplicate directory: %s' % '/test', reply)
        
        self.assertEqual(0, self._sub.poll(10))
        
        
    
if __name__ == '__main__':
    unittest.main()


