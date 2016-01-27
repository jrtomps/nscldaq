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
    variable ring          "";                   # ring buffer data source.
    
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
    
    ##
    # Glue a host and name together to a tcp uri:
    #
    method _makeRingUri {host name} {
        return [format tcp://%s/%s $host $name]    
    }
    
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
        set new [DataSourceObject %AUTO%]
        
        # Copy properties:
        
        set oldProps [$self getProperties]
        set newProps [$new getProperties]
        
        #   TODO:  Need to copy editable state too?
        
        $oldProps foreach p {
            set name [$p cget -name]
            set v    [$p cget -value]
            
            set dest [$newProps find $name]
            $dest configure -value $v
        }
        
        return $new
    }
    ##
    # type
    #   @return string - dataSource - object type.
    #
    method type {} {
        return datasource
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
        if {$direction eq "from"} {
            
            # Object must be an event builder.
            
            if {[$object type] ne "eventbuilder"} {
                error "Ring data sources can only send data to event builders"
            }
            # isConnectable would have told the client if we can't actually
            # connect but we'll be defensive too:
            
            if {$eventBuilder ne ""} {
                error "This data source is already connected to an event builder.  Disconnect first."
            }
            set eventBuilder $object
            
        } else {
            # Object must be a ring buffer:
            #
            if {[$object type] ne "ring"} {
                error "Ring data sources can only get data from ring buffers"
            }
            if {$ring ne ""} {
                error "This data source is already connected to a ringbuffer. Disconnect first"
            }
            #  - Set the ring property to the URI of the object and make it not
            #    editable.
            
            set properties [$data getProperties]
            set ringProp   [$properties find ring]
            
            set objectProps [$object getProperties]
            set ringName    [$objectProps find name]
            set ringHost    [$objectProps find host]
            set host [$ringHost cget -value]
            set name [$ringName cget -value]
            
            $ringProp configure -value [$self _makeRingUri host name] -editable 0
            
            set ring $object           
            
        }
    }
    ##
    # disconnect
    #   Remove a connection from $self.
    #
    # @param object   - the object that we will no longer be connected to.
    #
    method disconnect object {
        if {$object eq $eventBuilder} {
            set eventBuilder ""
        } elseif {$object eq $ring} {
            set ring ""
            set props [$data getProperties]
            set ringProp [$props find ring]
            $ringProp configure -value "" -editable 1
        } else {
            error "Object being disconnected is not actually connected to us.."
        }
    }
    ##
    # isConnectable
    #    Called prior to a connection to see if $self can accept this sort of
    #    connection at this time.
    #
    # @param direction from | to - Direction of connection.
    # @return bool -true if we can accept this sort of connection, false if not.
    #
    method isConnectable direction {
        if {$direction eq "from"} {
            return [expr {$eventBuilder eq ""}]
        } elseif {$direction eq "to"} {
            return [expr {$ring eq ""}]
        } else {
            error "Invalid direction keywords  $direction is not in {from, to}"
        }
    }
    ##
    # connectionPropertyChanged
    #  Called if an item we are connected to had a property change.
    #  We only care about the ring buffer connection.  If that had a property
    #  change we  need to update our ring URI.
    #
    # @param object - object whose property changed.
    #
    method connectionPropertyChanged object {
        if {$object eq $ring} {
            # Fetch the ring name and host:
            
            set ringProps [$ring getProperties]
            set hostProp  [$ringProps find host]
            set nameProp  [$ringProps find name]
            set host [$hostProp cget -value]
            set name [$nameProp cget -value]
            
            set ourring [$data getProperties]
            set ourringProp [$ourring find ring]
            $ourringProp configure -value [$self _makeRingUri $host $name]
            
        }
    }
    ##
    # getEventBuilder
    #    return the event builder object we are connected to.
    #
    # @return eventBuilderObject
    # @retval "" - not connected to any event builder.
    #
    method getEventBuilder {} {
        return $eventBuilder
    }
}
