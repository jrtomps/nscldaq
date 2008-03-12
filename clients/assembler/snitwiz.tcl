 # snitwiz.tcl --
 #
 #	This file implements package snitwiz, which is a wizard
 #       building package built with snit
 #
 # Copyright (c) 2004 Steve Cassidy
 #
 # Adapted and adopted by permission supplied in 
 # http://wikit.tcl.tk/13615
 #

package require snit
package require msgcat

if {[package present msgcat] != "package msgcat not present"} {namespace import msgcat::mc}

 # load message catalogs
 ::msgcat::mcload [file join [file dirname [info script]] .. msgs]

 package provide snitwiz 0.1

 namespace eval snitwiz {;
 #namespace export -clear *

     # create a default image
     image create photo snitwiz::feather -data {
	R0lGODlhIAAgALMAANnZ2QAAwAAA/wBAwAAAAICAgAAAgGBggKCgpMDAwP//
	/////////////////////yH5BAEAAAAALAAAAAAgACAAAAT/EMhJq60hhHDv
	pVCQYohAIJBzFgpKoSAEAYcUIRAI5JSFlkJBCGLAMYYIIRAI5ASFFiqDgENK
	EUIwBAI5ywRlyhAEHFKKEEIgEMgJyiwUBAGHnCKEEAyBQM4yy5RhCDikFDBI
	SSCQExRKwxBDjAGHgEFKQyCQk9YgxBBjDAGDnAQCOWkNQgwxxDgwyGkIBHJS
	GoQQYohRYJDTEAjkpDWIIYQQBQY5A4FATlqDEEIMgWCQMxgCgZy0BiikRDDI
	GQyBQE5aAxRSIhjkNIRAICetAQop04BBTgOBnLTKIIQQacAgZzAQyEkrCEII
	kQYMckoDgZy0giCESAMGOaWBQMoydeeUQYhUYJBTGgikLHNOGYRACQY5pYFA
	yjLnnEGgNGCQMxgAACgFAjnpFEUNGOQ0BgI5Z6FUFlVgkJNAICctlMqiyggB
	BkMIBHLOUiidSUEiJwRyzlIopbJQSilFURJUIJCTVntlKhhjCwsEctJqr0wF
	Y0xhBAA7
     }

 }; # end of namespace snitwiz

 snit::widget wizard {

     hulltype toplevel

     variable buttonFrame
     variable layoutFrame
     variable sep
     variable helpButton
     variable backButton
     variable nextButton
     variable cancelButton
     variable finishButton

     variable currentstep
     variable disabledSteps {}; # a list of steps that won't be shown

     option -title {Wizard}
     option -showhelp 1

     option -steps {}  ;# the order of steps

     # when we get new steps we make sure each knows it's parent
     # and set the current step to the first
     onconfigure -steps {value} {
	 set options(-steps) $value
	     foreach s $value {
		 $s configure -parent $self
	     }
	     set currentstep 0
     }

     constructor {args} {

	 # The dialog is composed of two areas: the row of buttons and the
	 # area with the dynamic content. To make it look the way we want it to
	 # we'll use another frame for a visual separator
	 install buttonFrame using frame $win.buttonFrame -bd 0
	 install layoutFrame using frame $win.layoutFrame -bd 0\
	     -height 200 \
	     -width 300
	 install sep using frame $win.sep1 -class WizSeparator\
	     -height 2 -bd 2 -relief groove

	 pack $layoutFrame -side top -expand 1 -fill both
	 pack $sep -side top -expand 0 -fill x
	 pack $buttonFrame -side top -expand 0 -fill x

	 # make all of the buttons
	 install helpButton using button $buttonFrame.helpButton \
	     -text [mc "Help"] \
	     -default normal \
	     -bd 1 \
	     -relief raised \
	     -command [mymethod help]

	 install backButton  using button $buttonFrame.backButton \
	     -text "< [mc Back]" \
	     -default normal \
	     -width 8 \
	     -bd 1 \
	     -relief raised \
	     -command [mymethod back]

	 install nextButton  using button $buttonFrame.nextButton \
	     -text "[mc Next] >" \
	     -default normal \
	     -width 8 \
	     -bd 1 \
	     -relief raised \
	     -command [mymethod next]

	 install finishButton using button $buttonFrame.finishButton \
	     -text [mc Finish] \
	     -default normal \
	     -width 8 \
	     -bd 1 \
	     -relief raised \
	     -command [mymethod finish]

	 install cancelButton using button $buttonFrame.cancelButton \
	     -text [mc Cancel]   \
	     -default normal \
	     -width 8 \
	     -bd 1  \
	     -relief raised \
	     -command [mymethod cancel]


	 # Must configure now so that this is all known prior to 
	 # layout.

	 $self configurelist $args


	 # pack the buttons
	 if {[$self cget -showhelp]} {
	     pack $helpButton -side left -padx 4 -pady 8
	 }
	 pack $cancelButton -side right -padx 4 -pady 8
	 pack $finishButton -side right -pady 8 -padx 4
	 pack $nextButton -side right -pady 8
	 pack $backButton -side right -pady 8

	 wm title $self [$self cget -title]
	 wm minsize $self 550 430

     }

     method layoutFrame {} {
	 return $layoutFrame
     }

     ## methods to handle the button commands
     method help {} {
	 event generate $self <<WizHelp>>
     }

     method next {} {
	 event generate $self <<WizNext>>
	 update idletasks

	 # go to the next non-disabled step
	 foreach step [lrange [$self cget -steps] [expr $currentstep+1] end] {
	     incr currentstep
	     if {[lsearch $disabledSteps $step] < 0} {
		 $self buildStep $step
		 return
	     }
	 }
     }

     method back {} {
	 event generate $self <<WizBack>>
	 update idletasks

	 ## go to the previous non-disabled step
	 for {incr currentstep -1} {$currentstep >= 0} {incr currentstep -1} {
	     set step [lindex [$self cget -steps] $currentstep]
	     if {[lsearch $disabledSteps $step] < 0} {
		 $self buildStep $step
		 return
	     }
	 }
     }

     method finish {} {
	 event generate $self <<WizFinish>>
     }

     method cancel {} {
	 event generate $self <<WizCancel>>
     }

     method buttonstate {name state} {

	 switch $name {
	     next {$nextButton configure -state $state}
	     back {$backButton configure -state $state}
	     finish {$finishButton configure -state $state}
	     cancel {$cancelButton configure -state $state}
	     help {$helpButton configure -state $state}
	 }
     }

     ## defining and rendering steps
     ## a step is an object of type wizardstep
     ## it is rendered by calling it's 'render' method
     ##

     method buildStep {step} {
	 # destroy the existing layout
	 eval destroy [winfo children $layoutFrame]

	 # run the step rendering method
	 $step render $layoutFrame $self

	 # enable/disable buttons as required
	 if {$currentstep >= [llength [$self cget -steps]]-1} {
	     # disable next
	     $self buttonstate next disabled
	 } else {
	     $self buttonstate next normal
	 }
	 if {$currentstep == 0} {
	     $self buttonstate back disabled
	 } else {
	     $self buttonstate back normal
	 }
     }

     ##  start --
     #
     # set the wizard going with the first step
     #
     method start {} {
	set currentstep 0
	$self buildStep [lindex $options(-steps) 0]
     }

     ## enable --
     #
     # enable a step, if previously disabled
     method enable {step} {

	 set pos [lsearch $disabledSteps $step]
	 if {$pos >= 0} {
	     set disabledSteps [lreplace $disabledSteps $pos $pos]
	 }
     }

     ## disable --
     #
     # disable a step so that it won't appear in the sequence
     #
     method disable {step} {

	 set pos [lsearch $disabledSteps $step]
	 if {$pos < 0} {
	     lappend disabledSteps $step
	 }
     }
 }

 snit::widget wizardLayout-basic {
     option -icon snitwiz::feather
     option -title ""
     option -subtitle ""
     option -pretext ""
     option -posttext ""

     variable titleframe
     variable title
     variable subtitle
     variable icon
     variable sep
     variable pretext
     variable posttext
     variable layoutFrame

     onconfigure -icon {value} {
	 $icon configure -image $value
	 set options(-icon) $value
     }

     onconfigure -title {value} {
	 $title configure -text $value
	 set options(-title) $value
     }

     onconfigure -subtitle {value} {
	 $subtitle configure -text $value
	 set options(-subtitle) $value
     }

     onconfigure -pretext {value} {
	 $pretext configure -text $value
	 set options(-pretext) $value
     }

     onconfigure -posttext {value} {
	 $posttext configure -text $value
	 set options(-posttext) $value
     }

     constructor {args} {

	 install titleframe using frame $win.tf -bd 4 -relief flat \
	     -background white
	 install title using label $titleframe.t\
	     -background white -width 40\
	     -text [$self cget -title]\
	     -anchor w -justify left
	 install subtitle using label $titleframe.st\
	     -height 2\
	     -background white\
	     -padx 15\
	     -width 40 \
	     -text [$self cget -subtitle]\
	     -anchor w -justify left
	 install icon using label $titleframe.icon\
	     -borderwidth 0 \
	     -image $options(-icon) \
	     -background white \
	     -anchor c
	 install sep using frame $win.sep -class WizSeparator\
	     -height 2 -bd 2 -relief groove

	 set labelfont [font actual [$title cget -font]]
	 $title configure -font [concat $labelfont -weight bold]

	 install pretext using label $win.pretext -width 40\
	     -text [$self cget -pretext]\
	     -anchor w -justify left

	 install layoutFrame using frame $win.layout -bd 0 -height 200
	 install posttext using label $win.posttext -width 40\
	     -text [$self cget -posttext]\
	     -anchor w -justify left

	 grid $title $icon -sticky ew
	 grid $subtitle ^ -sticky ew

	 grid $titleframe -sticky new
	 grid $sep -sticky new
	 grid $pretext -sticky ew
	 grid $layoutFrame -sticky nsew
	 grid $posttext -sticky ew

	 grid columnconfigure $win 0 -weight 1

	 grid rowconfigure $win 0 -weight 0   ;# titleframe
	 grid rowconfigure $win 1 -weight 0   ;# sep
	 grid rowconfigure $win 2 -weight 0   ;# pretext
	 grid rowconfigure $win 3 -weight 1   ;# layoutFrame
	 grid rowconfigure $win 4 -weight 0   ;# posttext

	 $self configurelist $args
     }

     method layoutFrame {} {
	 return $layoutFrame
     }
 }

 snit::type wizardstep {

     option -title {}
     option -subtitle {}
     option -icon snitwiz::feather
     option -pretext {}
     option -posttext {}
     option -body {}
     option -parent {}
     option -layout basic

     ## render method calls the body code to generate the frame
     method render {frame wizard} {
	 # first make the layout
	 set layout [wizardLayout-$options(-layout) $frame.wiz\
			 -title $options(-title)\
			 -subtitle $options(-subtitle)\
			 -icon $options(-icon)\
			 -pretext $options(-pretext)\
			 -posttext $options(-posttext)]

	 pack $layout -side top -fill both -expand 1

	 ## setup the special variable win as the container frame for
	 ## the content note that we also have $wizard as the name of
	 ## the containing wizard
	 set win [$layout layoutFrame]

	 # now render the step itself.  The step is assumed to
	 # be an object that has a renderForm method that takes
	 # as an argument the enclosing frame

	 set form [$self cget -body]
	 $form renderForm $win $wizard
     }
 }

 # require_all_keys --
 #
 #   Disable the Next button in a wizard unless all
 #   keys in the given array have values
 #
 # Arguments:
 #   wiz    -- a wizard widget
 #   arrayvar  -- array name
 #   keys   -- list of keys to check
 # Results:
 #   Puts a variable trace on the given
 #   array variables which will enable the next button
 #   only if all keys have non-null values.
 #
 proc snitwiz::require_all_keys {wiz arrayvar keys} {
     upvar $arrayvar array

     foreach k $keys {
	 trace add variable array($k) write \
	     [list snitwiz::require_all_keys_tracefn $wiz $keys]
     }
 }

 proc snitwiz::require_all_keys_cleanup {wiz arrayvar keys} {
     upvar $arrayvar array

     foreach k $keys {
	 trace remove variable array($k) write \
	     [list snitwiz::require_all_keys_tracefn $wiz $keys]
     }
 }

 # helper proc used by require_all_keys
 proc snitwiz::require_all_keys_tracefn {wiz keys arrayvar key op} {
     upvar $arrayvar array

     foreach k $keys {
	 if {$array($k) eq ""}  {
	     $wiz buttonstate next disable
	     return
	 }
     }
     # reset to normal only if all ok
     $wiz buttonstate next normal
 }
