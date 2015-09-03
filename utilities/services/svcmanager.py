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
# @file   svcmanager.py
# @brief  Manage system services/servers based on db.
# @author <fox@nscl.msu.edu>


##
# Usage:
#    python svcmanager dbFile
#
#  dbName - the path to the database file.
#
#  Accepts the following commands at the prompt:
#    *  start  - Starts the services defined by the database.
#    *  stop   - stops the services that are currently being run.
#    *  exit   - Exit the program.
import sys
import os
import select
import fcntl

from nscldaq.programs import ssh
from nscldaq.vardb import services

#
# Module level definitions:

active = False                     # Not active.
uri    = ''
dbPath = ''
programs = list()
##
# setFlag
#   Set a file descriptor to non blocking mode.
#
#  @param fd - the file descriptor to set.
#  @param flag - The flag to set.
#
def setFlag(fd, flag):
    fcntl.fcntl(
        fd, fcntl.F_SETFL,
        fcntl.fcntl(fd, fcntl.F_GETFL) | flag
    )
##
# clearFlag
#    Clear an IOCTL flag from a file descriptors flag set.
#
# @param fd   - the file descriptor.
# @param flag - The flag to clear.
#
def clearFlag(fd, flag):
    fcntl.fcntl(
        fd, fcntl.F_SETFL,
        fcntl.fcntl(fd, fcntl.F_GETFL) & (~flag)
    )

##
# usage
#
#  outupts the program usage to setderr then exits.
#
def usage() :
    print >> sys.stderr, "Usage:"
    print >> sys.stderr, "   svcmanager dbFile"
    print >> sys.stderr, "Where:"
    print >> sys.stderr, "   dbFile is the database file path."
    sys.exit(os.EX_USAGE)

##
# getDB
#   Return the services object for the requested URI
#
# @param a - the program argv vector.
def getDB(a):
    global dbPath
    global uri
    
    if len(a) != 2:
        usage()
    
    dbPath = os.path.realpath(a[1])
    uri = 'file://%s' % dbPath
    db =  services.Api(uri)
    if not db.exists():
        print >> sys.stderr, "The database has no services definition"
        sys.exit(os.EX_DATAERR)
    
    return db

##
# startProgram
#    Start a single program.
#
# @param name    - name of the program.
# @param command - Command to execute in the program.
# @param host    - Host in which the program is to run.
#
# @return list - Read sides of the stdout and stderr pipes for the program.
#
# @note several environment variables will be created for the process:
#       * PROGRAM_NAME - name
#       * HOST         - Host in which the program is running.
#       * DB_URI       - URI (request) of the database;
#                        file: equivalent of DB_PATH.
#       * DB_PATH      - Path to database file.
#       * PYTHONPATH  - value of our PYTHONPATH
#       * DAQROOT     - Value of our daqroot.
#       * DAQBIN      - value of our daqbin
#       * DAQLIB      - Value of our daqlib.
#
def startProgram(name, command, host):
    global programs
    env = {
        'PROGRAM_NAME': name, 'HOST': host, 'DB_PATH': dbPath, 'DB_URI': uri,
        'PYTHONPATH' : os.getenv('PYTHONPATH'), 'DAQROOT' : os.getenv('DAQROOT'),
        'DAQBIN'      : os.getenv('DAQBIN'),      'DAQLIB'  : os.getenv('DAQLIB')
    }
    program = ssh.program(host, command, env, name)
    programs.append(program)
    return (program.stdout(), program.stderr())


##
# startPrograms
#
#   Starts all the system programs.
#
# @param db - The services API which will give us the paths/hosts.
# @return fds - tuple of fds that have the stdout/stderr of our programs.
#
def startPrograms(db):
    global active
    programDefs = db.list()
    fds = list()
    for name in programDefs.keys():
        program = programDefs[name]
        command = program[0]
        host    = program[1]
        progFds = startProgram(name, command, host)
        fds.extend(progFds)
    active = True
    return tuple(fds)


##
# stopPrograms
#   Stops all programs by sending a control-C to any that have a live
#   stdin.  Once that's done, all files get closed.
#   The program list is also emptied.
#
def stopPrograms():
    global programs
    global active
    for program in programs:
        if not program.stdin().closed:
            #
            #  Sometimes stdin stays open so encapsulate:
            #
            try :
                program.intr()
            except:
                pass
            if not program.stdin().closed :
                program.stdin().close()
            if not program.stdout().closed:
                program.stdout().close()
            if not program.stderr().closed:
                program.stderr().close()
    programs = list()
    active = False

##
# processCommand
#    Process a command from the operator.
#    This is really a command dispatcher.
#
# @param db - The services APi object that lets us figure out which services
#             are configured.
# @param fds - The current set of file descriptors.
# @return tuple - new list of file descripter to be senstive to.
# @note - If the command is exit, we won't return.
# @note - The processing here assumes that stdin is a line mode device.
#         with respect to notifying select().
#
def processCommand(db, fds):
    global active
    cmd = file.readline(sys.stdin)

    if cmd == 'start\n':
        if not active :
            newFds = startPrograms(db)
            newFdSet = set(fds) | set(newFds);   # Merge in the new fds.
            return tuple(newFdSet)
        else:
            print("Programs are already active!")
            return fds
    elif cmd == 'stop\n':
        if active:
            stopPrograms()
        else:
            print("Programs are not active")
        return fds
        
    elif cmd == 'exit\n':
        if active:
            stopPrograms()        
        exit(os.EX_OK)
    else:
        print("Unrecognized command: %s" % cmd)
        return fds
    

##
# processProgramInput
#
#   Handles input ready from a program.
#   - Switch the fd -> nonblocking
#   - read what we can
#   - Switch the fd back to blocking.
#   - output what we got.
#
# Edge case:
#    If the fd is at eof, then close it and
#    return False indicating that.
#
# @param  fd      - 
# @return Boolean - True indicates the fd is still live, False that it's been
#                   closed - which is kinda bad for a service/system program
#                   unless we expect that.
# 
def processProgramInput(fd):
    setFlag(fd, os.O_NONBLOCK)
    try:
        data = fd.read()
    except:
        return False     # File might have already been closed.

    if len(data) == 0:
        fd.close()
        return False

    clearFlag(fd, os.O_NONBLOCK)
    sys.stdout.write(data)    
    
    return True

##
# mainLoop
#    Selects the existing file descriptors and processes input.
#    os.stdin is special since that's where commands come from .
#
# @param db - Services API object.
#
# TODO: just keep fds as a set rather than converting back and forth.
# TODO: Remove programs whose files died.
#
def mainLoop(db) :
    fds = (sys.stdin,)
    
    removeFds = set()
    while True:
        readyfds = select.select(fds, (), ())[0]
        for fd in readyfds:
            if fd == sys.stdin:
                fds = processCommand(db, fds)
            else:
                keep = processProgramInput(fd)
        
        # Trim the fds in removing closed files.  Turns out that some
        # External forces can close files and using the idea that _I_ closed
        # the file in processProgramInput is not reliable.  Hence the code below.
        
        newfds = list()
        for fd in fds:
            if not fd.closed:
                newfds.append(fd)
        
        fds = newfds
        
##
#  Entry point:
#

def main():
    # Get the db and from that the list of program definitions.
    
    db = getDB(sys.argv)
    mainLoop(db)    
    

if __name__ == '__main__':
    main()
