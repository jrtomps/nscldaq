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
# @file <filename>.tcl
# @brief <brief purpose>
# @author Ron Fox <fox@nscl.msu.edu>
#

package provide Service 1.0
package require serviceData
package require daqObject
package require img::png


##
# @class Service
#   Encapsulates the data and user interface for defining a service item.
#   A service is a program that must run but is neither connected to data flow
#   nor to the state manager.
#
snit::type Service {
    component data
    component gui
 
    delegate option -provider to data
    delegate option -changecmd to data
    
    delegate option -canvas   to gui
    
    # Expose all but clone (which we have to handle)
    # to the world:
    
    delegate method getProperties to data
    delegate method addSink       to data
    delegate method clearSinks    to data
    delegate method rmSink        to data
    delegate method getSinks      to data
    
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
    delegate method getPosition   to gui

    
    ##
    # Construct our icon image:
    
    typeconstructor {
        image create photo ServiceIcon  \
            -format png                 \
            -file [file join [file dirname [info script]] sysprogram.png] 
    }
    ##
    # constructor
    #   Install the pieces and configure
    #
    constructor args {
        install data using ServiceData %AUTO%
        install gui  using DaqObject %AUTO% -image ServiceIcon
        
        $self configurelist $args
    }
    ##
    # destructor -- destroy our components.
    #
    destructor {
        $data destroy
        $gui destroy
    }
    ##
    #  clone
    #   Create a copy of self.
    #
    # @return copy of self.
    #
    method clone {} {
        set newObj [Service %AUTO%]
        set myprops [$data getProperties]
        set newprops [$newObj getProperties]
        
        $myprops foreach property {
            set name [$property cget -name]
            set value [$property cget -value]
            
            set newprop [$newprops find $name]
            $newprop configure -value $value
        }
        
        return $newObj
    }
    ##
    # type
    #   Return the object type
    #
    method type {} {
        return service
    }
    ##
    # connect
    #   This tosses an error since we've told the world we can't be a party
    #   to any connections.
    #
    # @param direction
    # @param object
    #
    method connect {direction object} {
        error "This object cannot participate in any connections"
    }
    ##
    # disconnect
    #   Throws an error since we can't have been connected to anything.
    #
    # @param object
    #
    method disconnect object {
        error "disconnect - This object cannot have been connected to anything."
    }
    ##
    # isConnectable
    #
    #  @param direction
    #  @return false
    #
    method isConnectable direction {
        return false
    }
    ##
    # connectionPropertyChanged
    #    Called if the properties of an object we are connected to changed.
    #    Note that since we cannot be connected to anything this is a no-op.
    # @param object - the object whose properties changed.
    #
    method connectionPropertyChanged obj {}
    
}