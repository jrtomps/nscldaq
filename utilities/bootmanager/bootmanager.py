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
# @file   bootmanager.py
# @brief  Main for boot manager.0
# @author <fox@nscl.msu.edu>

#
#  This is the main program for a boot manager.  Boot manager:
#  * Watches for transitions to Readying  when seen, the boot manager
#    starts all programs.
#  * Watches for transitions to NotReady (global).  If all programs don't transition
#    to NotReady within a timeout, the remaining ones are killed off.
#  * Watches for a program to exit:
#    #  If necessary the program's state is set to NotReady.
#    #  The global state is set to NotReady.
#

import nscldaq.vardb.statemanager
from  nscldaq.programs import ssh
from  nscldaq.programs import programs

import argparse
import time
import select
import socket
import urlparse

import os
import fcntl

#  Global variable that will get the changes caught by processMessages.

changes = []

##
# setFlag
#   Set a file descriptor to non blocking mode.
#
#  @param fd - the file descriptor to set.
#
def setFlag(fd, flag):
    fcntl.fcntl(
        fd, fcntl.F_SETFL,
        fcntl.fcntl(fd, fcntl.F_GETFL) | flag
    )

def clearFlag(fd, flag):
    fcntl.fcntl(
        fd, fcntl.F_SETFL,
        fcntl.fcntl(fd, fcntl.F_GETFL) & (~flag)
    )

##
# processArgs
#  Process the command line parameters or exit on failure.
#
# @return list - first element is the req port, second the sub port.
#
def processArgs():
    parser = argparse.ArgumentParser()
    parser.add_argument('req_uri')
    parser.add_argument('sub_uri')
    
    args = parser.parse_args()
    return [args.req_uri, args.sub_uri]
 
##
# recordTransitions
#  @param client - API object.
#  @param notdict - Dictionary object that describes the change.l
# We only care about global state transitions.
# those get saved in the changes list.
#
def recordTransitions(client, notdict, arg):
    if (notdict['type'] == 'GlobalStateChange') :
        changes.append(notdict)
    

##
# makeAbsoluteUri
#   If the URI's for the variable database server have localhost, they need to have
#   that replaced with our hostname or else the URI can't be propagated properly
#   into the programs:
#
# @param uri - a uri to transform.
# @return string -the transformed URI.
#
def makeAbsoluteUri(uri):
    parsedUri = urlparse.urlparse(uri)
    netloc = parsedUri.netloc
    if netloc == 'localhost':
        netloc = socket.gethostname()
    
    # Put it all back together now:
    
    result = parsedUri.scheme + "://"
    result = result + netloc
    if parsedUri.path != '':
        result = result +  parsedUri.path
    return result


##
# processChanges
#   Deal with changes as described in the heading comment
#
# @param changes - list of dicts describing global and program state
#                  changes.
#
def processChanges(changes):
    for c in changes:
        if c['state'] == 'Readying':
            programs.start()
            client.waitTransition()
        elif c['state'] == 'NotReady':
            print('system shutting down')
            programs.stop()

##
# relayProgramOutput
#
# @param program - name of the program.
# @param data    - data read from the program.
#
def relayProgramOutput(program, data):
    print("%s output: %s" % (program.name(), data))

##
# processProgramInput
#    Called when a program pipe is readable.
# @param readable - list of readable program pipes.
#
def processProgramInput(readable):
    for f in readable:

        try:
            program = programs.getProgram(f)
            setFlag(f, os.O_NONBLOCK)
            line    = f.read()               # program may have exited.
            clearFlag(f, os.O_NONBLOCK)
        except:
            return
        
        if (line == ''):
            print("A program %s exited. Shutting down system" % program.name())
            programs.stop()
            try :
                client.setProgramState(program.name(), 'NotReady')
            except:
                pass
            client.setGlobalState('NotReady')
            break
        else:
            relayProgramOutput(program, line)

#  Initialize states:  Global state is NotReady and any
#  programs that are not in that state get set to that state
#  All of these should kill off any programs that are still live and
#  properly reacting.
#
#  @param client - state manager client.
def initializeStateMachines(client):
    
    #
    #  This try block is because NotReady -> NotReady is not allowed.
    #
    try:
        client.setGlobalState('NotReady')
    except:
        pass
    
    # Set all programs that are not NotReady to NotReady - this will
    # kill off the standalone programs.
    
    
    for program in client.listPrograms():
        currentState = client.getProgramState(program)
        if currentState != 'NotReady':
            client.setProgramState(program, 'NotReady')

# Main entry point:

# Create the state client.
    
result = processArgs()
client = nscldaq.vardb.statemanager.Api(result[0], result[1])

# the programs need a URI without a localhost in it.

reqUri = makeAbsoluteUri(result[0])
subUri = makeAbsoluteUri(result[1])




# Main loop: alternate between processing messages and waiting
# for input from the processes.


programs = programs.Programs(reqUri, subUri, client)
initializeStateMachines(client)

while True :
    time.sleep(1)
    changes = []
    client.processMessages(recordTransitions)
    
    if len(changes) > 0:
        processChanges(changes)

    inWaits = programs.getFiles()
    result = select.select(inWaits, [], [], 1)
    readReady = result[0]
    if len(readReady) > 0:
        processProgramInput(readReady)
