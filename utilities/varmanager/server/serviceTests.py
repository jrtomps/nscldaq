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
# @file   serviceTests.py
# @brief  Test server's abilty to create the pub and req services.
# @author <fox@nscl.msu.edu>


##
#  Note, this only tests the server's ability to register
#  these services with the port manager.  Other functional
#  tests that interact with the server itself will be used
#  to ensure that the sockets are created and are either responding to input
#  (REQ) or transmitting changes (PUB).


import unittest
import os
import os.path
import subprocess
import tempfile
import signal
import time
import getpass
import testBase

from nscldaq.portmanager import PortManager


class ServiceTests(testBase.TestBase):
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
        pass
    ##
    #  We need to:
    #   *  Delete any temporary file we may have created (database).
    #   *  Kill off the server if it's running (non Null self._pid)
    #   *  close any pipe fd's that might have also been created.
    #
    def tearDown(self):
        if self._pid is not None:
            self._process.kill()
        if self._stdout is not None:
            self._stdout.close()
        if os.path.isfile(self._dbName):
            os.unlink(self._dbName)
            

    ##
    #  starting the server with defaults should create
    #  vardb-changes in the port manager.
    def test_defaultpub(self):
        self.startServer(['--database', self._dbName, '--create-ok', 'yes'])
        pm = PortManager.PortManager('localhost')
        time.sleep(2)
        
        ports = pm.find(service='vardb-changes', user=getpass.getuser())
        self.assertEquals(1, len(ports))
        
    ##
    #  starting the sever with defaults should create vardb-request
    
    def test_defaultreq(self):
        self.startServer(['--database', self._dbName, '--create-ok', 'yes'])
        pm = PortManager.PortManager('localhost')
        time.sleep(2)
        
        ports = pm.find(service='vardb-request', user=getpass.getuser())
        self.assertEquals(1, len(ports)) 
    
    ##
    # Starting the server with non standard --publish/--request-service
    # Modifies the set of services created:
    
    def test_nonstandardsvcs(self):
        self.startServer(
            ['--database', self._dbName, '--create-ok', 'yes',
             '--publish-service', 'vardb-pub', '--request-service', 'vardb-req']
        )
        pm = PortManager.PortManager('localhost')
        time.sleep(2)
        
        ports = pm.find(service='vardb-req', user=getpass.getuser())
        self.assertEquals(1, len(ports))
        
        ports = pm.find(service='vardb-pub', user=getpass.getuser())
        self.assertEquals(1, len(ports))
    
if __name__ == '__main__':
    unittest.main()
    
