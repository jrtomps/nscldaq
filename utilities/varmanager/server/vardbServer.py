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
# @file   vardbServer.py
# @brief  Contains the main part of the variable database server.
# @author Ron Fox <fox@nscl.msu.edu>

##
# This file contains the main parts of the variable database server.
#
# \verbatim
#   Usage:
#     python vardbServer.py [--publish-service service-name] \
#                           [--request-service service-name] \
#                           --database=path-to-database-file \
#                           [--create-ok=yes|no]
# \verbatim
#
# Where the switches are as follows:
#
#  *  --publish-service - Overrides the default publish service name
#                         of "vardb-changes".  This is the service
#                         name registered with the local port manager for
#                         the 0MQ publish port that notifies clients of changes
#                         to the database.
#  *  --request-service - Overrides the default request service name of
#                          "vardb-request".  This is the service name registered
#                          with the top level server for the 0MQ request port
#                          on which clients request changes to the database.
#  *  --database         - Mandatory.  Specifies the path to the variable
#                          database this server controls.  If the database
#                          file specified, the action depends on the state of the
#                          --create-ok flag below.  Regardless of the state of
#                          this flag, if the database exists it is opened without
#                          modification.  If the file is not a valid database,
#                          the server will exit with an error.
#  * --create-ok         - The value of this flag must be either 'yes' or 'no'.
#                          If the value is 'no', and the file specified with
#                          --database does not exist, the server exits with an
#                          error.
#                          If the value is 'yes', ande the --database does not
#                          exist, a new, empty one is created.
#
# The message definition for both the REQ/REP and PUB/SUB are described in the
# NSCLDAQ redmine wiki currently at:
#  https://swdev-redmine.nscl.msu.edu/projects/sfnscldaq/wiki/Variable_Manager_program#Server-protocol
#

import argparse
import sys
import os.path

import nscldaq.vardb.vardb
import nscldaq.vardb.dirtree


import zmq
from nscldaq.portmanager import PortManager



class databaseServer():
    #
    #  Construction
    #   Set the default variable values:
    def __init__(self):
        self._publishService = 'vardb-changes'
        self._requestService = 'vardb-request'
        self._databaseFile   = None
        self._createOk       = False
        self._pm             = None
        
    
    ##
    # _parseArguments
    #   Process the command line parameters into the
    #   Variables created by the constructor:
    def _parseArguments(self):
        parser = argparse.ArgumentParser(
            prog='vardbServer', description='Variable database server'
        )
        parser.add_argument('-p', '--publish-service', default=self._publishService)
        parser.add_argument('-r', '--request-service', default=self._requestService)
        parser.add_argument('-f', '--database', default=None)
        parser.add_argument('-c', '--create-ok', default='no')
        
        result = parser.parse_args()
        return result
    ##
    # _createService
    #    Create a zeromq service an return the socket object.
    #    * The service name is regisetered with the port manager
    #    * The socket returned by the port manager is used to create a
    #      zmq socket.
    #
    #   In this way the service 
    #
    # @param zmqType     - is the socket type (e.g. zmq.REQ).
    # @param serviceName - is the name of the service.
    # @return zmq.Socket - Socket that is listening for data on the service.
    #
    def _createService(self, zmqType, serviceName):
        
        # Allocate the port.
        
        if self._pm is None :  
            self._pm   = PortManager.PortManager('localhost')
            
        port = self._pm.getPort(serviceName)
        #  Create the ZMQ socket:
        
        socket = self._zmq.socket(zmqType)
        bindstring = 'tcp://*:%d' % (port)
        socket.bind(bindstring)
        
        return socket
    #--------------------------- Action handlers ------------------------
    
    ##
    # _mkdir
    #   Create a new directory.
    #
    # @param path - path of the directory to create
    #
    def _mkdir(self, path):
        dir = nscldaq.vardb.dirtree.DirTree(self._database)
        try:
            dir.mkdir(path)
            self._req.send('OK:')
            self._notifyMkdir(path)
        except nscldaq.vardb.dirtree.error as e:
            self._req.send('FAIL:%s' % (e))
    
    ##
    # _rmdir
    #   Attempt to remove a directory given its path.
    #
    # @param path - path of the directory to remove
    #
    def _rmdir(self, path):
        dir = nscldaq.vardb.dirtree.DirTree(self._database)
        try:
            dir.rmdir(path)
            self._req.send('OK:')
            self._notifyRmdir(path)
        except nscldaq.vardb.dirtree.error as e:
            self._req.send('FAIL:%s' % (e))
    
    #--------------------------- publication methods ------------------

    ##
    # _notifyMKdir
    #   Publish a MKDIR notification
    # @param path - full path to the new directory
    #
    def _notifyMkdir(self, path):
        pathParts = nscldaq.vardb.dirtree.parsePath(path)
        if (len(pathParts) == 1):
            parent = '/'
            child = pathParts[0]
        else :
            parent = '/' + '/'.join(pathParts[:-1])
            child  = pathParts[-1]
        self._publishMessage(parent, 'MKDIR', child)
    
    
    ##
    # _notifyRmDir
    #
    #   Notify the removal of a directory
    #
    # @param path - full path to the removed directory.
    #
    def _notifyRmdir(self, path):
        pathParts = nscldaq.vardb.dirtree.parsePath(path)
        if (len(pathParts) == 1):
            parent = '/'
            child = pathParts[0]
        else :
            parent = '/' + '/'.join(pathParts[:-1])
            child  = pathParts[-1]
        self._publishMessage(parent, 'RMDIR', child) 

    #-------------------------- other utilities ------------------------
     
    

    ##
    # _publishMessage
    #   Given a message write it to the publish socket
    # @param path      - The path modified.
    # @param operation - The operation performed.
    # @param data      - Associated data
    #
    def _publishMessage(self, path, operation, data):
        message = ':'.join([path, operation, data])
        self._pub.send(message)
    

    
    ##
    # _processRequest
    #   Process a request and provide a reply. On success, the result is also
    #   published to subscribers.
    #
    # @param msg - Message received on the req port.
    #
    def _processRequest(self, msg):
        msgParts = msg.split(':')
        command = msgParts[0]
        path    = msgParts[1]
        data    = msgParts[2]
        if command == 'MKDIR':
            self._mkdir(path)
        elif command == 'RMDIR':
            self._rmdir(path)
        else:
            self._req.send('FAIL:Unrecognized operation code %s' % (command))
        
    
    #---------------------------------------------------------------------------
    # Public methods.
        
    ##
    #  run
    #   Entry point for the database server.
    #
    #  *  Process command line parameters into local variables.
    #  *  Get my port manager service ports.
    #  *  set up my zmq ports.
    #  *  Accept and process requests.
    #
    def run(self):
        parsedArgs = self._parseArguments()

        # Require the database parameter:
        
        if parsedArgs.database is None :
            sys.stderr.write("No database file was given\n")
            sys.exit(-1)
            
        if not os.path.isfile(parsedArgs.database):
            if parsedArgs.create_ok == 'yes':
                nscldaq.vardb.vardb.create(parsedArgs.database)
            else:
                sys.stderr.write("No such database file\n")
                sys.exit(-1)
                
        try:
            db = nscldaq.vardb.vardb.VarDb(parsedArgs.database)
            self._database = db
        except nscldaq.vardb.vardb.error as error:
            sys.stderr.write('--database is not a valid variable database file')
            sys.exit(-1)
        
        #  Initialize zeromq
        
        self._zmq = zmq.Context.instance()
        #  Create the Reply socket
        
        self._req = self._createService(zmq.REP, parsedArgs.request_service)
        
        #  Create the PUB socket.
        
        self._pub = self._createService(zmq.PUB, parsedArgs.publish_service)
        
        # In the main loop, we just get messages from the request port and
        # process them.
        #
            
        while True:
            msg = self._req.recv()
            self._processRequest(msg)

if __name__ == '__main__':
    # Die if the paraent dies:

    
    # Start up our server class/object:
    
    server = databaseServer()
    server.run()

