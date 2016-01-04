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
# @file   eventbuilders.py
# @brief  Classes to manage event builder programs.
# @author <fox@nscl.msu.edu>

from nscldaq.programs import ssh
from nscldaq.vardb    import VardbEvb
import os
from os import path
import getpass
import time

##
# @class DataSource
#   Encapsulates a single data source program.  Data source programs have
#   existence only in the presence of event builders as their purpose is to
#   transmit event fragments to an event builder.
#
class DataSource(object):
    ##
    #  __init__
    #   @param evbDict  - Dict that describes the event builder
    #                     (from VardbEvb.evbInfo())
    #   @param infoDict - Dict that describes the data source
    #                     (from VardbEvb.dsInfo()).
    #   @param api      - Event builder Api object.
    def __init__(self, evbDict, infoDict, api):
        self._evbInfo = evbDict
        self._srcInfo = infoDict
        self._api     = api
        self._source  = None
        
    
    ##
    # _createEnv
    #   Create the environment dict for the data source:
    #
    #  INHERITED VARS:
    #    -   DAQROOT
    #    -   DAQBIN
    #    -   DAQLIB
    #    -   PYTHONPATH
    #
    #  OTHER VARS:
    #    -  REQURI  - Database Request URI
    #    -  EVBNAME - Event builder name (in database)
    #    -  DSNAME  - Data source name (in database).
    #    -  TCLLIBPATH = Libraries directory root for Tcl packages.
    #
    # @note if the inherited vars don't exist yet (e.g. daqsetup.bash has not
    #       been sourced), /usr/opt/daq/current is used as DAQROOT and the
    #       other vars computed relative to that for an NSCLDAQ standard
    #       installation.
    #
    # @return dict - keys are env names values are their values.
    #
    def _createEnv(self):
        result = {}
        
        #  Inherited values:
        
        result['DAQROOT'] = os.getenv('DAQROOT', '/usr/opt/daq/current')
        result['DAQBIN']  = os.getenv(
            'DAQBIN', path.join(result['DAQROOT'], 'bin')
        )
        result['DAQLIB']  = os.getenv(
            'DAQLIB', path.join(result['DAQROOT'], 'lib')
        )
        result['PYTHONPATH'] = os.getenv(
            'PYTHONPATH', path.join(result['DAQROOT'], 'pythonLibs:')
        )
        #  Other vars:
        
        result['REQURI']  = self._api.uri
        result['EVBNAME'] = self._evbInfo['name']
        result['DSNAME']  = self._srcInfo['name']
        result['TCLLIBPATH'] = path.join(result['DAQROOT'], 'TclLibs')
        
        return result
    
    
    ##
    # _computeEvbServiceName
    #   Using the EVB prefix, suffix and our username, compute the event builder
    #   service name as prefix:user:suffix
    #
    def _computeEvbServiceName(self):
       
        svc = self._evbInfo['serviceSuffix']
        
        return svc    
    ##
    #  _createDsCommand
    #     Create the command line that starts the data source
    #
    #  @return string - command line to start the data source with the appropriate
    #                   options.
    #
    def _createDsCommand(self) :
        
        #  Fixed part of the command.
        
        cmd = self._srcInfo['path']
        cmd = cmd + ' --evbhost=' + self._evbInfo['host']
        cmd = cmd + ' --evbport=managed'
        cmd = cmd + ' --evbname=' + self._computeEvbServiceName()
        cmd = cmd + ' --info="'    + self._srcInfo['info'] + '"'
        cmd = cmd + ' --ids='     + ','.join(str(item) for item in self._srcInfo['ids'])
        cmd = cmd + ' --ring='     + self._srcInfo['ring']
        
        # Remaining options depend on the bodyheaders setting:
        
        if self._srcInfo['bodyheaders'] :
            cmd = cmd + ' --expectbodyheaders'
        else:
            cmd = cmd + ' --default-id=' + str(self._srcInfo['defaultId'])
            cmd = cmd + ' --timestampextractor=' + self._srcInfo['tsextractor']
        cmd = cmd + ">/dev/null "
            
        return cmd
        
    #--------------------------------------------------------------------------
    #   Public methods
    #
    
    ##
    # start
    #   Start up the data source connected to our event builder.
    #
    #
    def start(self):
        envDict      = self._createEnv()
        dsourceCmd   = self._createDsCommand()
        print("Data source command: " + dsourceCmd)
        self._source = ssh.program(
            self._srcInfo['host'], dsourceCmd, envDict, self._srcInfo['name']
        )
        print('data source started: ' + self.name())
    ##
    # stop
    #  stop our data source.
    #
    def stop(self):
        if self._source is not None:
            try :
                self._source.intr()
                self._source.writeLine('exit')
            except:
                pass
            self._source = None                     # Allow garbage collection.
    ##
    #  getFiles
    #    Get the file descriptors for the data source.
    #
    # @return list - not in any specific order.
    def getFiles(self):
        if self._source is not None:
            return [self._source.stdout(), self._source.stderr()]
        else:
            return []
    
    ##
    # name
    #   @return string - 'Evb: name Source: srcname'
    #
    def name(self):
        return 'Evb: ' + self._evbInfo['name'] + ' Source: ' + self._srcInfo['name']
    
    
##
# @class EventBuilder
#    Encapsulates an event builder and the data sources that transmit data to
#    it.
#
class EventBuilder(object):
    ##
    # __init__
    #   @param evbDict - Dict describing the event builder being constructed.
    #                    (from e.g. VardbEvb.evbInfo)
    #   @param api     - VardbEvb.VardbEvb object connected to the database from
    #                    which evbDict came.
    #
    def __init__(self, evbDict, api):
        self._api       = api
        self._info      = evbDict
        self._dSources  = self._createDataSources()
        self._eventBuilder = None


    #-------------------------------------------------------------------------
    # private methods
    #
    
    ##
    # _createDataSources
    #    Create the infrastructure that describes the data sources we have
    #
    def _createDataSources(self):
        dataSources = self._api.listDataSources(self._info['name'])
        result      = []
        for dsInfo in dataSources:
            print('Creating data source' + dsInfo['name'])
            result.append(DataSource(self._info, dsInfo, self._api))
        
        return result
    
    ##
    # _defineEventBuilderEnv
    #   Returns a dict whose keys are environment variables and whose
    #   values are the values of those vars.  We're going to define:
    #
    # Copies from our env:
    #   -  DAQROOT
    #   -  DAQBIN
    #   -  DAQLIB
    #   -  PYTHONPATH
    #
    #  Other variables:
    #    -  EVBNAME - name of the event builder.
    #    -  REQURI  - URI for the database request port.
    #    -  TCLLIBPATH - $DAQROOT/TclLibs
    #
    # @return - environment dict.
    #
    def _defineEventBuilderEnv(self):
        result = {}
        
        #  Copies from our environment
        
        result['DAQROOT'] = os.getenv('DAQROOT', '/usr/opt/daq/current')
        result['DAQBIN']  = os.getenv(
            'DAQBIN', path.join(result['DAQROOT'], 'bin')
        )
        result['DAQLIB']  = os.getenv(
            'DAQLIB', path.join(result['DAQROOT'], 'lib')
        )
        result['PYTHONPATH']  = os.getenv(
            'PYTHONPATH', path.join(result['DAQROOT'], 'pythonLibs:')
        )
        #  New variables:
        
        result['EVNAME'] = self._info['name']
        result['REQURI'] = self._api.uri
        result['TCLLIBPATH'] = path.join(result['DAQROOT'], 'TclLibs')
        
        return result
    
    ##
    # _makeGlomCmd
    #    Create the glom command from the event builder definition.
    #    @note we can assume that $DAQBIN is a valid env. name when the command
    #          is executed.
    #   @return string - the command to execute.
    #
    def _makeGlomCmd(self ):
        
        # Construct the base command.
        
        bindir = os.getenv('DAQBIN', '/usr/opt/daq/current/bin')
        glomcmd = path.join(bindir, 'glom')
        glomcmd = glomcmd + ' '
        
        #  Construct the --dt switch:
        
        glomcmd = glomcmd + '--dt=' + str(self._info['coincidenceInterval'])
        
        #  Need a --nobuild?
        
        if not self._info['build'] :
            glomcmd = glomcmd + ' --nobuild'
        else:
            # Timestamp policy and source id are relevant when building:
            
            glomcmd = glomcmd +                       \
                ' --timestamp-policy=' + self._info['timestampPolicy']
            glomcmd = glomcmd + ' --sourceid=' + str(self._info['sourceId'])

        stdintoring = path.join(bindir, 'stdintoring')
        stdintoring = stdintoring + ' ' + self._info['ring']
        return glomcmd + '|' + stdintoring
    
    ##
    # _createEventBuilderPipeCmd
    #    Return the command for the event orderer/glom pipeline.
    #
    #
    def _createEventBuilderPipeCmd(self ):
        glom = self._makeGlomCmd()
        
        # TODO below - tclsh should really be the value of the @TCLSH@ automake
        #              symbol.
        
        return 'tclsh ' + ' | ' + glom
    
    ##
    # _constructOrderScript
    #
    #   Returns the script that must be pushed into the event builder
    #   tcl interpreter.
    #
    # @return list - list of command lines.
    #
    def _constructOrdererScript(self):
        result = []
        startScript = path.join(os.getenv('DAQBIN'), 'startOrderer')

        result.append('source ' + startScript)
        result.append('set ::AppnamePrefix ' + self._info['servicePrefix'])
        result.append('set ::OutputRing ' + self._info['ring'])
        result.append('start "' + self._info['serviceSuffix'] + '"')
    
        return result
    
    ##
    # _startEventBuilder
    #    Start our event buidler from the description.
    #
    #  We need to set up a program object as an event builder/glom
    #  pipeline.
    #
    def _startEventBuilder(self):
        environ = self._defineEventBuilderEnv()
        cmd     = self._createEventBuilderPipeCmd()
        script  = self._constructOrdererScript()
        
        
        # Start up the program....
        
        print("event builder command '" + cmd + "'")
        
        self._eventBuilder = ssh.program(
            self._info['host'], cmd, environ, self._info['name']
        )
        # Push the script lines:
        
        time.sleep(0.5)          # Seems a delay is needed before poking scripts.
        for line in script:
            print("Pushing '%s' to tclsh" % line)
            self._eventBuilder.writeLine(line)
        
        
        
    
    #-------------------------------------------------------------------------
    # public methods
    #
    
    ##
    #  start
    #    Start our event builder and its data sources.
    #   
    def start(self):
        self._startEventBuilder()
        time.sleep(1.0)              # let the service get set up.
        for ds in self._dSources:
            print("Starting data source: " + ds.name())
            ds.start()
        
        
    ##
    #  stop our event builder and its data sources:
    #
    def stop(self):
        if self._eventBuilder is not None:
            for ds in self._dSources:
                ds.stop()
            
            # To stop the event builder first send the INTR signal and then
            #  send the exit command to the remaining shell (if any).
            
            try:
                self._eventBuilder.intr()
                self._eventBuilder.writeLine('exit')
            except:
                pass
            self._eventBuilder = None     # Allow garbage collection
        
    ##
    #  Return the fds that may have output from both our event buidler
    #  and its sources.
    #
    # @return list of files. These should not be expected to be in any order.
    #
    def getFiles(self):
        if self._eventBuilder is not None:
            result = [self._eventBuilder.stdout(), self._eventBuilder.stderr()]
            for ds in self._dSources:
                result.extend(ds.getFiles())
        else:
            result =  []
        return result
    ##
    # name
    #   @return string - 'Event Builder: name'
    #
    def name(self):
        return 'Event builder: ' + self._info['name']
        
    ##
    # findProgram
    #   For a file descriptor known to be used by either the event builder or
    #   one of its data sources, return either self (used by event builder)
    #   or the data source the uses the file descriptor.
    #
    # @param f - files descriptor to match
    # @return EventBuilder or DataSource object that uses the file descriptor.
    # @retval None  - if the file descriptor is not used.
    def findProgram(self, f):
        if self._eventBuilder is not None:
            if f in [self._eventBuilder.stdout(), self._eventBuilder.stderr()]:
                return self
            else:
                # It's used by an event source:
                print ('looking in data sources')   
                for s in self._dSources:
                    print('trying ' + s.name())
                    if f in s.getFiles() :
                        print('found in ' + s.name())
                        return s
                    else:
                        print('Not found')
                        print(f)
                        print(s.getFiles())
                    print('should not be here')
                print('Did not find')    
                return None
                
        else:
            return None

##
#  @class EventBuilders
#     Encapsulates a collection of EventBuilder objects providing a simple set of
#     convenience methods to the client.
#
#   
class EventBuilders(object):
    ##
    # __init__
    #   @param api - VardbEvb.VardbEvb objet connected to the database that
    #                describes the event builders and data sources we'll create.
    #
    def __init__(self, api):
        self._api              = api
        self._eventBuilderList = api.listEventBuilders()
        self._eventBuilders    = self._createEventBuilders()
    
    #--------------------------------------------------------------------------
    # private methods:
    #

    
    ##
    # _createEventBuilders
    #   Create the event builders described by the event builder list.
    #   We don't actually start anything, we're just creating the objects
    #   that _will_ eventually start stuff.
    #
    # @return list of event builder objects we created.
    #
    def _createEventBuilders(self):
        result = []
        for e in self._eventBuilderList:
            result.append(EventBuilder(e, self._api))
        
    
        return result

        
    #--------------------------------------------------------------------------
    #  public methods:
    
    ##
    # start
    #   Start all of the event builders and, by implication, their event sources.
    #
    def start(self):
        for e in self._eventBuilders:
            e.start()
        
    ##
    # stop
    #   Stop all of the event builders and, by implication, their event sources.
    #
    def stop(self):
        for e in self._eventBuilders:
            e.stop()
        
    ##
    #  getFiles
    #   @return list of file descriptors (for e.g. select) from all of the
    #            event builders.
    def getFiles(self):
        result = []
        for e in self._eventBuilders:
            result.extend(e.getFiles()) 
        return result
    
    ##
    # findProgram
    #   Returns either the event builder or data source that uses the
    #   specified file descriptor.
    #
    # @param  f   - file to lookup.
    # @return EventBuilder or DataSource object
    # @raises KeyError if the file is not used by any event builder or
    #         data source.
    #
    def findProgram(self, f):
        #
        #  We're going to figure out which event builder has it and then
        #  ask it to refine the search:
        
        for e in self._eventBuilders:
            if f in e.getFiles():
                return e.findProgram(f)
        
        raise KeyError('File descriptor not used by an event builder or any source.')
        
    
    



