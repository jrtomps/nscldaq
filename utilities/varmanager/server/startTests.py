#
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
# @file   startTests.py
# @brief  Test startup of the server.
# @author <fox@nscl.msu.edu>

import unittest
import tempfile
import os
import os.path
import signal
import subprocess
import time
import testBase
import nscldaq.vardb.vardb

class TestServerStart(testBase.TestBase):
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
        
    ##
    #  We need to:
    #   *  Delete any temporary file we may have created (database).
    #   *  Kill off the server if it's running (non Null self._pid)
    #   *  close any pipe fd's that might have also been created.
    #
    def tearDown(self):
        if self._pid is not None:
            os.kill(self._pid, signal.SIGKILL)
        if self._stdout is not None:
            self._stdout.close()
        if os.path.isfile(self._dbName):
            os.unlink(self._dbName)
            
    
    #-------------------------------------------------------------------------
    #   Tests for starting the server with various --database problems.
    
    ##
    # test_nodbgiven
    #   The server should fail to start and give us an error that
    #   'no database file was given'.
    #   (first line, remainder will be the Usage text).
    #   if we don't provide a --database option.
    def test_nodbgiven(self):
        process = self.startServer([])
        outputLines = self.getLines(process)
        
        # Should also no longer be a process:
        
        self.markExited()
        self.assertEquals("No database file was given", outputLines[0])
        
 
    ##
    #  test_dbnotfound
    #
    #  If --create is no (default) and the database is not a valid
    #  file, the error message is:  'No such database file'.
    #
    def test_dbnotfound(self):
        process = self.startServer(['--database', 'this/does/not/exist'])
        output = self.getLines(process)
        self.markExited()
        
        self.assertEquals("No such database file", output[0])
    ##
    # If the file exists but is not a database the server should complain:
    #  '--database is not a valid variable database file'.
    
    def test_dbnotdb(self):
        f = open(self._dbName, 'w+')
        f.close()
        
        process = self.startServer(['--database' , self._dbName])
        output = self.getLines(process)
        self.markExited()
        
        self.assertEquals('--database is not a valid variable database file', output[0])
    
    ##
    # with --create yes the database can be created if it does not exist:
    #
    def test_dbcreate(self):
        process = self.startServer(['--database', self._dbName, '--create-ok', 'yes'])
        time.sleep(1)
        self.assertTrue(os.path.isfile(self._dbName))
    
    ##
    #  Test PING operation.
    
    def test_ping(self):
        process = self.startServer(['--database', self._dbName, '--create-ok', 'yes'])
        self._requestService = 'vardb-request'
        self._pubService     = 'vardb-changes'
        time.sleep(1)
        self.setup0mq()
        self._req.send('PING::')
        msg = self._req.recv()
        msgFields = msg.split(':')
        self.assertEqual('OK', msgFields[0])
        
        pass
    
    
    
if __name__ == '__main__':
    unittest.main()


