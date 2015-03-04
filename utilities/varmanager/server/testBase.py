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
# @file   testBase.py
# @brief  Base class for tests in this directory
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
from nscldaq.portmanager import PortManager

##
#   Base class for tests in this directory...provides common utility methods:
#

class TestBase(unittest.TestCase):
    ##
    # Start the server with an appropriate set of command line args
    # for the test
    #
    # @param args - iterable of command line arguments.
    # @return POpen object 
    # @note
    #    *  self._pid is filled in with the process pid so tearDown can stop it.
    #    *  self._stdout is filled in with the fd as well so tearDown can close it.
    #    *  self._server contains the path to the server program.
    #
    def startServer(self, args):
        fullargs =[self._server] + args
        process = subprocess.Popen(
            fullargs, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
            close_fds=True
        )
        self._pid     = process.pid
        self._stdout  = process.stdout
        self._process = process
        time.sleep(2)                 # Let server establish services.
        return process
    
    ##
    # Create a 0mq client for a service
    #
    # @param type - type of socket created (e.g. zmq.REQ
    # @param service - name of service to connect to.
    # @return socket object.
    # @note since we're running tests, the service is assumed to be in the
    #       localhost
    #
    def createClient(self, type, service):
        pm   = PortManager.PortManager('localhost')
        portList = pm.find(service=service, user=getpass.getuser())
        self.assertEqual(len(portList), 1)     # there can be only one.
        
        uri = 'tcp://localhost:%d' % portList[0]['port']
 
        socket = self._zmq.socket(type)
        socket.connect(uri)
        return socket
        
        
        
    ##
    # setup the zeromq stuff
    #  *  Create a contex store it in self._zmq
    #  *  Create sockets attached to the request and publish ports (localhost).
    #      store those in self._req and self._sub
    
    def setup0mq(self):

        self._zmq = zmq.Context.instance()
        self._req = self.createClient(zmq.REQ, self._requestService)
        self._sub = self.createClient(zmq.SUB, self._pubService)
        self._sub.setsockopt(zmq.SUBSCRIBE, '')
        
    #
    #  Gets the lines of data from stdout of a process as an array
    #
    def getLines(self, process):
        data = process.communicate()
        outputLines = data[0].split("\n")
        return outputLines
    
    def markExited(self):
        self._pid = None
        self._stdout = None      

