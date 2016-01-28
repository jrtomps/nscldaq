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
# @file dialogWrapper.tcl
# @brief Provide megawidgets to support dialogs.
# @author Ron Fox <fox@nscl.msu.edu>
#

package provide dialogWrapper 1.0
package require Tk
package require snit

##
# NonModalDialogWrapper
#   Wraps a widget in a dialog container.  This includes
#   * OK/Cancel buttons located below the widget.
#   * Delegation of all unrecognized options and methods to the contained
#     widget.
#   * Ability to hook scripts to Ok, Cancel and <Destroy>
#   
# OPTIONS
#  -form    - Sets the widget that will appear in the control area.
#  -showcancel - Determines if the widget displays the cancel button.  If
#                false, only the Ok button is displayed.
#  -okcommand  - Script to invoke on OK.
#  -cancelcommand - Script to invoked on cancel.
#  -destroycommand - Script to invoke on destroy.
#
# METHODS
#  controlarea - Returns the parent that should be used when creating the
#                -form widget.

snit::widgetadaptor NonModalDialogWrapper {
    component controlarea;                 # The wrapped widget.
    component okbutton
    component cancelbutton

    option -form -default ""  -configuremethod _setControlArea
    option -showcancel -default true   -configuremethod _showCancelConfig
    option -destroycommand

    delegate option -okcommand to okbutton as -command
    delegate option -cancelcommand to cancelbutton as -command
    delegate option * to controlarea
    delegate method * to controlarea

  
    
    variable action "";               # Will contain the event that ended modality.
    
    ##
    # constructor
    #   lays out the generic shape of the dialog and fills in the
    #   action area.  The dialog is of the form:
    #
    #  +-----------------------------------+
    #  |  frame into which controlarea     | (control area)
    #  |  is put                           |
    #  +-----------------------------------+
    #  | [OK]   [Cancel]                   | (action area).
    #  +-----------------------------------+
    #
    constructor args {
        installhull using ttk::frame
        
        ttk::frame $win.controlframe -relief groove -borderwidth 2
        ttk::frame $win.actionframe
        install okbutton     using ttk::button $win.actionframe.ok     -text Ok
        install cancelbutton using ttk::button $win.actionframe.cancel -text Cancel
        
        grid $win.actionframe.ok $win.actionframe.cancel
        grid $win.controlframe -sticky nsew
        grid $win.actionframe

        grid rowconfigure $win 0 -weight 1       
        grid columnconfigure $win 0 -weight 1       
        grid rowconfigure $win.controlframe 0 -weight 1       
        grid columnconfigure $win.controlframe 0 -weight 1       
 
        $self configurelist $args

	bind $win <Destroy> [mymethod _dispatchDestroy %W]
    }
    ##
    # destructor
    #   Ensure there are no callback handlers left:
    #
    destructor {
        catch {$okbutton     configure -command [list]}
        catch {$cancelbutton configure -command [list]}
        catch {$win bind <Destroy> [list]}
        
    }
    #-------------------------------------------------------------------------de
    #  Public methods
    
    ##
    # controlarea
    #
    # @return widget path - the parent of any -form widget.
    #
    method controlarea {} {
        return $win.controlframe
    }
    
    #-------------------------------------------------------------------------
    # Configuration handlers:
    
    ##
    # _setControlArea
    #
    #   Option handler for the -form configuration option.  This sets the
    #   widget that will be contained in the control area.
    #
    # @param optname - Name of the option being configured (-form)
    # @param widget  - Widget path to the object that should be pasted into the form.
    #
    # @note - The form is installed as the controlarea component so that options
    #         and methods can be delegated to it.
    #
    method _setControlArea {optname widget} {
        install controlarea using set widget
        grid $widget -in $win.controlframe -sticky nsew
        
        set options($optname) $widget
    }
    ##
    # _showCancelConfig
    #    Modify the value of the -showcancel option.
    #
    # @param optname - option name.
    # @param value   - new value.
    #
    method _showCancelConfig {optname value} {
        set old $options($optname)
        set options($optname $value)
        
        if {$old != $value} {
            if {$value} {
                grid $win.actionframe.cancel -row 0 -column 1
            } else {
                grid forget $win.actionframe.cancel
            }
        }
    }
    #---------------------------------------------------------------------------
    #  Private action handlers.
    
    ##
    #  _dispatchDestroy
    #    Dispatch the destroy event
    #
    # @parameter w - the widget being destroyed....we require it to b $win.
    #
    method _dispatchDestroy w {
	if {$w  == $win} {
	    set cmd $options(-destroycommand) 
	    if {$cmd ne ""} {
		uplevel #0 $cmd
	    }
	}
    }
    
}

##
# DialogWrapper
#   Wraps a widget in a dialog container this includes:
#   *  OK/Cancel buttons located below the widget.
#   *  Method to become application modal.
#   *  Delegation of all unrecognized options and methods to the contained
#      widget<methodsynopsis>
# OPTIONS
#  -form    - Sets the widget that will appear in the control area.
#  -showcancel - Determines if the widget displays the cancel button.  If
#                false, only the Ok button is displayed.
# METHODS
#  controlarea - Returns the parent that should be used when creating the
#                -form widget.
#  modal    - grabs events and blocks until either OK or Cancel is clicked.
#             Returns either Ok, Cancel or Destroyed to indicate what
#             caused the exit from modality.
#
snit::widgetadaptor DialogWrapper {
    component wrapper
    delegate option * to wrapper
    delegate method * to wrapper

    
    variable action "";               # Will contain the event that ended modality.
    
    ##
    # constructor
    #   lays out the generic shape of the dialog and fills in the
    #   action area.  The dialog is of the form:
    #
    #  +-----------------------------------+
    #  |  frame into which controlarea     | (control area)
    #  |  is put                           |
    #  +-----------------------------------+
    #  | [OK]   [Cancel]                   | (action area).
    #  +-----------------------------------+
    #
    constructor args {
        installhull using ttk::frame
	install wrapper using NonModalDialogWrapper $win.dlg
	pack $wrapper -expand 1 -fill both
        
        $self configurelist $args
    }
   
    #-------------------------------------------------------------------------de
    #  Public methods
    
    
    ##
    # modal
    #   Enters the modal state:
    #   *   Adds handlers for the buttons and destroy event.
    #   *   grabs events to $win
    #   *   vwaits on the action var.
    #   *   When the vwait finishes, kills off the handlers.
    #   *   Returns the contents of the action variable which will have been
    #       set by the action handler that fired.
    #
    # @return string the action that ended the wait
    #        * Ok - the Ok button was clicked.
    #        * Cancel - the Cancel button was clicked.
    #        * Destroy - The widget is being destroyed.
    #
    method modal {} {
	$wrapper configure -okcommand [mymethod _setAction Ok]
	$wrapper configure -cancelcommand [mymethod _setAction Cancel]
	$wrapper configure -destroycommand [mymethod _setAction Destroyed]
        
        # Here's the modal section.
        
        grab set $win
        vwait [myvar action]
        catch {grab release $win};         # catch in case we're being destroyed.
        
        #  Catches here in case the windows being configured have been
        #  destroyed...
        
        catch {$wrapper configure  -okcommand [list]}
        catch {$wrapper configure  -cancelcommand [list]}
        catch {$wrapper configure -destroycommand [list]}
        
        if {[info exists action]} {
            return $action
        } else {
            return "Destroyed";                # Destruction too far in process
        }
    }
    
   
    #---------------------------------------------------------------------------
    #  Private action handlers.
    
    ##
    # _setAction
    #   Set the action variable to a specific value.
    #
    method _setAction value {
        set action $value
    }
}