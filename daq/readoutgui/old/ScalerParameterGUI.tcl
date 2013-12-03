#
#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2005.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#     Author:
#             Ron Fox
#	     NSCL
#	     Michigan State University
#	     East Lansing, MI 48824-1321
#

#  This file contains a snit megawidget that
#  encapsulates the scalerParameterDialog code created by the
#  Komodo GUI designer.
#    Client code can either use the widget directly for
#  their own purposes or call the convenience unbound
#  functions
#     ScalerParameterGUI::create   - Create a non-modal instance of the megawidget.
#     ScalerParameterGUI::modal    - Run a modal instance of the megawidget.
#
#
#  The megawidget has the following configuration options:
#     -scalers      - Number of scalers desired.
#     -period       - period between scaler readout.
#     -okcommand    - Provides a script to be called when the ok button is clicked.
#     -cancelcmd    - Provides a script to be called when the cancel button is clicked.
#
package provide ScalerParameterGUI 2.0
package require scalerParameterDialog
package require snit
namespace eval ScalerParameterGUI {}

snit::widget ScalerParameterGUI {
    hulltype toplevel
    option   -scalers    0
    option   -period     2
    option   -okcommand {}
    option   -cancelcmd {}

    # Construct the widget.  This is invoked
    # as if it were a tk widget creation.
    #    ScalerParameterGUI -okcommand {puts ok hit}
    # For trivial example.
    #
    constructor {args} {

        #  Create the GUI  and set defaults.

        scalerParameterDialog::ui $win

        $win.ok       configure -command [mymethod onOk]
        $win.cancel   configure -command [mymethod onCancel]

        #  Process the user configuration parameters.

        $self configurelist $args
    }
    destructor  {
    }
    #
    #   Public methods
    #

    # $object modal
    #     Turns the object into a modal dialog by doing a grab
    #     on the toplevel and waiting for a hidden window to be
    #     destroyed.
    #
    method modal {} {
        frame $win.hidden
        grab  $win
        tkwait window $win.hidden
        catch [grab release $win]
    }
    #   Private methods

    # DispatchScript
    #     Factors out the grundge of dispatching to a possibly
    #     empty or nonexistent script.
    # Parameters:
    #      script   - the script to eval if non null.
    #      args     - args for script.
    method DispatchScript {script} {
        if {$script != ""} {
            eval $script
        }
    }
    # EndModality
    #    Exit the modal event loop by killing $win.hidden
    #
    method EndModality {} {
        catch {destroy $win.hidden}
    }
    # Configuration set handlers

    # Set the number of scaler channels in the box.
    #
    onconfigure -scalers {value} {
        $win.channels set $value
    }
    # Set the number of seconds between readouts.
    #
    onconfigure -period  {value} {
        $win.period set $value
    }
    # Configuration get handlers.

    oncget -scalers  {
        return [$win.channels get]
    }
    oncget -period {
        return [$win.period get]
    }
    # Event handlers.
    #
    #    Note that both event handlers
    #    exit the modal event loop

    # onOk
    #    Called when the ok button is clicked.
    #
    method onOk {} {
        $self DispatchScript $options(-okcommand)
        catch {$self EndModality}
    }
    # onCancel
    #   Called when the cancel button is clicked:
    #   Dispatch the script while the window is still
    #   alive then destroy the toplevel.
    #   This allows us to distinguish between
    #   Ok and Cancel.
    #
    method onCancel {} {
        $self DispatchScript $options(-cancelcmd)
        catch {destroy $win}
    }
}

# Helper functions:
#

# ScalerParameterGUI::create win loadChannels loadPeriod
#    Creates a non modal scaler parameter dialog box and
#    loads it as described.  Returns the widget created
#    so that the caller can subsequently configure e.g. callbacks.
# Parameters:
#   win             - name of the toplevel to create
#   loadChannels    - The number of channels to supply to the
#                     -scalers configuration parameter.
#   loadPeriod      - the scaler readout period to supply to
#                     the -period parameter.
# Returns:
#    The snit object that represents the megawidget dialog.
#    this allows the user to post customize the widget.
#
proc ScalerParameterGUI::create {win loadChannels loadPeriod} {
    return [ScalerParameterGUI $win -scalers $loadChannels -period $loadPeriod]
}
# ScalerParameterGUI::modal win loadChannels loadPeriod
#    Creates a modal scaler parameter dialog box and
#    loads it as described.  Returns the selected
#    scaler parameters.
# Parameters:
#    win           - Name of the toplevel window to create to hold the dialog.
#    loadChannels  - Number of channels to load into the channels spinbox initially.
#    loadPeriod    - Period in seconds to load into the seconds spinbox.
# Returns:
#    A two element list containing a channel count and a period in that order.
#    If the dialog  was cancelled, the original loadChannels and loadPeriod
#    are returned.
#
proc ScalerParameterGUI::modal {win loadChannels loadPeriod} {
    set widget [ScalerParameterGUI::create $win $loadChannels $loadPeriod]
    $widget modal

    #  Now build up the answer list:

    if {[catch {$widget cget -scalers} values]} {
        set values $loadChannels
    }
    lappend result $values

    if {[catch {$widget cget -period} values]} {
        set values $loadPeriod
    }
    lappend result $values
    catch {destroy $widget}

    return $result
}
