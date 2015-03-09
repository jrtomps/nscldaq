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
import nscldaq.vardb.variable
import nscldaq.vardb.enum
import nscldaq.vardb.statemachine


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
    ##
    # _createStateMap
    #
    #    Given the data field of an SMACHINE request, returns the state transition
    #    map for the state machine the requestor is trying to create.
    #    The data field is a set of pipe separated fields.  Within each
    #    of those fields are subfields that are comma separated.  The first
    #    of those is a state name, the remainder are states reachable from
    #    that state.
    #
    # @param data - The data as described above.
    # @return map - Map containing the data as described.
    #
    def _createStateMap(self, data):
        result = {}
        stateDefs = data.split('|')
        for stateDef in stateDefs:
            transitions = stateDef.split(',')
            state = transitions[0]
            transitions = transitions[1:]
            result[state] = transitions
        return result
            
    
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
            
    ##
    # _declare
    #   Declares a new variable and notifies consumers of the creation.
    #
    # @param path  - full path to the variable.
    # @param dType - Data type name for the variable (e.g. 'integer')
    # @param value - If not an empty string, the new value for the item.
    #
    def _declare(self, path, dType, value):
        try:
            if value == '':              # default initial value
                nscldaq.vardb.variable.create(self._database, None, path, dType)
                var = nscldaq.vardb.variable.Variable(self._database, path=path)
                value = var.get()        # Be able to return the actual value.
            else:                        # default supplied:
                nscldaq.vardb.variable.create(
                    self._database, None, path, dType, value
                )
            self._req.send('OK:')
            self._notifyDecl(path, dType, value)
        except nscldaq.vardb.variable.error as e:
            self._req.send('FAIL:%s' % e)
    
    ##
    # _set
    #   Set a new value for a variable.
    #
    # @param path  - Path to that variable.
    # @param value - New requested value.
    #
    def _set(self, path, value):
        try:
            var = nscldaq.vardb.variable.Variable(self._database, path=path)
            var.set(value)
            self._req.send('OK:')
            self._notifySet(path, value)
        except nscldaq.vardb.variable.error as e:
            self._req.send('FAIL:%s' % e)
     
    ##
    # _createEnum
    #
    #  Create a new enumerated type.
    #
    # @param typeName - Name of the new data atype.
    # @param values   - Iterable of values.
    #
    def _createEnum(self, typeName, values):
        try:
            nscldaq.vardb.enum.create(self._database, typeName, values)
            self._req.send('OK:')
            self._notifyNewEnum(typeName)
        except nscldaq.vardb.enum.error as e:
            self._req.send('FAIL:%s' % e)
            
    ##
    # _createStateMachine
    #
    #   Creates a new statemachine data type.
    #
    # @param typeName     - name of the new type.
    # @param transitionMap- Transition map as defined by the python statemachine
    #                       bindings.
    #
    def _createStateMachine(self, typeName, transitionMap):
        try:
            nscldaq.vardb.statemachine.create(self._database, typeName, transitionMap)
            self._req.send('OK:')
            self._notifyStateMachine(typeName)
        except nscldaq.vardb.statemachine.error as e:
            self._req.send('FAIL:%s' % (e))
    
    ##
    # _get
    #   Return the value of a variable to a client:
    #
    # @param path - path to the variable.
    #
    def _get(self, path):
        try:
            var = nscldaq.vardb.variable.Variable(self._database, path=path)
            self._req.send('OK:%s' % var.get())
        except nscldaq.vardb.variable.error as e:
            self._req.send('FAIL:%s' % (e,))
    
    #--------------------------- publication methods ------------------

    ##
    # _notifyStateMachine
    #
    # @param typeName - new type name.
    #
    def _notifyStateMachine(self, typeName):
        self._publishMessage(typeName, 'TYPE', 'statemachine')

    ##
    # _notifyNewEnum
    #
    #  Notify the existence of a new enum:
    #
    # @param typeName - name of new type
    #
    def _notifyNewEnum(self, typeName):
        self._publishMessage(typeName, 'TYPE', 'enum')

    ##
    # _notifySet
    #    Publish an ASSIGN notification.
    #
    # @param path - path to the variable that was modified.
    # @param value- New value.
    #
    def _notifySet(self, path, value):
        self._publishMessage(path, 'ASSIGN', value)

    ##
    # _notifyDecl
    #   Publish a NEWVAR notification.
    #
    # @param path - path to the varaible.
    # @param dType - Variable data type.
    # @param value - actual initial value of the variable.
    def _notifyDecl(self, path, dType, value):
        data = '|'.join((dType, value))
        self._publishMessage(path, 'NEWVAR', data)

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
        elif command == 'DECL':
            dataParts = data.split('|')
            dType = dataParts[0]
            value = dataParts[1]
            self._declare(path, dType, value)
        elif command == 'SET':
            self._set(path, data)
        elif command == 'ENUM':
            self._createEnum(path, data.split('|'))   # type, values.
        elif command == 'SMACHINE':
            type = path
            map  = self._createStateMap(data)
            self._createStateMachine(type, map)
        elif command == 'GET':
            self._get(path)
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

