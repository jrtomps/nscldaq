#
#   Provides data source prompting megawidget.
#

package require Tk
package require snit
package require Iwidgets


# dataSourcePrompt
#
# Provides a megawidget dialog that can be used to prompt for
# a data source.  The shape of the widget:
#
#  +---------------------------------------------+
#  |   Host:  [   entry                 ]        |
#  |   User:  [   entry                 ]        |
#  +---------------------------------------------+
#  |  <Ok>   <Cancel>              <Help>        |
#  +---------------------------------------------+
#
# OPTIONS:
#   Data options:
#    -host           - Value of host entry.
#    -user           - Value of user entry.
#    -helptext       - help text (html)
#   Action handler methods:
#    -okcommand      - Script to execute on ok.
#    -cancelcommand  - Script to execute on cancel.
# METHODS:
#    modal   - Makes the dialog application modal.
#    dismiss - Removes modality
#
snit::widget dataSourcePrompt {
    hulltype toplevel                   ;# dialogs live in top levels
    
    option -host localhost
    option -user $::tcl_platform(user)
    option -helptext {<html><head><title>No help</title><body><h1>No help has been provided</h1></body</html>}
    
    option -okcommand {}
    option -cancelcommand {}
    
    variable fakewin   {}
    
    #
    #   Construct the dialog
    #
    constructor args {
        $self configurelist $args           ;# handle command line args.
        
        # Dialogs usually have a pair of frames, the action area which has
        # controls that are maniuplated and the command area that has the
        # buttons that tell the application what to make of the controls.
        #
        
        set action  [frame $win.action  -relief raised]
        set command [frame $win.command -relief raised]
        
        # Create/layout the action area.
        
        label $action.hostl  -text {Host: }
        entry $action.host   -textvariable ${selfns}::options(-host)
        
        label $action.userl  -text {User: }
        entry $action.user   -textvariable ${selfns}::options(-user)
        
        grid $action.hostl $action.host  -sticky w
        grid $action.userl $action.user  -sticky w
        
        
        # Create/layout the command area:
        
        button $command.ok      -text Ok \
                                -command [mymethod dispatch -okcommand]
        button $command.cancel  -text Cancel \
                                -command [mymethod dispatch -cancelcommand]
        button $command.help    -text Help \
                                -command [mymethod showHelp]
        grid $command.ok $command.cancel $command.help
        
        # Layout the dialog as a whole:
        
        grid $action  -sticky ew
        grid $command -sticky ew
    }
    #-------------------------------------------------------------------------
    #  Public methods
    #
    
    #
    #  modal  - Set the window to modal.  The application will
    #           run the event loop fromn this method and not return
    #           to the caller until it is destroyed.
    #
    method modal {} {
        if {$fakewin eq ""} {
            set fakewin [frame $win.fakewin]
            wm deiconify $win
            focus $win
            grab $win
            tkwait window $fakewin
        }
    }
    #
    #  dismiss - Used by a modal command handler to dismiss the widget.
    #
    method dismiss {} {
        if {$fakewin ne ""} {
            destroy $fakewin
        }
    }
    
    #-----------------------------------------------------------------------
    #  Private methods:
    #
    
    # Do % substitutions on a script.
    #   %W   -> $win
    #   %H   -> $options(-host)
    #   %U   -> $options(-user)
    # Parameters:
    #   script  - The script that will have substitutions applied.
    #
    method substitute {script} {
        regsub -all {%W} $script $win    script
        regsub -all {%H} $script $options(-host) script
        regsub -all {%U} $script $options(-user) script
        
        return $script
    }
    
    # dispatch  - Dispatches an event handler.
    #             The following script substitutions are available:
    #              %W  - The widget hull.
    #              %H  - The current value of the host name.
    #              %U  - The current value of the user name.
    # Parameters:
    #    reason  - Index into the options array that holds the script to
    #              be executed.  If $options($reason) is empty, this is a no-op.
    #              The script will be executed at the global level.
    #
    method dispatch {reason} {
        set script $options($reason)
        if {$script ne ""} {
            set script [$self substitute $script]         ;# substitute the %'s.
            set result [catch {uplevel 0 $script} message]
            if {$result} {
                bgerror $message                          ;# error from target script.
            }
        }
    }
    #  Display the help for the dialog.  This is going to be a frame that is
    #  tacked on to the bottom of the dialog that will be a scrolled html
    #  display and contain the text in options(-helptext)
    #  If the help frame already exists, this is a no-op.
    method showHelp {} {
        if {![winfo exists $win.help]} {
            set help [iwidgets::scrolledhtml $win.help]
            $help render $options(-helptext)
            grid $help
        }
    }
}
