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
# @file   Programs.py
# @brief  Program management for boot application.
# @author <fox@nscl.msu.edu>

import os
import zmq
from nscldaq.expconfiguration import project
from nscldaq.boot             import ssh, config

_RunningPrograms = list()     # List of active program objects.

##
# _listPrograms
#   List the programs a given database file.
#
# @param database - path to the database file.
# @return         - iterable container from project.Programs.list()
#
def _listPrograms(database):
    dbase  =   project.Project(database)
    dbase.open()
    progs  =   project.Programs(dbase)
    return progs.list()
    
##
# @class Program
#
# Private class for maintaining a program.
#
#
class Program:
    ##
    #  constructor
    #    Starts up the program via a persistent shell:
    # @param monitor - The state monitor (we register input on stdout,stderr for calbacks)
    # @param host    - Host on which the command runs.
    # @param command - The base command.
    # @param args    - The command arguments
    # @param workingDir - The working directory in which the program runs.
    # @param env     - Dict defining additional env variables to define.
    #
    def __init__(self, monitor, host, command, args, workingDir, env):
        self._ssh           = ssh.shell(host)
        self._expectingExit = False            # This is true when we ask for an exit.
        self._monitor       = monitor          # so we can unregister.
        self._setupEnv(env)
        self._setWd(workingDir)
        self._startProgram(command, args)
        self._registerCallbacks(self._ssh)
        _RunningPrograms.append(self)           # Self registration.
    
    ##
    #  destructor
    #    need to unregister our callbacks from the poller.
    #    Note that this must be called at garbage collection time in response
    #    to the object being removed from _RunningPrograms.
    #
    def __del__(self):
        stdout = self._ssh.stdout()
        stderr = self._ssh.stderr()
        poller = self._monitor.poller
        poller.unregister(stdout)
        poller.unregister(stderr)
        
    # class.Program private utility methods:
    
    ##
    # _setupEnv
    #   Set up the program environment.  This is done by
    #   *  Invoking . $DAQBIN/daqsetup in the remote system.
    #   *  Doing export's for each item in the env map.
    #
    # @param env  - Map of envvarName -> value pairs.
    #
    def _setupEnv(self, env):
        self._ssh.writeLine('. %s/daqsetup' % (os.getenv('DAQBIN')))
        for var in env.keys():
            self._ssh.writeLine('export %s=%s' % (var, env[var]))
        self._ssh.writeLine('printenv')
    
    ##
    # _setWd
    #
    #   Set the working directory (just a cd <thing>)
    #
    # @param wd - the desired working directory
    #
    def _setWd(self, wd):
        self._ssh.writeLine('cd %s' % (wd))
    ##
    # _startProgram
    #   starts the actual target program.
    #
    # @param program - path to the program (argv[0]).
    # @param args    - ordered list of program arguments.
    #
    def _startProgram(self, program, args):
        command = '%s %s' % (program, ' '.join(map(lambda x: "'" + x + "'", args)))
        self._ssh.writeLine(command)
    
    ##
    # _registerCallbacks
    #   Set readable callbacks on the shell's stdout and stderr.
    #
    # @param shell - The ssh shell whose stdout/stderr we want to monitor.
    #
    def _registerCallbacks(self, shell):
        poller = self._monitor.poller
        poller.register(shell.stdout(), zmq.POLLIN, self._programOutput)
        poller.register(shell.stderr(), zmq.POLLIN, self._programOutput)
        
    ##
    # _programOutput
    #   Called by the poller/event loop when our program's stdin/stderr
    #   is readable
    #   * Try to read a line - If we can, discard it.
    #   * If reading a line shows that the file descriptor is at EOF (was closed)
    #     then force a fail transition, and unregister us from the program list
    #     because we are alread gone.    
    #
    # @param poller - The poller that called us.
    # @param f      - The file descriptor that is now readable.
    # @param mask   - Mask of events (zmq.POLLIN).
    #
    # @note - on EOF we also unregister the file descriptor to ensure our events
    #         don't starve other event sources.
    # @note - If self._expectingExit is true we don't attempt the state
    #         transition but do unregister as this means we got the because we were
    #         asked to end the program.
    #
    def _programOutput(self, poller, f, mask) :
        l = f.readline()
        print("Output %s" % (l))
        if len(l) == 0:
            print("zero length")
            poller.unregister(f)
            if not self._expectingExit:
                self._monitor.requestTransition('FAIL')
                try:
                    _RunningPrograms.remove(self)
                except:
                    pass              # Might already be gone.
    
    # class.Programs - public entries.
    
    ##
    # exit
    #   sends the 'exit' string to the program in hopes that will kill it off.
    #
    def exit(self):
        self._expectingExit = True
        self._ssh.writeLine('exit')
    ##
    #  intr
    #    Send an intr signal to the program (control-C)
    #
    def intr(self):
        self._expectingExit = True
        self._ssh.writeLine('\x03')

#-----------------------------------------------------------------------------
# Public entries.

##
# startPrograms
#    For each program in the specified database:
#    *  Starts is up via an ssh using pipes for in/out/error.
#    *  Registers Records the existence of the program.
#    *  Registers stdout and stderr to be monitored via the state monitor's
#       poller to fire a procedure when the fd is readable.
#
def startPrograms(monitor, database):

    print('Starting programs in %s ' % (database))
    #
    # The env we're going to pass tells the program how to be a client of the
    # statemanager, Note that $DAQBIN/daqsetup will be run in the shell as well.
    
    env = {                                                 \
        'TRANSITION_REQUEST_URI'      : config.remoteRequestUri,   \
        'TRANSITION_SUBSCRIPTION_URI' : config.remoteStateUri      \
    }
    
    programListing = _listPrograms(database)
    for program in programListing:
        path = program['path']
        wd   = program['working_dir']
        host = program['host_name']
        args = program['args']
        p    = Program(monitor, host, path, args, wd, env)
