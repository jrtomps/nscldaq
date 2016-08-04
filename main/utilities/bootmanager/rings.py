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
# @file   rings.py
# @brief  Create and destroy rings in remote systems (or local).
# @author <fox@nscl.msu.edu>

from nscldaq.programs import ssh
from nscldaq.vardb import VardbRingbuffer
import os
from os import path

def _ringbufferCommand():
    bindir = os.getenv('DAQBIN')
    command = path.join(bindir, 'ringbuffer')
    return command


def _doSomethingToAllRings(api, something):
    rings = api.list()
    for ring in rings:
        host = ring['host']
        name = ring['name']
        something(host, name)

##
# delRing
#   Delete a ring - it's not an error to delete a ring that does not exist.
#
# @param host - host in which the ring is located.
# @param name - name of the ring.
#
def delRing(host, name):
    command = _ringbufferCommand()
    
    # Delete the ringbuffer drain output/error ignoring:
    
    print("Deleting %s@%s" % (name, host))
    deleter = ssh.Transient(
        host, '%s %s %s' % (command, 'delete', name)
    )
    deleter.output()
    deleter.error()
##
# makeRing
#
#   Create a ring in some host.  If the ring already exists, it is
#   first destroyed.  This is done to provide for the capability to
#   later modify ringbuffer characteristics (e.g. size).
#
#  @param host    - Host in which the ring is to live.
#  @param name    - Name of the new ringbuffer.
#
#  @note - We assume that:
#      *  The environment variable DAQBIN is defined and is the bin directory
#         of the version of nscldaq we are running.
#      *  The remote system has this version of nscldaq installed in the same
#         place.
#   
def makeRing(host, name):
    delRing(host, name)
    print ("makeRing %s@%s" % (host, name))
    command = _ringbufferCommand()
    
    #  Create the new ring buffer, ignore the output but any error lines
    #  raise an exception:
    
    print('%s %s %s' % (command, ' create', name))

    creator = ssh.Transient(
        host, '%s %s %s' % (command, ' create', name)
    )
    creator.output()
    creator.error()
    
##
# makeAllRings
#   Creat all rings defined in a variable database
#
# @param api - VardbRingbuffer api object.
#
#  If there's an error createing a ring, the exception
#  raised by makeRings will be allowed to propagate.
#
def makeAllRings(api):
    _doSomethingToAllRings(api, makeRing)

##
# rmAllRings
#    Removes all rings defined in the variable database.
#
# @param api - VardbRingbuffer Api object.
#
def rmAllRings(api):
    _doSomethingToAllRings(api, delRing)
        

    