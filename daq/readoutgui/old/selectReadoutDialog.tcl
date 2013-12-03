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
#            NSCL
#            Michigan State University
#            East Lansing, MI 48824-1321
#


# This package provides a snit widget and convenience functions
# for the readout chooser dialog.
#  The dialog widget has the following options:
#      -host          - Initial value for host (defaults to "").
#      -path          - Initial value for path (defaults to localhost).
#      -okcommand     - Script to run when the ok button is plinked.
#      -cancelcmd     - Script to run when the cancel button is plinked.
# In addition, the following convenience function is supplied:
#
#    selectReadoutDialog::createModal initialhost initialpath
#
#  Which creates a modal dialog and returns the results of the interaction.
#
package provide selectReadoutDialog 1.0

package require snit
package require selectReadout

namespace eval selectReadoutDialog {}

snit::widget selectReadoutDialog {
    hulltype toplevel;                 # We're a dialog after all.

    # Widget options.

    option -host localhost
    option -path {}
    option -okcommand {}
    option -cancelcmd {}

    # Construction and desctruction.

    constructor {args} {
        # Create the gui and set our callbacks.

        selectReadout::ui  $win
        $win.ok     configure -command [mymethod onOk]
        $win.cancel configure -command [mymethod onCancel]

        # Process configuration stuff.

        $self configurelist  $args
    }
    destructor {}

    # Public methods.

    # object modal
    #      Turn the dialog into a modal one.
    #
    method modal {} {
        frame $win.hidden
        grab set $win
        tkwait window $win.hidden
        catch {grab release $win}

    }

    # Private methods

    # End the modality of the dialog.
    #
    method endModal {} {
        catch {destroy $win.hidden}
    }
    # Dispatch a script if nonblank.
    #
    method dispatchScript {script} {
        if {$script != ""} {
            eval $script
        }
    }
    # Called when OK is clicked.
    #
    method onOk {} {
        $self dispatchScript $options(-okcommand)
        $self endModal
    }
    # Called when cancel is clicked.

    method onCancel {} {
        $self dispatchScript $options(-cancelcmd)
        if {[winfo exists $win.hidden]} {
            catch {destroy $win}
        }
    }

    # $self setEntry name text
    #    Sets an entry widget relative to the
    #    top of the widget to a value.
    #
    method setEntry {name text} {
        $win.$name delete 0 end
        $win.$name insert end $text
    }

    # option configuration handlers.

    onconfigure -host {value} {
        $self setEntry hostname $value
    }
    onconfigure -path {value} {
        $self setEntry path $value
    }


    # option configuration selectors.

    oncget -host {
        return [$win.hostname get]
    }
    oncget -path {
        return [$win.path get]
    }


}
# selectReadoutDialog::createModal host path
#
#  Convenience procedure to do a modal dialog
# Parameters:
#   win    - name of top levelwidget.
#   host   -  initial host name.
#   path   - initial path.
# Returns:
#    list of host and path.
#
proc selectReadoutDialog::createModal {win host path} {
    set select [selectReadoutDialog $win -host $host]
    if {$path != ""} {
        $select configure -path $path
    }
    $select modal

    if {[winfo exists $select]} {
        set host [$select cget -host]
        set path [$select cget -path]
        destroy $select
    }
    return [list $host $path]
}
