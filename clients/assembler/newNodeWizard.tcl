#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2005.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Author:
#             Ron Fox
#	     NSCL
#	     Michigan State University
#	     East Lansing, MI 48824-1321

package provide NodeWizard 1.0
package require Tk
package require snit
package require snitwiz
package require EnterNode
package require EnterReadout
package require NodeTimingEntry

#
#   This package provides a wizard that allows you to 
#   define the characteristics of a new node in the event builder.
#   No attempt is made to validate what the user filled in, however.
#   
#  This file actually contains several snit types.
#  Each type with a name of the form stepSomething
#  is a wizard step  newNodeWizard is the wizard object itself.
#  The wizard object composes itself from all the step objects.
#  The step objects are 'derived' from wizardstep objects
#  so that they can hold the additional state needed to pass
#  information from the wizard back to the user.
#  I say 'derived' rather than derived because snit uses delegation.
#  derivation is simulated by delegating all unknown methods to the
#  'base class'.
#
#  The steps are hidden in the newNode namespace.  
#  The wizard itself is in the global namespace.
#


namespace eval ::newNode {

}

#----------------------------------------------------------------
#
#  stepNode - Provides a wizard step that prompts for
#  the node name and id of the node:
# 
#  OPTIONS:
#   -node      - Specifies the node dns or IP address.
#   -id        - Specifies the id of the node's data.
# 
# NASTY TRICK:
#   We cannot create the EnterNode form until the wizard
#   starts our step.. But we want the user to be able to 
#   configure/cget -node/-id before and after the step.
#   The variable rendered  will keep track of 
#   whether configure/cget have a nodeForm component to
#   manually delegate to.
#

snit::type ::newNode::stepNode {
    component nodeForm;			# The form for entering the node info.
    component step

    option -node ""
    option -id  

    delegate option *  to step
    delegate method *  to step


    variable rendered false

    constructor args {
	install step     using wizardstep %AUTO% \
	    -title {Node information} \
	    -pretext {Fill in node information below:}   \
	    -body $self

	$self configurelist $args
    }
    #
    # Destroys the components..
    #

    destructor {
	#
	# the catches are because I don't know how much
	# the wizard has destroyed already (e.g. if our step is not
	# executing, the subwidgets of the form may have been destroyed.
	#
	catch {$step destroy}
	catch {$nodeForm destroy}
    }
    # Render the form in the enclosing window:

    method renderForm {w wiz} {
	install nodeForm using EnterNode $w.nodeform -node $options(-node) -id $options(-id)
	grid $nodeForm -sticky nsew

	set rendered true
	bind $wiz <<WizNext>> [mymethod stepOut]
    }
    #
    #  if we step to another step, we need to load our options
    #  from the widget before it gets destroyed:
    #
    method stepOut  {} {
	set rendered false

	foreach option [list -node -id] {
	    set options($option) [$nodeForm cget $option]
	}
    }
    #  Configures.. pass through to the nodeForm if it exists.
    #

    method configPassThrough {option value} {
	set $options($option) $value
	if {$rendered} {
	    $nodeForm configure $option $value
	}
    }

    onconfigure -node value {
	$self configPassThrough -node $value
    }
    onconfigure -id value {
	$self configPassThrough -id $value
    }


    # Cgetters.. pass through tothe nodeForm if it has been rendered.
 
    method cgetPassThrough option {
	if {$rendered} {
	    set options($option) [$nodeForm cget $option]
	}
	return $options($option)
    }
    
    oncget -node {
	return [$self cgetPassThrough -node]
    }
    oncget -id {
	return [$self cgetPassThrough -id]
    }
}
#---------------------------------------------------------------------------------
#
#  stepReadout - Provides a wizard step that prompts for the
#                path to the readout program and the command line arguments
#                that should be passed to it when it's run.
#
# OPTIONS:
#   -path   -  Path to the readout program.
#   -args   -  Arguments to pass the readout program.
#
# NASTY TRICK:
#
#   See stepNode with regard to  option handling.. it's the same old same old here.
#

snit::type  newNode::stepReadout {
    component readoutForm
    component step

    option   -path
    option   -args

    delegate option * to step
    delegate method * to step

    variable rendered false

    constructor args {
	install step using wizardstep %AUTO% \
	    -title {Readout program} \
	    -pretext {Fill in information about the readout program below} \
	    -body $self

	$self configurelist $args
    }

    destructor {
	catch {$step destroy}
	catch {$readoutForm destroy}
    }

    #  Render the form into the window:

    method renderForm {w wiz} {
	install readoutForm using EnterReadout $w.readoutform -path $options(-path) \
	                                               -args $options(-args)
	grid $readoutForm
	set rendered true

	bind $wiz <<WizNext>> [mymethod stepOut]

    }
    #
    #  Stepping out means we need to copy the form's options locally
    #  before the form is destroyed:
    #
    method stepOut {}  {

	set rendered false

	foreach option [list -path -args] {
	    set options($option) [$readoutForm cget $option]
	}
    }

    # Configures:

    method configPassThrough {option value} {
	set $options($option) $value
	if {$rendered} {
	    $readoutForm configure $option $value
	}
    }


    onconfigure -path value {
	$self configPassThrough -path $value
    }
    onconfigure -args value {
	$self configPassThrough -args $value
    }

    method cgetPassThrough option {
	if {$rendered} {
	    set options($option) [$readoutForm cget $option]
	}
	return $options($option)
    }

    oncget -path  {
	return [$self cgetPassThrough -path]
    }
    oncget -args {
	return [$self cgetPassThrough -args]
    }
}

#------------------------------------------------------------------------------

# stepTiming - this step allows the user to enter the match criteria for the 
#              object.
# OPTIONS:
#   -istrigger       - Bool -true if this node is trigger false otherwise.
#   -matchwindow     - Width of matching window in time stamp ticks.
#   -matchoffset     - Offset to apply to timestamp matching window i timestamp ticks.
#
# NASTY BUGGERY:
#   See the notes on the previous steps about handling option propagation.
#

snit::type newNode::stepTiming {
    component timingForm
    component step

    option -istrigger    0
    option -matchwindow  1
    option -matchoffset  0

    delegate method * to step
    delegate option * to step

    variable rendered false

    constructor args {
	install step using wizardstep %AUTO% \
	    -title {Fragment match window}   \
	    -pretext {Fill in the fragment matching criteria below} \
	    -body $self

	$self configurelist $args
    }
    destructor {
	catch {$step destroy}
	catch {$timingForm destroy}
    }

    # Render the step into its frame.

    method renderForm {w wiz} {
	install timingForm using nodeTimingEntry $w.timingform -istrigger   $options(-istrigger) \
	                                                 -matchwindow $options(-matchwindow) \
 	                                                 -matchoffset $options(-matchoffset)
	grid $timingForm -sticky nsew

	set rendered true

	bind $wiz <<WizNext>> [mymethod stepOut]
    }
    
    # Stepping out of the widget means it will soon be destroyed,
    # We must fetch the settings from the form local:
    #
    method stepOut {} {

	set rendered false

	foreach option [list -istrigger -matchwindow -matchoffset] {
	    set options($option) [$timingForm cget $option]
	}
    }

    # Now the configuration pass throughs:

    method configPassThrough {option value} {
	set $options($option) $value
	if {$rendered} {
	    $timingForm configure $option $value
	}
    }

    onconfigure -istrigger value {
	$self configPassThrough -istrigger $value
    }
    onconfigure -matchwindow value {
	$self configPassThrough -matchwindow $value 
    }
    onconfigure -matchoffset value {
	$self configPassThrough -matchoffset $value
    }

    #  The cget delegations:

        method cgetPassThrough option {
	if {$rendered} {
	    set options($option) [$timingForm cget $option]
	}
	return $options($option)
    }


    oncget -istrigger {
	return [$self cgetPassThrough -istrigger]
    }
    oncget -matchwindow  {
	return [$self cgetPassThrough -matchwindow]
    }
    oncget -matchoffset {
	return [$self cgetPassThrough -matchoffset]
    }

}

#------------------------------------------------------------------------------------
#
#  "We're off to see the wizard....
#   The wonderful wizard of oz!
#                   - Harold Arlen
#
#  The wizard class encapsulates the three steps above,
#  delegating the options to the various steps and amalgamating them into a wizard.
# 
# OPTIONS
#  -node        - The node name/IP address.
#  -id          - The assembly id of the node.
#  -path        - Path to the readout program to run in that node.
#  -args        - Command arguments passed to the readout program when it is run.
#  -istrigger   - True if the node is the trigger node.
#  -matchwindow - Width of the matching window
#  -matchoffset - Match window offset.
#  
# Methods:
#   start       - Starts the wizard. The wizard is considered modal, and
#                 control will remain here until the wizard is completed or cancelled.
#   canceled    - Returns true if the wizard completed on cancellation
#
snit::type newNodeWizard {
    component nodeForm
    component readoutForm
    component timingForm
    component wizard

    # Option delegation:

    delegate option -node to nodeForm
    delegate option -id   to nodeForm

    delegate option -path to readoutForm
    delegate option -args to readoutForm

    delegate option -istrigger   to timingForm
    delegate option -matchwindow to timingForm
    delegate option -matchoffset to timingForm

    delegate option * to wizard

    variable hiddenFrame
    variable completionState

    typevariable wizardIndex 0

    constructor args {
	install nodeForm    using ::newNode::stepNode    %AUTO%
	install readoutForm using ::newNode::stepReadout %AUTO%
	install timingForm  using ::newNode::stepTiming  %AUTO%

	$self CreateWizard


	$self configurelist $args
    }

    destructor {
	catch {destroy $nodeForm}
	catch {destroy $readoutForm}
	catch {destroy $timingForm}
	catch {destroy $wizard}
    }
    # Create the wizard (factorization).
    #
    method CreateWizard {} {
	install wizard      using wizard .wiz$wizardIndex -steps [list $nodeForm $readoutForm $timingForm] \
	    -showhelp 0
	wm withdraw $wizard
	incr wizardIndex
    }


    # Start the wizard:
    #   - Display the wizard.
    #   - create a hidden frame whose destruction marks the end of the wizard's execution.
    #   - establish handlers for the  <<Finish>> and <<Cancel>> events.
    #   - grab to start modality 
    #   - wait on the destruction of the hidden frame.
    #   - withdraw the wizard... if it was destroyed, recreate it.
    #   - Report the return status to the user.
    #  
    
    method start {} {
	wm deiconify $wizard

	set hiddenFrame [frame $wizard.hidden$self]

	bind $wizard <<WizFinish>> [mymethod Finished]
	bind $wizard <<WizCancel>> [mymethod Cancelled]

	$wizard start;			# display the first page.

	focus $wizard
	grab $wizard

	tkwait window $hiddenFrame;	# modal now.

	if {![winfo exists $wizard]} {
	    $self CreateWizard
	    set completionState Cancelled
	} else {
	    grab release $wizard
	    wm withdraw  $wizard
	}

	return $completionState
    }
    #  Called when the finish button is whacked:

    method Finished {} {
	set completionState Finished
	destroy $hiddenFrame
    }
    # Called when the Cancelled buttin s whacked.
    
    method Cancelled {} {
	set completionState Cancelled
	destroy $hiddenFrame
    }
}
