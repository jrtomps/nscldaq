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
# @file ConnectorObject.tcl
# @brief This is a connector as a DAQ objecdt.
# @author Ron Fox <fox@nscl.msu.edu>
#

package provide connectorObject 1.0
package require snit
package require img::png
package require properties
package require daqObject

##
# @class
#    This is a connector as a DAQObject.  This class is mostly stub with the
#    behavior of a true connector set up by ConnectorInstaller.
#    What we need to do, however is provide a connector icon that can appear
#    in the toolbar.  So we consist of:
#    *  An arrow icon from arrow.png
#    *  An empty property list.
#
#  The classical delegations required by the UI that will actually never get
#  called.
#
snit::type ConnectorObject {
    component data

    variable dummyPropertyList
    
    delegate option -canvas   to gui
    
    # Expose all but clone (which we have to handle)
    # to the world:
    
    
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
    
    ##
    # typeconstructor
    #    Creates the arrowicon image from the arrow.png.
    #
    typeconstructor {
        image create photo ArrowIcon -format png \
            -file [file join [file dirname [info script]] arrow.png]
                              
    }
    ##
    # constructor
    #   Glue everything together.
    #
    constructor args {
        install gui using DaqObject %AUTO% -image ArrowIcon
        set dummyPropertyList [propertylist %AUTO%]
        
        $self configurelist $args
    }
    ##
    # Return the property list
    #
    method getProperties {} {
        return $dummyPropertyList
    }
    method type {} {
        return connector    
    }
}