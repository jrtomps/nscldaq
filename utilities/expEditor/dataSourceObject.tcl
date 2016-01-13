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
# @file dataSourceObject.tcl
# @brief Encapsulate both data source and its representation on a canvas.
# @author Ron Fox <fox@nscl.msu.edu>
#

package provide dataSourceObject 1.0
package require snit
package require dataSource
package require Tk
package require daqObject

##
# @class DataSourceObject
#
#   This encapsulates the properties, behavior and presentation of a
#   data source to the DAQ configuration editor.  Essentially we have
#   two components;
#
#   *  a DataSource which manages the properties.
#   *  a DaqObject which has the graphical representation of the object.
#
#  We provide all the glue logic.
#
snit::type DataSourceObject {
    component data;                 # Properties holding objet.
    component gui;                  # Graphical representation.
    
    
    #  Option delegation:
    
    delegate option -changecmd to data
    delegate option -canvas    to gui
    
    #  Method delegation
    
    delegate method getProperties to data
    
    delegate method drawat        to gui
    delegate method moveto        to gui
    delegate method moveby        to gui
    delegate method addtag        to gui
    delegate method rmtag         to gui
    delegate method tags          to gui
    delegate method getPosition   to gui
    delegate method getId         to gui
    delegate method size          to gui
    delegate method bind          to gui
    
    variable eventBuilder  "";                   # Name of target event builder.
    
    ##
    # typeconstructor
    #    Called when the type is created (not instantiated).  Create the
    #    graphical representation we're going to use for the gui component:
    #
    typeconstructor {
        image create photo DataSourceIcon -format png \
            -file [file join [file dirname [info script]] dsource.png]
    }
    ##
    # constructor
    #   Called at object instantiation time. We just need to create our
    #   components and confgure our options (well our components's options).
    #
    # @param args - option list.
    #
    constructor args {
        install data using DataSource %AUTO%
        install gui  using DaqObject %AUTO% -image DataSourceIcon
        
        $self configurelist $args
    }
    
    #---------------------------------------------------------------------------
    #  Private methods.
    #
    
    
    #---------------------------------------------------------------------------
    # Public methods
    #
    
    ##
    # clone
    #    Create a new object that is an exact copy of this object --
    #    except the -changecmd and -canvas options are not configured/copied.
    #    Normally this is done by an object installer which will supply those.
    #
    #    The purpose of the clone is really to be able to ensure the copy has
    #    duplicated the default properties of an exemplar object.
    #
    # @return EventBuilderObject name - copy of $self.
    #
    method clone {} {
        set new [DataSourceObject %AUTO]
        
        # Copy properties:
        
        set oldProps [$self getProperties]
        set newProps [$new getProperties]
        
        #   TODO:  Need to copy editable state too?
        
        foreach p $oldProps {
            set name [$p cget -name]
            set v    [$p cget -value]
            
            set dest [$newProps find $name]
            $dest configure -value $v
        }
        
        return $new
    }
    ##
    # connect
    #   Called when the user attempts to connect this object to another.
    #   We have severe restriction on what we can be connected to:
    #   -  Connections terminating in us must come from a ringbuffer.
    #   -  Connections initiaiting in us must go to an eventbuilder.
    #
    #  @note isConnectable will enforce the fact that we can have at most
    #        one inbound and one outbond connection at a time.
    #
    # @param direction - from | to - indicating the connection direction.
    # @param object    - Object on the other end of the connection that's being
    #                    attempted.
    #
    method connect {direction object} {
        
    }
}