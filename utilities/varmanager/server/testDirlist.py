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
# @file   testDirlist.py
# @brief  Test the dirlist function of the server.
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

class TestDirlist(testBase.TestBase):
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

    
    # Empty directory gives an empty data field:
    
    def test_empty(self):
        self._req.send('DIRLIST:/:')
        reply = self._req.recv()     # No change so no publication to flush.
        replyList = reply.split(':')
        self.assertEqual(2, len(replyList))
        self.assertEqual('', replyList[1])
        
    #  If a have a single directory that should come back as the single item
    
    def test_one(self):
        self.mkdir('/subdir')
        self._req.send('DIRLIST:/:')
        reply = self._req.recv()
        replyList = reply.split(':')
        self.assertEqual('subdir', replyList[1])
        
    # Should get a list of all subdirs if there are several
    #  These will be pipe separated.
        
    def test_several(self):
        self.mkdir("/dir1")
        self.mkdir("/dir2")
        self.mkdir("/dir3")
        self._req.send('DIRLIST:/:')
        reply = self._req.recv()
        replyList = reply.split(':')
        dirList   = replyList[1].split('|')
        self.assertEqual(3, len(dirList))
        dirList   = sorted(dirList)             # no assumptions about order.
        self.assertEqual('dir1', dirList[0])
        self.assertEqual('dir2', dirList[1])
        self.assertEqual('dir3', dirList[2])
    
    # Should get the info from the right subdir:
        
    def test_subdir(self):
        self.mkdir('/dir1')
        self.mkdir('/dir2')
        self.mkdir('/dir1/subdir')
        self._req.send('DIRLIST:/dir1:')
        reply = self._req.recv()
        replyList = reply.split(':')
        
        self.assertEqual('subdir', replyList[1])
        
    # Should get an error if listing a nonexistent path:
    
    def test_badPath(self):
        self._req.send('DIRLIST:/dir1:')
        reply = self._req.recv()
        replyList = reply.split(':')
        self.assertEqual('FAIL', replyList[0])
        
    
    
if __name__ == '__main__':
    unittest.main()

