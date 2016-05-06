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
# @file   testGet.py
# @brief  Test server's ability to get the value of a variable.
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

class TestGet(testBase.TestBase):
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

    def test_getExisting(self):
        self.mkdir('/test')
        self.makevar('/test/var', 'integer', '1234')
        
        self._req.send('GET:/test/var:')
        self.assertEqual('OK:1234', self._req.recv())
        
        
    def test_noSuchDir(self):
        self._req.send('GET:/test/var:')
        self.assertEqual('FAIL:CVarDirTree::cd - bad path: /test', self._req.recv())

    def test_nosuch(self):
        self._req.send('GET:/junk:')
        self.assertEqual('FAIL:CVariable constructor - no such variable', self._req.recv())

if __name__ == '__main__':
    unittest.main()


