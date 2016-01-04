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
# @file eventBuilderObject.tcl
# @brief encapsulates data and GUI of an event builder.
# @author Ron Fox <fox@nscl.msu.edu>
#

package provide eventBulderObject 1.0
package require Tk
package require snit
package require eventBuilder
package require daqObject
package require img::png

##
# @class EventBuilderObject
#    This class encapsulates everything the experiment editor needs for an
#    instance of an event builder (see also the EventBuilderInstaller which is
#    needed to create  an event builder on the editor canvas from the toolbar
#    however):
#
#   This enapsulation includes:
#
#    *    The data properties associated with an event builder.
#    *    The Graphical representation of an event builder.
#    *    Much of the graphical behavior of an event builder (some of that
#         is managed by the EventBuilderInstaller however).
#    *    Handling of connections with other objects.
#

snit::type EventBuilderObject {
    component data;                         # Property list.
    component gui;                          # Graphical represntation.
    
    # Delegations of options and methods:
    
    delegate option -changecmd to data
    delegate option -canvas   to gui
    
    # Expose all but clone (which we have to handle)
    # to the world:
    
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
    
    # Flag indicating if we have an output connection (to a ringbuffer).
    # Since we can only allow 1.
    
    variable outputRing ""
    
    ##
    # typeconstructor
    #    When the type has been successfully 'compiled'  this is called
    #    once only.  The method produces an EventBuilderIcon photo image that
    #    contains the event builder icon.
    #
    typeconstructor {
        image create photo EventBuilderIcon -format png \
            -file [file join [file dirname [info script]] eventBuilder.png]
    }
    
    ##
    # constructor
    #   This is the object constructor called to initialize the state of an
    #    object at construction time.
    #
    # @param args - -option value pairs.
    #
    constructor args {
        install data using EventBuilder %AUTO%
        install gui using  DaqObject %AUTO% -image EventBuilderIcon
        
        $self configurelist $args
    }
    
    destructor {
        $data destroy
        $gui  destroy
    }
    
    #---------------------------------------------------------------------------
    #  Private methods
    
    
    ##
    # @return property  - named property..
    
    method _findProperty {name} {
        set pl [$data getProperties]
        return [$pl find $name]
    }
    ##
    # Disable editing of the output ring property (we're connected).
    #
    method _disableRingEditing {} {
        set p [$self _findProperty ring]
        $p configure -editable 0
    }
    ##
    #  enable editing the ring property
    #
    method _enableRingEditing {} {
        set p [$self _findProperty ring]
        $p configure -editable 1
    }
    ##
    #  Disable editing of the host property.
    #
    method _disableHostEditing {} {
        set p [$self _findProperty host]
        $p configure -editable 0
    }
    ##
    # Enable editing the host property
    #
    method _enaleHostEditing {} {
        set p [$self _findProperty host]
        $p configure -editable 1
    }
    
    #---------------------------------------------------------------------------
    #  Public methods
    
    ##
    # clone
    #   Create a copy of self:
    #
    # @return EventBuilderObject - exact duplicate of $self.
    #
    method clone {} {
        set new [EventBuilderObject %AUTO%]
        
        #  Copy our data properties into new:
        
        set oProps [$self getProperties]
        set nProps [$new  getProperties]
        
        $oProps foreach prop {
            set name [$prop cget -name]
            set v    [$prop cget -value]
            
            set nprop [$nProps find $name]
            $nprop configure -value $v
        }
        
        
        return $new
    }
    
    ##
    # type
    #   @return string - 'eventbuilder'
    #
    method type {} {
        return eventbuilder
    }
    
    ##
    #  connect
    #   Handle connections between us and other objects.
    #   We only really care about the outbound connection;
    #   we flag that we have one since we need to disallow multiple connections.
    #   Setting an output connection will also have us disabling the
    #   ring property.
    #
    # @param direction - from indicating we are the source or to indicating we are
    #                    the connection sink.
    # @param object    - Objectw we are being connected to.
    #
    
    method connect {direction object} {
        if {$direction eq "from"} {
            if {$outputRing ne ""} {
                error "This event builder already has an output ring"
            }
            if {[$object type] ne "ring"} {
                error "Event builder output can only go to a ring buffer."
            }
            set outputRing $object
            $self _disableRingEditing
            $self _disableHostEditing
        }
        
    }
    ##
    # disconnect
    #   Called when this object is being disconnected from another.
    #   we only care if the connection is our output ring in which case
    #   we need to  mark that this was done as well as re-enabling editing of
    #   the affected properties.
    #
    # @param object - the object being disconnected from us
    #
    method disconnect {object} {
        if {$object eq $outputRing} {
            set outputRing ""
            $self _enableRingEditing
            $self _enableHostEditing
        }
    }
    ##
    # isConnectable
    #   Determines if a connection is legal.  It's always legal to
    #   connect to us (for direction to be to).  It's only legal to connect
    #   'from' us if there's no current output ring:
    #
    # @return bool
    #
    method isConnectable {direction} {
        if {($direction eq "to") && ($outputRing ne "") } {
            return false
        } else {
            return true
        }
    }
    ##
    # connectionPropertyChanged:
    #  Called if a property changed in a connected object.  If the
    #  object is our output ring, refresh our host and ring name from the
    #  object.
    #
    # @param obj - Connected object after it's been changed.
    #
    method connectionPropertyChanged obj {
        if {$object eq $outputRing} {
            set host [$self _findProperty host]
            set ring [$self _findProperty ring]
            set objprops [$obj getProperties]
            
            # Update host
            
            set objHost [$objprops find host]
            $host configure -value [$objHost cget -value]
            
            # Update ring:
            
            set objRing [$objprops find name]
            $ring configure -value [$objRing cget -value]
        }
    }
}
