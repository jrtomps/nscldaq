#!/bin/sh
# -*- tcl -*-
# The next line is executed by /bin/sh, but not tcl \
exec tclsh "$0" ${1+"$@"}

# This software is Copyright by the Board of Trustees of Michigan
# State University (c) Copyright 2014.
#
# You may use this software under the terms of the GNU public license
# (GPL).  The terms of this license are described at:
#
# http://www.gnu.org/licenses/gpl.txt
#
# Authors:
#         Ron Fox
#         Jeromy Tompkins
#         NSCL
#         Michigan State University
#         East Lansing, MI 48824-1321
#


##
# @file LogDisplay.tcl
# @brief Ties together the model, view and controller for the log message display
# @author Ron Fox <fox@nscl.msu.edu>
#
package provide LogDisplay 1.0
package require Tk
package require snit
package require LogModel
package require LogView
package require LogController

##
# @class LogDisplay
#    Ties together a model, view, controller for the log display.
#    We need to know the file for the database of the model and a widget
#    path for the view.
#    At this point, filtering is an external request -- could be coming from
#    a separate chunk of the GUI.
#
snit::type LogDisplay {
    option -filename -default :memory: -readonly 1
    option -widget   -default .logview -readonly 1
    option -colors   -default [dict create \
        DEBUG black INFO green WARNING orange SEVERE red DEFECT red] \
        -configuremethod _configColors
    
    
    component model
    component view
    component controller
    
    delegate method addMessage to controller

    
    ##
    # constructor
    #   Put this all together.  We don't manage the -widget as we don't know
    #   if the application is going to grid or pack so that's left to them.
    #
    constructor args {
        $self configurelist $args
        
        install model using LogModel %AUTO% -file $options(-filename)
        install view  using LogView  $options(-widget) -colors $options(-colors)
        install controller using LogController %AUTO% -model $model -view $view

    }
    ##
    # destructor
    #   Kill off our components
    #
    destructor {
        $controller destroy
        $model      destroy
        $view       destroy
        
    }
    #--------------------------------------------------------------------------
    #   public methods
    #
    
    ##
    # setFilter
    #    Set a new filter:
    #
    # @param filter
    #
    method setFilter filter {
        $controller configure -filter $filter
    }
    
    #-------------------------------------------------------------------------
    # Private methods
    
    ##
    # _configColors
    #   Configure a new color scheme for the widget.
    #
    # @param name -colors option name.
    # @param value - Color scheme definition.
    #
    method _configColors {name value} {
        $view configure -colors $value
        set options($name) $value
    }
}
