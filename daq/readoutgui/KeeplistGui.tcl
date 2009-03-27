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


package provide KeeplistGUI 2.0
package require keeplistDialog
package require snit

#
#  This snit widget creates a dialog
#  that displays the keep list dialog.
#  The dialog is displayed and can be run either
#  as a modal or non modal dialog.
#  The package provides two helper functions:
#  modalKeeplistDialog  - Make a modal keeplist dialog
#  keeplistDialog       - Make a non modal keeplist dialog.
#
# options:
#    -eligible   - Set/Get the contents of the listbox
#                  of runs that are elligible for retension.
#    -scheduled  - Set/Get the contents of the listbox of runs
#                  that are scheduled for retension.
#    -retained   - Set/Get the contents of the listbox of runs
#                  that are already retained.
#    -command    - Register a script to be called when the Ok button
#                  is called, The script is given the widget name
#                  as a parameter.
#    -cancelcmd  - Register a script to be called when the Cancel button
#                  is called, the script is given the widget name as a parameter.
#    Note that the command callback wrappers will destroy the GUI of the widget
#    >after< the callback script is invoked.
#
# Public Methods (other than construction/destruction)
#    modal       -
#

snit::widget keeplistGUI {
    hulltype toplevel
    option -eligible
    option -scheduled
    option -kept
    option -command    {}
    option -cancelcmd  {}

    # Constructor:

    constructor {args} {
        keeplistDialog::init $win
        $win.ok     configure -command [mymethod onOk]
        $win.cancel configure -command [mymethod onCancel]
        $self configurelist $args
    }
    # Destructor:

    destructor {
    }

    # Public members:

    # modal:
    #    Make the dialog modal.
    #    This is done by creating a hidden widget
    #    doing a grab and then a tkwait on the
    #    hidden window so that we exit when the
    #    hidden window is destroyed.
    #    Doing it this way also allows us to potentially
    #    de-modalize the dialog as a result of a -command
    #    (unsupported in this release)
    #
    method modal {} {
        frame $win.hidden
        grab  $win
        tkwait window $win.hidden
        catch {grab release $win}
    }
    # Private members:

    method FillListBox {widget list} {
        set newlist [lsort -command keeplistDialog::compareRunFiles $list]
        $widget delete 0 end
        foreach item $newlist {
            $widget insert end $item
        }

    }
    # EndModalEventLoop
    #    If the dialog is modal ($win.hidden exists),
    #    the hidden widget is destroyed exiting the modal
    #    event loop.
    #
    method EndModalEventLoop {} {
        if {[winfo exists $win.hidden]} {
            destroy $win.hidden
        }
    }
    # DispatchScript script parameter
    #   Dispatches a user script (if it is not empty)
    #   appending the parameter.
    #
    method DispatchScript {script parameter} {
        if {$script != ""} {
            eval $script $parameter
        }
    }
    # Configuration set handlers:

    # Fill the elligible run list of the dialog box
    # with the sorted list of runs in the list.
    # The list is a set of filenames (with or without
    # paths.
    #   The widget we are filling is $win.unkeptruns
    # We use the proc
    #   keeplistDialog::compareRunFiles
    # To help us sort the incoming list
    #
    onconfigure -eligible {list} {
        $self FillListBox $win.unkeptruns $list
    }
    # Same as for -eligible
    # except that the target widget is:
    #   $win.scheduledforkeep
    onconfigure -scheduled {list} {
        $self FillListBox $win.scheduledforkeep $list
    }
    #  Same as for -eligible except that the widget
    #  filled is $win.keptruns
    onconfigure -kept {list} {
        $self FillListBox $win.keptruns $list
    }

    # Configuration get handlers:

    #  Retrieve and return the contents of the
    #  $win.unkeptruns list box.
    #
    oncget -eligible  {
        return [$win.unkeptruns get 0 end]
    }
    #  Retrieve and return the contents of the
    #  $win.scheduledforkeep listbox.
    #
    oncget -scheduled {
        return [$win.scheduledforkeep get 0 end]
    }
    # Retrieve and return the contents of the
    #  $win.kept listbox.
    #
    oncget -kept      {
        return [$win.keptruns get 0 end]
    }
    # Event handlers;

    # OK button was hit.
    # If the user has registered a -command invoke it with the
    # top level widget.
    # After that, if the hidden widget exists, destroy it
    # to exit the modal event loop, otherwise leave things
    # well enough alone.
    #
    method onOk {} {
        $self DispatchScript $options(-command) $win
        $self EndModalEventLoop
    }
    # onCancel
    #   invokes the user's script and then destroys
    #   the window.  Destroying the event loop ensures
    #   that the client cannot fetch modified lists.
    #
    method onCancel {} {
        $self DispatchScript $options(-cancelcmd) $win
        destroy $win
    }
}

namespace eval KeeplistGUI {
    variable KeeplistModified 0
}

# KeeplistGui::StuffListBox widget filenames
#       Stocks a listbox with run file names
#       (not paths).  The names are assumed to be
#       runfiles and are inserted sorted by run number
# Parameters:
#   widget    - The name of the list box to stuff.
#   filenames - The list of filenames (potentially with
#               path information) to stuff into the box.
#
proc KeeplistGUI::StuffListBox {widget filenames}  {
    # First a new list with no path info:

    set nameonly ""
    foreach file $filenames {
        lappend nameonly [file tail $file]
    }

    set nameonly [lsort -command keeplistDialog::compareRunFiles $nameonly]

    # Stuff the list box:

    foreach name $nameonly {
        $widget insert end $name
    }

}

# KeeplistGUI::KeeplistDialog Elligible Keeplist KeptList
#     Let the user manipulate the kept run list.
#     Kept runs are not deleted after a stage pass.
#     Note that while the parameters are run files, they
#     are turned into run names (e.g. run123-4096.evt).
#     prior to being put in the list box.
#
# Parameters:
#    Elligible - run files that are elligible for staging, but not
#                marked for keep.
#    KeepList  - run files that are elligible for staging, but are
#                scheduled to be retained.
#    KeptList  - Run files that have been staged and kept.
#                (readonly).
# Returns the name of the toplevel widget it creates:
#
proc KeeplistGUI::keeplistDialog {topname Elligible KeepList KeptList} {
    return [keeplistGUI $topname -eligible $Elligible -scheduled $KeepList -kept $KeptList]

}
#  Same as above, however the dialog is made modal and
#  the return value is a 3 element list consisting of
#  the contents of the three list boxes (eligible, scheduled and kept)
#  on exit.
#    If cancel, or destroyed, the original lists are returned.
#
proc KeeplistGUI::modalKeeplistDialog {topname Elligible Keeplist KeptList} {
    set widget [KeeplistGUI::keeplistDialog $topname $Elligible $Keeplist $KeptList]
    $widget modal

    if {[catch {$widget cget -eligible} values]} {
        set values $Elligible
    }
    lappend result $values

    if {[catch {$widget cget -scheduled} values]} {
        set values $Keeplist
    }
    lappend result $values

    if {[catch {$widget cget -kept} values]} {
        set values $KeptList
    }
    lappend result $values

    catch {destroy $widget };    # Normal exit only destroys hidden.

    return $result

}


