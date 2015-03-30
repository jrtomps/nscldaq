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
# @file   testVarlist.py
# @brief  Test the server VARLIST operation.
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

        if self._stdout is not None:
            self._stdout.close()
        if os.path.isfile(self._dbName):
            os.unlink(self._dbName)
            pass
        self._zmq.destroy()
        
    

        
        
    #----------------------------------- tests -----------------------------
    
    # Empty directory gives me an empty list:
    
    def test_empty(self):
        self._req.send('VARLIST:/:')
        reply = self._req.recv()
        replyList = reply.split(':')
        self.assertEquals(2, len(replyList))
        self.assertEquals('OK', replyList[0])
        self.assertEquals('', replyList[1])
    
    # If the directory has a single int:
        
    def test_oneInt(self):
        self.makevar('/aninteger', 'integer', '0')
        self._req.send('VARLIST:/:')
        reply = self._req.recv()
        replyList = reply.split(':')
        self.assertEquals('aninteger|integer|0', replyList[1])
        
    def test_oneReal(self):
        self.makevar('/areal', 'real', '1.414')
        self._req.send('VARLIST:/:')
        reply = self._req.recv()
        replyList = reply.split(':')
        self.assertEquals('areal|real|1.414', replyList[1])
        
    def test_oneString(self):
        self.makevar('/astring', 'string', 'hello world')
        self._req.send('VARLIST:/:')
        reply = self._req.recv()
        replyList = reply.split(':')
        self.assertEquals('astring|string|hello world', replyList[1])
        
    def test_several(self):
        self.makevar('/aninteger', 'integer', '0')
        self.makevar('/areal', 'real', '1.414')
        self.makevar('/astring', 'string', 'hello world')
        
        self._req.send('VARLIST:/:')
        reply = self._req.recv()
        
        vars = self._analyzeVarlistReply(reply)
        
        self.assertEquals(3, len(vars))
        
        self.assertEquals('aninteger', vars[0]['name'])
        self.assertEquals('integer', vars[0]['type'])
        self.assertEquals('0', vars[0]['value'])
        
        self.assertEquals('areal', vars[1]['name'])
        self.assertEquals('real', vars[1]['type'])
        self.assertEquals('1.414', vars[1]['value'])
        
        self.assertEquals('astring', vars[2]['name'])
        self.assertEquals('string', vars[2]['type'])
        self.assertEquals('hello world', vars[2]['value'])
        
    def test_subdir(self):
        self.mkdir("/subdir")
        self.makevar('/subdir/avar', 'integer', '1234')
        self._req.send('VARLIST:/subdir:')
        reply = self._req.recv()
        
        vars = self._analyzeVarlistReply(reply)
        self.assertEquals(1, len(vars))
        self.assertEquals('avar', vars[0]['name'])
        self.assertEquals('integer', vars[0]['type'])
        self.assertEquals('1234', vars[0]['value'])
        
    def test_badPath(self):
        self._req.send("VARLIST:/subdir:")
        reply = self._req.recv()
        self.assertEquals('FAIL', reply.split(':')[0])

   
    
if __name__ == '__main__':
    unittest.main()
