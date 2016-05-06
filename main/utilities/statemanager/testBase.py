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
        return process
    ##
    # getport
    #   Given figure out the port that corresponds to a service with a 30 second timeout:
    #
    def getport(self, service):
        pm   = PortManager.PortManager('localhost')
        waited = 0
        
        # Give the subprocess up to 30 secondsto publish its service.
        
        while waited < 30:    
            portList = pm.find(service=service, user=getpass.getuser())
            if len(portList) == 0:
               time.sleep(.1)
            else:
                break
            waited = waited + 1
        
        self.assertEqual(len(portList), 1)     # there can be only one.
        return portList[0]['port']
    
    # waitPortGone
    #   Wait for a port to no longer be advertised
    # @param service - service being watched
    #
    def waitPortGone(self, service):
        pm   = PortManager.PortManager('localhost')
        waited = 0
        
        # Give the subprocess up to 30 secondsto publish its service.
        
        while waited < 30:
            portList = pm.find(service=service, user=getpass.getuser())
            if len(portList) > 0:
                time.sleep(.1)
            else:
                break
            waited = waited+1
        self.assertEqual(len(portList), 0)
        
    
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
        port = self.getport(service)
        
        uri = 'tcp://localhost:%d' % port
 
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
        
    def setdb(self,db):
        self._db = db
        
    def mkdir(self, path):
        self._req.send('MKDIR:%s:' % path)
        self._req.recv()
        self._sub.recv()
        
    def makevar(self, path, typeName, value):
        self._req.send('DECL:%s:%s|%s' % (path, typeName, value))
        self._req.recv()
        self._sub.recv()
        
        
    ##
    # _analyzeVarlistReply
    #   Takes a reply message and breaks it up into a list of dicts
    #   sorted by variable name.  dicts have the keys
    #   'name', 'type' and 'value' with obvious meanings.
    #
    # @param reply - the full reply from the server ("OK:variable descriptions")
    def _analyzeVarlistReply(self, reply):
        variableDescriptions = reply.split(':')[1].split('|')
        result = list()
        if (len(variableDescriptions) > 1):   # 1 because an empty string gives 1 on the split.
            for i in range(0, len(variableDescriptions), 3):
                item = {
                    'name': variableDescriptions[i],
                    'type': variableDescriptions[i+1],
                    'value': variableDescriptions[i+2]
                }
                result.append(item)
            

        # Now sort using a custom sort function:
        #  See e.g. http://www.php2python.com/wiki/function.strcmp/ for the comparison
        #  function....this is python3 clean while cmp is not.
        
        return sorted(
            result,
            lambda x,y: ((x['name'] > y['name']) - (x['name'] > y['name'])))
    
