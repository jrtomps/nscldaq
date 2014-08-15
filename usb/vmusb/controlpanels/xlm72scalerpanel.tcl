##
# xlm72scalerpanel.tcl
#
#
# Herein contains the source code for running the XLM72ScalerGUI.
# It is only half of the application as it requires the user to
# to have setup a VMUSBReadout to have registered an instance of
# the XLM72ScalerControl to the slow controls server as a tcl 
# module. 
# 
# The code is broken up into two distinct classes. The XLM72ScalerGUI
# class is an itcl megawidget that provides the UI widgets and layout.
# It owns a reference to an XLM72SclrControl class that handles the
# interaction of the GUI with the VMUSBReadout slow controls server. 
# The XLM72ScalerGUI is not purely a passive view, but rather handles
# the logic for refreshing itself when live updating is enabled.  
# 
#

package provide xlm72scalerpanel 1.0

package require Itcl
package require snit 
package require Tk 
package require usbcontrolclient 
package require TclServer
package require portAllocator 
package require InstallRoot
package require ScalerClient

# Set a default runstate in case the sclclient has
# not been able to tell us anything yet. 
set RunState "*Unknown*" 


#####################################################################
####################################################################
#
# Some procs for the TclServer
#

proc _OnTclServerConnect {fd ip port } {
  puts "TclServer connection request $ip:$port"
  gets $fd user
  puts $user
  return true
}

proc _OnTclServerError {chan command msg } {
  puts "Error when executing \"$command\" : $msg"
  return true
}
proc startServer {} {
  ::portAllocator create allocator 
  set port [allocator allocatePort XLM72ScalerGUI]
  set ::server [TclServer %AUTO% -port $port -onconnect _OnTclServerConnect]
  $::server configure -onerror _OnTclServerError
  $::server start
  
  return $port
}

########################################################################
########################################################################
########################################################################

## XLM772ScalerGUI UI 
#
# Provides the main UI for the XLM72ScalerGUI. It also handles the logic
# for performing live updates. The class sends all requests created by 
# user actions to the mediator (an instance of XLM72SclrControl) that
# ultimately performs the communication with the VMUSBReadout slow controls
# server. On exit, this also stores the state of itself into a file
# that then is reloaded when constructed again.
#
# 
#

itcl::class XLM72ScalerGUI {
  
  private variable parentWidget
  private variable mediator

  private variable around 
	private variable cancel ""

	public variable scaler
	public variable rate
	public variable enable
	public variable trigger
	public variable wrap 
	public variable name 
	public variable live
	public variable frequency

  constructor {parent modulename host port} {
    set parentWidget $parent
		set enable 0
	  set live 0
    set frequency 1
		set around [expr 1<<24]
		for {set i 0} {$i < 32} {incr i} {
  		set scaler($i) 0
			set rate($i) 0
  	 	set trigger($i) 0
  	 	set wrap($i) 0
		}


		# look for module and update from it
    set mediator [::XLM72SclrGUICtlr %AUTO% -host $host -port $port -name $modulename]
    $this BuildGUI $parentWidget
    $mediator configure -widget $this 

    # First let's see if it even makes sense to run this thing
    if {[IsGoodFirmware 0xdaba0002]} {
        UpdatePanel 1
    } else {
        tk_messageBox -icon error \
                   -message "Failed to read good firmware id from module. Maybe the firmware needs to be loaded?"
        exit
    }

    # look for a file saved from before 
    LoadSavedSettings 
    
  }
 
  method IsGoodFirmware {signature}
  method BuildGUI {parent}
  method BuildPanel {parent name offset}
  method BuildPanels {parent name}
  method BuildSclrControlBox {parent name}
  method BuildUpdateControlBox {parent name}
  method SetupStyle {}
  method GetParent {} {return $parentWidget} 
  method SaveSettings {}
  method UpdatePanel {live}
  method SetLive {}
  method OnExit {}
  method SetChildrenState {state}
  method UpdateValues {}
  method ParseTriggerRegister {value}
  method OnReset {}
  method OnEnable {}
  method OnTriggerToggle {ch}

  method GetTopFrame {} {return $parent.topframe}
  method LoadSavedSettings {} 
}

itcl::body XLM72ScalerGUI::IsGoodFirmware {signature} {
  set fw [$mediator GetFirmware]
  if {[string is integer -strict $fw]} {
    if {$fw==$signature} {
      return 1 
    }  else {
      return 0
    }
  } else {
    return 0
  }
}

itcl::body XLM72ScalerGUI::LoadSavedSettings {} {
  if {[file exists "XLM72Scaler_[string trimleft $this :].tcl"]} {
    source "XLM72Scaler_[string trimleft $this :].tcl"
  } else {
    for {set i 0} {$i < 32} {incr i} {
      set wrap($i) 0
      set name($i) ""
    }
  }
  # Establish consistency
  if {$enable} {
    $this OnEnable
  }

  if {$live} {
    $this SetLive
  }
}

itcl::body XLM72ScalerGUI::BuildGUI {parent} {
  puts "Parent : $parent"
  ## Build the left column
  if {$parent eq "." } { set top "" }
  set top $parent.topframe
  ttk::frame $top
  
  SetupStyle

  # Build the big pieces
  set panels [BuildPanels $top panels]
  set sclrctrls [BuildSclrControlBox $top sclrctrl]
  set updatectrls [BuildUpdateControlBox $top updatectrl]

  SetChildrenState active

  ## Build the button box
  set w $top.buttons
  ttk::frame $w 
  ttk::button $w.exit -text Exit -command "$this OnExit"

  grid $w.exit -sticky news -padx 3 -pady 6
  grid rowconfigure $w 0 -weight 1 
  grid columnconfigure $w 0 -weight 1 

  ## Put it all together
  grid $panels -columnspan 2 -sticky news -padx 3
  grid $sclrctrls $updatectrls -sticky nsew -padx 3 -pady 6
  grid $top.buttons -columnspan 2 -sticky nsew

  ## Grid the frame
  grid $top -sticky nsew  -padx 6 -pady 6
  grid columnconfigure $top 1 -weight 1
  grid rowconfigure $top 1 -weight 0
}

itcl::body XLM72ScalerGUI::SetupStyle {} {
  ttk::style configure TFrame -background lightblue
  ttk::style configure TLabel -background lightblue
  ttk::style configure TLabelframe -background lightblue
  ttk::style configure TLabelframe.Label -background lightblue
  ttk::style configure TCheckbutton -background lightblue

  ttk::style map TFrame -background [list disabled lightblue active lightblue]
  ttk::style map TLabel -background [list disabled lightblue active lightblue]
#  ttk::style map TLabelframe -background [list disabled lightblue active lightblue]
#  ttk::style map TLabelframe.Label -background [list disabled lightblue active lightblue]
  ttk::style map TCheckbutton -background [list disabled lightblue active lightblue]


  puts "Labelframe layout : [ttk::style layout TLabelframe]"
}

itcl::body XLM72ScalerGUI::BuildPanel {parent name offset} {
  set top $parent 

  set w $top.$name
  ttk::frame $w 

  ttk::label $w.chanLbl -text "Ch#"
  ttk::label $w.nameLbl -text "Name"
  ttk::label $w.rateLbl -text "Rate"
  ttk::label $w.totalLbl -text "Total"
  ttk::label $w.enableLbl -text "Trig"

  set begin $offset
  set nrows 16
  set end [expr $offset+$nrows]
  for {set i $begin} {$i < $end} {incr i} {

    set trigger($i) 0
    ttk::label $w.ch$i -text [format "%02d" $i] -width 2
    ttk::entry $w.name$i   -textvariable [itcl::scope name($i)] -width 8 \
                      ;#-validatecommand SaveSettings
    ttk::label $w.scaler$i -textvariable [itcl::scope scaler($i)]  \
                          -padding 2 -width 10 -anchor e
    ttk::label $w.rate$i   -textvariable [itcl::scope rate($i)] \
                          -padding 2 -width 8 -anchor e
    ttk::checkbutton $w.trigger$i -variable [itcl::scope trigger($i)] \
                             -command "$this OnTriggerToggle $i" \
                             -takefocus 0

  }

  ## Grid the panel
  grid $w.chanLbl $w.nameLbl $w.totalLbl $w.rateLbl $w.enableLbl -stick nsew
  for {set i $begin} {$i < $end} {incr i} {
    grid $w.ch$i $w.name$i $w.scaler$i $w.rate$i $w.trigger$i -sticky news
    grid rowconfigure $w [expr ($i-$begin)+1] -weight 1
  }
  grid columnconfigure $w {1 2 3} -weight 1 


  return $w
}

itcl::body XLM72ScalerGUI::BuildPanels {parent name} {
  set frmname $parent.$name

  ttk::frame $frmname

  set leftPanel [$this BuildPanel $frmname s1 0]
  set rightPanel [$this BuildPanel $frmname s2 16]
  ttk::separator $frmname.vs -orient vertical

  grid $leftPanel $frmname.vs $rightPanel -sticky news -padx 3

  return $frmname
}

itcl::body XLM72ScalerGUI::BuildSclrControlBox {parent name} {

  set w $parent.$name

  # Create the frame and configure it
  ttk::labelframe $w -text "Scaler Controls"
  set lblfrmClass [winfo class $w]

  # make the widgets
  ttk::checkbutton $w.enable -text "Enable" -variable [itcl::scope enable] \
                        -command "$this OnEnable" 
  ttk::button $w.reset -text "Reset Scalers" -command "$this OnReset"

  grid $w.enable $w.reset -sticky nsew -padx 3 -pady 3
  grid columnconfigure $w {0 1} -weight 1 

  return $w
}

itcl::body XLM72ScalerGUI::BuildUpdateControlBox {parent name} {

  set w $parent.$name

  # create the frame and configure the background color
  ttk::labelframe $w  -text "Live Update"

  # create the widgets
  ttk::checkbutton $w.live -text "Enable" -variable [itcl::scope live] \
                      -command "$this SetLive"
  puts "Checkbutton class : [winfo class $w.live]"
  ttk::spinbox $w.freq -textvariable [itcl::scope frequency] -width 3 -increment 1 \
                  -from 1 -to 10
  ttk::label $w.freqLbl -text "Update Period (s)"

  # grid it 
  grid $w.live $w.freqLbl $w.freq -sticky nsew -padx 3 -pady 3
  grid columnconfigure $w 0 -weight 1

  return $w
}

itcl::body XLM72ScalerGUI::SaveSettings {} {
  set file [open "XLM72Scaler_[string trimleft $this :].tcl" w]
  for {set i 0} {$i < 32} {incr i} {
    puts $file "set trigger($i) $trigger($i)"
    puts $file "set name($i) \"$name($i)\""
    puts $file "set wrap($i) $wrap($i)"
  }
  puts $file "set enable $enable"
  puts $file "set live $live"
  puts $file "set frequency $frequency"
  close $file
  return 1
}

##
# UpdatePanel
#
# THis handles the updates to the GUI. In this case, it will relay an Update
# call through the mediator and then retrieve the updated values through some
# Get commands
#
itcl::body  XLM72ScalerGUI::UpdatePanel {once} {
  global RunState
#	set top ".aXLM72Scaler_[string trimleft $this :]"

  set active [expr {$::RunState eq "Active"}]
  if {$active} {
    SetChildrenState disabled
  } else {
    # if the period between refreshes is shorter than it takes
    # for data to be observed by the sclclient and set RunState 
    # us, then we end up getting through to the slow controls server
    # when the run is active. This then pauses the run and it is 
    # annoying. So we can at least prevent it from happening more than once
    # by asking for the state.
    set state [$mediator GetRunState]
    if { ($state ne "idle") && ($state ne "paused")} {
      # Oops we are running and weren't alerted yet. 
      # Update properly and then set the RunState manually
      SetChildrenState disabled 
      set ::RunState Active
    } else {

      SetChildrenState "!disabled"
      UpdateValues ;# Update the values
    }
  }

  # If needed reschedule the next update
  if {!$once} {
    set cancel [after [expr $frequency*1000] "$this UpdatePanel 0"]
  }
}

# Update state of child widgets
itcl::body XLM72ScalerGUI::SetChildrenState {state} {
  set top [GetParent].topframe 
#  foreach c [winfo children $top.panels] {$c configure -state $state}
  foreach c [winfo children $top.panels.s1] {$c state $state}
  foreach c [winfo children $top.panels.s2] {$c state $state}
  foreach c [winfo children $top.sclrctrl] {$c state $state}
  foreach c [winfo children $top.updatectrl] {$c state $state}
  update
}

itcl::body XLM72ScalerGUI::UpdateValues {} {
    $mediator Update
    set enable  [$mediator GetEnable]
    set trigreg [$mediator GetAllTriggers]
    ParseTriggerRegister $trigreg
    set values   [$mediator GetAllSclrValues]
    if {[llength $values] != 32 } {
      tk_messageBox -icon error -message "Failed to read all 32 scaler values"
    } else {
      for {set i 0} {$i < 32} {incr i} {
        set value [lindex $values $i]
        # determine whether the number has wrapped
        if {$value < $scaler($i)} {
          incr wrap($i)
        }
        # compute the new scaler value
        set updated [expr $wrap($i)*$around + $value]
        set rate($i) [format %.1f [expr ($updated-$scaler($i))/$frequency]]
        set scaler($i) $updated
      }
    }
}

itcl::body XLM72ScalerGUI::SetLive {} {
  if {$live} {
    $this UpdatePanel 0
  } else {
    after cancel $cancel
  }
}

itcl::body XLM72ScalerGUI::OnExit {} {
  after cancel $cancel
  SaveSettings
  $mediator OnExit
  destroy [GetParent] 
}

itcl::body XLM72ScalerGUI::ParseTriggerRegister {value} {
	for {set i 0} {$i < 32} {incr i} {
		set trigger($i) [expr ($value&(1<<$i))>>$i]
	}
}

itcl::body XLM72ScalerGUI::OnReset {} {
  $mediator Reset
	for {set i 0} {$i < 32} {incr i} {
		set wrap($i) 0
		set scaler($i) 0
		set rate($i) 0
	}
}

itcl::body XLM72ScalerGUI::OnEnable {} {
  $mediator SetEnable $enable
}

itcl::body XLM72ScalerGUI::OnTriggerToggle {ch} {
  $mediator SetTrigger $ch $trigger($ch)
}





###############################################################################
###############################################################################
###############################################################################
###############################################################################
###############################################################################


##
# Controller for XLM72ScalerGUI
#
# This is ultimately the piece that interacts with VMUSBReadout program.
# Any time a request is made in the GUI, it causes this to send out 
# requests to the VMUSBReadout slow controls server. Communication is performed
# through the usb controlclient provided by the usbcontrolclient package.
#
# It also manages a sclclient program that it starts up in the background. When
# the OnExit method is called, which should happens when the GUI's exit button is
# pressed, it kills off this process. To ensure that when the "x" button on the
# window (i.e. the window close button) this method is called, it is important
# to set the "wm protocol win_name WM_DELETE_WINDOW { script }" to ultimately
# call the OnExit of this. Usually this is done through the GUI's OnExit proc.
#
#
snit::type XLM72SclrGUICtlr {

  option -host       -default localhost
  option -port       -default 27000
  option -connection -default ""
  option -name       -default ""
  option -widget     -default ""
  option -onlost     -default ""
  
  variable sclclientPID ""

  constructor args {
    variable sclclientPID

    $self configurelist $args
    
    $self Reconnect $options(-host) $options(-port)
    set sclclientPID [startScalerClient localhost $::port \
                                        localhost $::env(USER)]
  }
  
  method OnExit {} {
    variable sclclientPID
    puts $sclclientPID
    exec kill -9 $sclclientPID
  }
  method SetEnable {enable} {
    $self Set [list enable $enable]
  }

  method SetTrigger {ch val} {
    $self Set [list [format "trigger%d" $ch ] $val]
  }

  method Reset {} {
    $self Set [list reset 1]
  }

  method GetFirmware {} {
    return [$self Get [list firmware]]
  }

  method GetEnable {} {
    return [$self Get [list enable]]
  }

  method GetAllTriggers {} {
    return [$self Get [list alltriggers]]
  }

  method GetAllSclrValues {} {
    return [$self Get [list allscalers]]
  }

  method GetRunState {} {
    return [$self Get [list runstate]]
  }

  method Reconnect {host port} {

    while {[catch {controlClient %AUTO% -server $host -port $port} connection]} {
      tk_messageBox -icon info \
        -message "The control panel requires readout be running, please start it: $connection"
    }
    set options(-connection) $connection
  }

# perform  transactions with the device.
#

  method Get args {
    set name $options(-name)
    set conn $options(-connection)
    set value [$self transaction [concat [list $conn Get $name] [lindex $args 0]]]
    return $value
  }

  method Set {args}  {
    set name $options(-name)
    set conn $options(-connection)
    set args [lindex $args 0] 
    set command [concat [list $conn Set $name] $args]
    return [$self transaction $command]

  }
  method Update {} {
    set name $options(-name)
    set conn $options(-connection)

    return [$self transaction [list $conn Update $name]]
  }


  method transaction {script} {
# loop until transaction works or error signalled.

    while {[catch {eval $script} msg] && ($msg ne "")} {
      if {$options(-onlost) ne ""} {# comm fail handler..
        eval $options(-onlost) $options(-connection)
        set script [lreplace $script 0 0 $options(-connection)]
      } else {			# no handler.
        error "Communication failure $msg"
      }
    }

    if {[string is list $msg]} {
      if {[lindex $msg 0] eq "ERROR"} {
        puts "THE CLIENT RECEIVED ERROR"
          error $msg;			# Transaction error.
      }
    }

  	return $msg

  }


}

##
# Some procs that must exist for state changes.
#
# These are called by sclclient and must exist somewhere. 
# We really don't care to do anything with them so they are
# just empty procs.
#
proc Update {} {}
proc BeginRun {} {}
proc EndRun {} {}
proc PauseRun {} {}
proc ResumeRun {} {}
proc RunInProgress {} {puts "RUN IN PROGRESS"}
proc UpdateRunTime {src time} {}

set port [startServer ]
puts "TclServer started on port = $port"
