#!/bin/sh

# start wish \
exec wish ${0} ${@}



tk_messageBox -type ok -icon warning -title {Obsolete software} \
    -message {You are running an obsolete version of the shaperapp.tcl script 
The current versions of the shaper application software are now stored in
$DAQROOT/Scripts/ControlApplications}
#
#  SEE application for shaper control.
#  Source in configuration files build the appropriate set of GUI's.
#  Has all the code that doesn't need to know about gui widgets
#  that are globally known.  This allows the 8/16 channel versions
#  to be common except for the ChannelCount variable.



set base 0x100000;			# Default base address of controller.
if {[info var crate] == "" } {
    set crate 0;				# Default base address of crate.}
if {[info var ChannelCount] == ""} {
    set ChannelCount 8;			# Default# channels controlled.
}

set Controllers("")     "-null-"
set Aliases("")          "-null-"



proc ResetDefaults {} {
    global outconfig outpolarity shapetime coarsegain finegain pole0 offset

    for {set i 0} {$i < 16} {incr i } {
	set outconfig($i)     0
	set outpolarity($i)   1
	set shapetime($i)     1
	set coarsegain($i)    0
	set finegain($i)      0
	set pole0($i)         0
        set offset            0
    }
}


proc KillMe {} {
    global argv0
    set name [file tail $argv0]
   set psout [exec ps axuww | grep $name  | grep -v grep ]
   set processes [split $psout "\n"]
   foreach process $processes {
      set pid [lindex $process 1]
      catch "exec kill -9 $pid"
   }
}

#
#
#  Exit with confirmation prompt:
#
proc Exit {}  {
   global Config
   global Controllers
   set confirm [tk_dialog  .confirm "Exit Confirmation" \
				   "Are you sure you want to exit"  \
				   questhead 1 Yes No ]
   if {$confirm == 0} {
     # Close the controllers:
     
     foreach controller [array names Controllers] {
	    catch "caennet::delete $controller"
     }
     #  Not sure why but exit hangs so we kill ourself.

      KillMe
   }
   
}
#
#  Enable/Disable the mux depending on the
# value of Config($name.muxenable)
#
proc SetMuxOnOff {name} {
   global Config
   set state $Config($name.muxenable)
   if {$state} {
      n568b::EnableMuxOut $Config($name.controller) \
                                    $Config($name.node) 
   } else {
      n568b::DisableMuxOut $Config($name.controller) \
                                    $Config($name.node) 
   }
 }
#
#   Select a channel for the mux.  This is done by just
#   requesting the value of the parameters for the channel.
#
proc SelectChannel {name} {
   global Config
   set controller $Config($name.controller)
   set node       $Config($name.node)
   set channel   $Config($name.muxsel)
   
   SetMuxOnOff $name
   n568b::SetOutputConfiguration $controller $node $channel \
	   $Config($name.outcfg.$channel)
   
}
#
#   Procedure to configure an output channel to be either
#   normal or inverting.
#
proc ConfigureOutput {name chan} {
   global Config
   
   set controller $Config($name.controller)
   set node       $Config($name.node)
   set onoff       $Config($name.outcfg.$chan)
   
   n568b::SetOutputConfiguration $controller $node $chan $onoff
   
   SelectChannel $name

}
#
#  Procedure to set the output polarity of the module.
#

proc SetPolarity {name channel} {
   global Config
   set controller $Config($name.controller)
   set node       $Config($name.node)
   set value      $Config($name.outpol.$channel)
   
   
   n568b::SetPolarity $controller $node $channel $value
   
   SelectChannel $name
   
}
#
#   Set the shaping time for the specified channel:
#
proc SetShapingTime {name channel} {
    global Config
    
    set controller $Config($name.controller)
    set node       $Config($name.node)
    set value      $Config($name.shapetime.$channel)
    
    n568b::SetShapingTime $controller $node $channel $value
    
    SelectChannel $name
}
#
#  Set the coarse gain on a channel.
#  In addition to setting the hardware, the label in the associated
#  widget has to be set to the appropriate gain factor.
#
proc SetCoarseGain {name channel widget value} {  
   global Config
   
   set controller $Config($name.controller)
   set node       $Config($name.node)
   set value       [expr $value]
   
   set actual [expr 1 << $value]
   
   #  Update the value of the widget's label to reflect the
   #  actual gain.
   
   $widget configure -label $actual
   
   # Set the device.
  
   n568b::SetCoarseGain $controller $node $channel $value
   
   SelectChannel $name
}
#
#   Set the fine gain on a channel.  The label of the associated scale
#   is just the raw register value.
#

proc SetFineGain {name channel widget value} {
   global Config
   set     controller $Config($name.controller)
   set     node       $Config($name.node)
   set     value       [expr int($value)]
   set     displayvalue $value
   
   #  Update the widget's label:
   
   $widget configure -label $displayvalue
   
   # Set the device
   
   n568b::SetFineGain $controller $node $channel $value
   
   SelectChannel $name
}

#
#   Set the module offset.  The offset
#  is represented as 0 - 255 in the slider, but we adjust the
#  label so that it shows -75 - +210mv
#

proc SetOffset {name widget value} {
	global Config
	
	set controller $Config($name.controller)
        set node       $Config($name.node)
	set value      [expr int($value)]
	
	set mv [expr $value*285.0/255.0 -75.0]
	set mv [expr int($mv*100.0)/100.0]
	
	$widget configure -label "$mv mv"
	
	n568b::SetOffset $controller $node $value
	
	SelectChannel $name   ;# Not really needed.
}
#
#  Set a channel pole 0 adjust value.
#

proc SetPole0 {name channel value} {
   global Config
   set controller $Config($name.controller)
   set node       $Config($name.node)
   
   n568b::SetPole0 $controller $node $channel $value
   
   SelectChannel $name
   
}

#
#   If necessary,  create a new controller given the name and
#   and other info. Two arrays are used:
#    Controllers  - indexed by actual controller names
#                            contains the {base crate} list.
#    Aliases         - Indexed by aliased controllers, contains
#                             the name of the actual controller first
#                             created at the address described.
# Returns
#    The primary name of a caennet controller.
#  It is an error to assign the same alias to two controllers.
#  It is an error to try to change the base/crate of an existing
#  controller.

proc UpdateControllers  {name base crate}  { 
    global Controllers 
    global Aliases
    #
    # If the name is in the Controllers array try that:
    #
    if {[array names Controllers $name]  != ""} {
	set actualbase  [lindex $Controllers($name) 0]
	set actualcrate [lindex $Controllers($name) 1]
	if {($actualbase != $base) || ($actualcrate != $crate)} {
	     error "Attempted to redefine $name ($actualbase $actualcrate) \
	     as ($base $crate)"
        } else {
	   return $name
        }
    }
    # If the name is in the alias list already, then ensure that the
    # caller isn't trying to redefine again:
    #
    if {[array names Aliases $name] != ""}  {
	set ActualName $Aliases($name)
	set ActualBase  [lindex $Controllers($ActualName) 0]
	set ActualCrate [lindex $Controllers($ActualName) 1]
        if {($ActualBase != $base) || ($ActualCrate != $crate)} {
            error "Attempted to redfine $name ($ActualBase $ActualCrate) \
                   as ($base $crate)"
        } else {
            return $ActualName
        }
    }
    #   This is either a new controller or a new alias.
    #   If the base/crate match an existing controller,
    #   This is just a new alias:
    foreach ControllerName [array names Controllers] {
        set ControllerBase  [lindex $Controllers($ControllerName) 0]
        set ControllerCrate [lindex $Controllers($ControllerName) 1]
        if {($ControllerBase == $base) && ($ControllerCrate == $crate)} {
            set Aliases($name) $ControllerName
            return $ControllerName
        }

    }
    #  Need to make a new controller:
    #

    set caennetname [caennet::create $base $crate]
    caennet::reset $caennetname
    set Controllers($caennetname) "$base $crate"
    if {$caennetname != $name} {
        set Aliases($name) $caennetname
    }
    return $caennetname
}

#
#  Create a new GUI given:
#      name        - A name for the device (no spaces).
#      controller - A CAENnet controller handle.
#      node          - A CAENnet node id (thumbwheel values).
#      base          - Base address of the controller.
#      crate         - VME address of the crate.
#
#   The Controllers array is indexed by controller name
#   and contains the base/crate of each controller.
#  
proc CreateModule {name controller node base {crate 0}} {
   global Config                               ;# Has configuration information.
   global muxenable 
   global muxsel
   global outconfig
   global outpolarity
   global shapetime
   global coarsegain
   global finegain
   global offset
   global pole0
   global ChannelCount
   
   # If necessary, create a new controller.
   #
  
   set RealController [UpdateControllers $name $base $crate]
  
   # Set up the configuration array:
   
   set Config($name.controller) $RealController
   set Config($name.node)       $node
   set Config($name.muxenable)  $muxenable
   set Config($name.muxsel)     $muxsel
   set Config($name.offset)     $offset
   set Config($name.base)       $base
   set Config($name.crate)      $crate

   for {set i 0} {$i < $ChannelCount} {incr i} {
      set Config($name.outcfg.$i) $outconfig($i)
      set Config($name.outpol.$i) $outpolarity($i)
      set Config($name.shapetime.$i) $shapetime($i)
      set Config($name.coarsegain.$i) $coarsegain($i)
      set Config($name.finegain.$i)     $finegain($i)
      set Config($name.pole0.$i)         $pole0($i)
   }
   

   
   # Configure the GUI.
   ConfigureGui $name
   MaintainFailsafe $name

}
#
#  Save the current configuration to a file
#
proc SaveFile {name filename} {
   global Config
   global ChannelCount
   
   if {[catch "open $filename w" fd] != 0} {
      tk_dialog .opfail " Open failed" \
                              "Open for file $name failed: $fd" \
			       error 0 "Dismiss"
      return
   }
   set timestamp [clock seconds]
   set timestamp [clock format $timestamp]
   puts $fd "# Shaper configuration file saved $timestamp \n#"
   puts $fd "#   Name                :  $name"
   puts $fd "#   Caennet Controller  :  $Config($name.controller)"
   puts $fd "#   Caennet node id     :  $Config($name.node)"
   puts $fd "#   VME crate           :  $Config($name.crate)";
   puts $fd "#   VME Base            :  $Config($name.base)"  
   puts $fd "set muxenable $Config($name.muxenable)"
   puts $fd "set  muxsel    $Config($name.muxsel)"

   puts $fd "set offset     $Config($name.offset)"
   puts $fd "set name     $name"
   puts $fd "set controller $Config($name.controller)"
   puts $fd "set crate      $Config($name.crate)"
   puts $fd "set base       $Config($name.base)"
   puts $fd "set nodeid    $Config($name.node)"

   puts $fd ""
   
   for {set i 0} {$i < $ChannelCount} {incr i} {
     puts $fd "set outconfig($i)    $Config($name.outcfg.$i)"
   }
   puts $fd ""
   for {set i 0} {$i < $ChannelCount} {incr i} {
      puts $fd "set outpolarity($i) $Config($name.outpol.$i)"
   }
   puts $fd ""
   for {set i 0} {$i < $ChannelCount} {incr i} {
      puts $fd "set shapetime($i) $Config($name.shapetime.$i)"
   }
   puts $fd ""
   for {set i 0} {$i < $ChannelCount} {incr i} {
      puts $fd "set coarsegain($i)  $Config($name.coarsegain.$i)"
   }
   puts $fd ""
   for {set i 0} {$i < $ChannelCount} {incr i} {
      puts $fd "set finegain($i)  $Config($name.finegain.$i)"
   }
   puts $fd ""
   for {set i 0} {$i < $ChannelCount} {incr i}  {
      puts $fd "set pole0($i)  $Config($name.pole0.$i)"
   }
   close $fd
}
#
#   Select a save file and save configuration to that file.
#
proc SaveConfiguration widget {

   # Extract the name of the shaper to save:
   
   set path [split $widget .]
   set name [lindex $path 1]              ;# leading . >then< name.
   
   set filename [tk_getSaveFile -defaultextension .shaper               \
                                            -title "Save $name configuration"  \
                                            -filetypes {                              \
					       {{Shaper Values}   {.shaper} }   \
					       {{All files}            {*} }}     ]
   if {$filename != "" }  {
      SaveFile $name $filename
   }
}
#
#   Maintain the failsafe file $name_failsafe.shaper
#
proc MaintainFailsafe name {
	append filename $name _failsafe.shaper
	SaveFile $name $filename
	
	after 10000 "MaintainFailsafe $name"
}
#
#   Restore for $name given a $filename:
#
proc RestoreFile {name filename} {
	global Config
	global ChannelCount

	# Set the defaults from the current settings.
	
	set muxenable  $Config($name.muxenable)
	set muxsel     $Config($name.muxsel)
	set offset     $Config($name.offset)
	set crate      $Config($name.crate)
        set base       $Config($name.base)
       set controller $Config($name.controller)

	for {set i 0} {$i < $ChannelCount} {incr i } {
		set outconfig($i)     $Config($name.outcfg.$i)
		set outpolarity($i)   $Config($name.outpol.$i)
		set shapetime($i)    $Config($name.shapetime.$i)
		set coarsegain($i)   $Config($name.coarsegain.$i)
		set finegain($i)       $Config($name.finegain.$i)
		set pole0($i)          $Config($name.pole0.$i)
        }
	# Source in the settings file.
	
	set status [catch "source $filename" err]
	if {$status == 1}  {
	     tk_dialog .sourcefail "Failed to source" \
		     "Failed to source $filename : $err" \
		     error 0 Dismiss
	     return
        }
	
	# Set up the shaper from the settings.
	
	set Config($name.muxenable)  $muxenable
	set Config($name.muxsel)     $muxsel
	set Config($name.offset)     $offset
        set Config($name.controller) $controller
        set Config($name.base)       $base
	set Config($name.crate)     $crate
	
	for {set i 0} {$i < $ChannelCount} {incr i} {
	   set Config($name.outcfg.$i)        $outconfig($i)
	   set Config($name.outpol.$i)        $outpolarity($i)
	   set Config($name.shapetime.$i)   $shapetime($i)
	   set Config($name.coarsegain.$i)  $coarsegain($i)
	   set Config($name.finegain.$i)      $finegain($i)
	   set Config($name.pole0.$i)         $pole0($i)
    }
	ConfigureGui $name
}
#
#   Restore settings (prompting for a file).
#

proc RestoreSettings widget {
	set path [split $widget .]
	set name [lindex $path 1]
	
	set filename [tk_getOpenFile -defaultextension .shaper \
                                            -title "Restore $name configuration"  \
                                            -filetypes {                              
					       {{Shaper Values}   {.shaper} }   
					       {{All files}            {*} }}     ]
        if {$filename != ""} {
	    RestoreFile $name $filename
       } 
}


set me $argv0                       ;# Full path to this script.
set instdir [file dirname $me]      ;# Where we are installed.

package require caennet
source $instdir/n568b.tcl
package require n568b

wm withdraw .



append uiname $instdir/shaper $ChannelCount .ui.tcl
source $uiname






set bads 0
foreach file $argv {

    ResetDefaults
    source $file
    toplevel .$name
    bind .$name <Destroy> KillMe
    set guiname shaper
    append guiname $ChannelCount _ui
    $guiname .$name
    set bad [catch "CreateModule $name $controller $nodeid $base $crate" errormsg]
    if {$bad} {
	incr bads
	exec kill -9 [pid]
    }

}

