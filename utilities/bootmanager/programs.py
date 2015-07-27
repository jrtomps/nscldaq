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
# @file   programs.py
# @brief  Class that manages programs.
# @author <fox@nscl.msu.edu>

from nscldaq.bootmanager import ssh
import os


##
# Class to manage a set of programs that are run on other platforms.
class Programs:
    def __init__(self, requri, suburi, client):
        self._reqUri = requri
        self._subUri = suburi
        self._client = client
        self._filesToProgram = dict()
    
    #----------------- Private interfaces --------------------


    ##
    # __getFds
    #   Return a list of the two fds for a program.
    # @param prog - program object.
    # @return list [2] stdin, stderr fds, no order gaurantees.
    #
    def __getFds(self, prog):
        return  [fd for fd in self._filesToProgram.keys()
            if self._filesToProgram[fd] == prog
        ]
    
    ##
    # __getProgramDef
    #     Given a program name returns a dict with:
    #     'host' - host the program runs in.
    #     'path' - Path to the program.
    #     'name' - program name again.
    # @param name - name of the program.
    # @return dict - see above.
    #
    def __getProgramDef(self, name):
        fullDef = self._client.getProgramDefinition(name)
        result  = {
            'host': fullDef['host'], 'path': fullDef['path'], 'name': name,
            'outring': fullDef['outring'], 'inring' : fullDef['inring']
        }
        return result
    
    ##
    # __makeProgeramEnv
    #
    #   Creates the environment definition for a program.  This is a map with:
    #   'REQ_URI' - the request URI.
    #   'SUB_URI' - The subscription URI.
    #   'PROGRAM' - The program name.
    #   'DAQROOT' - Current value
    #   'DAQBIN'  - Current value
    #   'DAQLIB'  - Current Value
    #   'PYTHONPATH' - Current value.
    #   'OUTRING' - Output ring.
    #   'INRING'  - Input ring.
    #
    #  @param progDef - program definition.
    #  @return dict as described above.
    def __makeProgramEnv(self, progDef):
        return {
            'PROGRAM': progDef['name'], 'REQ_URI': self._reqUri,
            'SUB_URI': self._subUri,
            'OUTRING' : progDef['outring'], 'INRING' : progDef['inring'],
            'DAQROOT' : os.getenv('DAQROOT'), 'DAQBIN' : os.getenv('DAQBIN'),
            'DAQLIB' : os.getenv('DAQLIB'), 'PYTHONPATH' : os.getenv('PYTHONPATH')
        }
    
    ##
    # Start a program
    #  @param program - name of the program to start.
    #
    def __startProgram(self, program):
        print('Starting program %s' % program)
        programDef = self.__getProgramDef(program)
        programEnv = self.__makeProgramEnv(programDef)
        host       = programDef['host']
        path       = programDef['path']
        
        # Set the program's state to readying:
        
        self._client.setProgramState(program, 'Readying')
        
        # Create the program and make map entries from its stdout/stderr for it.
        
        
        programObj = ssh.program(host, path, programEnv, program)
        
        stdout = programObj.stdout()
        stderr = programObj.stderr()

        self._filesToProgram[stdout] = programObj
        self._filesToProgram[stderr] = programObj
    
    #----------------- Public interfaces ---------------------
        
    ##
    # start
    #   start all of the programs that are defined.
    #
    def start(self):
        self._filesToProgram = dict()
        programList = self._client.listPrograms()
        for program in programList:
            self.__startProgram(program)
    ##
    # All programs were externally stopped due to system shutdown.
    #
    def stop(self):
        self._filesToProgram = dict()
    
    ##
    # getFiles:
    #
    #  Return the file descriptors for the stdout/stderr for all programs:
    #
    # @return list
    #
    def getFiles(self):
        return self._filesToProgram.keys()
    ##
    # getProgram
    #   Given a file descriptor return the program it belongs to:
    #
    # @param file - the file descriptor.
    # @return ssh.Program object.
    #
    def getProgram(self, file):
        return self._filesToProgram[file]
    ##
    # getPrograms
    #   Returns all of the program objects associated with the program:
    #
    # @return list - list of program objects.
    #
    def getPrograms():
        return list(set(self._filesToProgram.values()))  # set -> unique objects.

    def programExited(self, item):
        programName = item.name()
        print("programs.programExited called")
        fds = self.__getFds(item)
        self._filesToProgram.pop(fds[0])
        self._filesToProgram.pop(fds[1])
        try:
            self._client.setProgramState(name, 'NotReady')  # May already be.
        except:
            pass
        self.stop()
        