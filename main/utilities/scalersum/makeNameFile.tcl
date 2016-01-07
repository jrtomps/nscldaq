#!/bin/sh
# -*- tcl -*-
# The next line is executed by /bin/sh, but not tcl \
exec tclsh "$0" ${1+"$@"}

#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2014.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Authors:
#             Ron Fox
#             Jeromy Tompkins 
#	     NSCL
#	     Michigan State University
#	     East Lansing, MI 48824-1321



##
# @file makeNameFile.tcl
# @brief Create a name file from a scaler definition file.
# @author Ron Fox <fox@nscl.msu.edu>
#


#  Add TclLibs to the library search path - assuming we're in DAQBIN

set here [file dirname [info script]]
set libdir [file normalize [file join $here .. TclLibs]]
lappend auto_path $libdir

puts {Hello world}