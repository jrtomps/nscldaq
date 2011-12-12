#!/bin/sh

# start wish \
exec wish ${0} ${@}

set reply [tk_messageBox -icon warning -title "Obsolete software!" \
    -message {You are running an obsolete version of the CAENV812 CFD controller gui.
An up to date version is in $DAQROOT/Scripts/ControlApplications/caenv812control.tcl  where $DAQROOT is
where the DAQ software has been installed} -type okcancel]

if {$reply == "cancel"} {
    exit
}


# interface generated by SpecTcl version 1.1 from X:/llnl/clients/cfd/gui.ui
#   root     is the parent window for this user interface

proc gui_ui {root args} {

	# this treats "." as a special case

	if {$root == "."} {
	    set base ""
	} else {
	    set base $root
	}
    
	label $base.label#15 \
		-text {CAENV 812 @}

	label $base.base \
		-text label \
		-textvariable BaseAddress

	label $base.label#17 \
		-text {Serial Number: }

	label $base.serialno \
		-text label \
		-textvariable SerialNumber

	label $base.label#19 \
		-text {Configuration File}

	label $base.filename \
		-text label \
		-textvariable Filename

	label $base.label#21 \
		-text Module

	label $base.module \
		-text label \
		-textvariable ModuleName

	scale $base.threshold0 \
		-command {SetThreshold 0} \
		-from -1.0 \
		-label {Threshold  0 mv} \
		-orient h \
		-to -255.0

	scale $base.threshold1 \
		-command { SetThreshold 1} \
		-from -1.0 \
		-label {Threshold 1 mv} \
		-orient h \
		-to -255.0

	scale $base.threshold2 \
		-command {SetThreshold 2} \
		-from -1.0 \
		-label {Threshold 2 mv} \
		-orient h \
		-to -255.0

	scale $base.threshold3 \
		-command {SetThreshold 3} \
		-from -1.0 \
		-label {Threshold 3 mv} \
		-orient h \
		-to -255.0

	scale $base.threshold4 \
		-command {SetThreshold 4} \
		-from -1.0 \
		-label {Threshold 4 mv} \
		-orient h \
		-to -255.0

	scale $base.threshold5 \
		-command {SetThreshold 5} \
		-from -1.0 \
		-label {Threshold 5 mv} \
		-orient h \
		-to -255.0

	scale $base.threshold6 \
		-command {SetThreshold 6} \
		-from -1.0 \
		-label {Threshold 6 mv} \
		-orient h \
		-to -255.0

	scale $base.threshold7 \
		-command {SetThreshold 7} \
		-from -1.0 \
		-label {Threshold 7 mv} \
		-orient h \
		-to -255.0

	scale $base.width0 \
		-command {SetWidth 0} \
		-from 15.0 \
		-label {Widths Ch 0-7} \
		-orient h \
		-to 250.0

        label $base.lockstate

	scale $base.deadtime0 \
		-command {SetDeadtime 0} \
		-from 150.0 \
		-label {Deadtimes Ch 0-7} \
		-orient h \
		-to 2000.0

	scale $base.threshold8 \
		-command {SetThreshold 8} \
		-from -1.0 \
		-label {Threshold 8 mv} \
		-orient h \
		-to -255.0

	scale $base.threshold9 \
		-command {SetThreshold 9} \
		-from -1.0 \
		-label {Threshold 9 mv} \
		-orient h \
		-to -255.0

	scale $base.threshold10 \
		-command {SetThreshold 10} \
		-from -1.0 \
		-label {Threshold 10 mv} \
		-orient h \
		-to -255.0

	scale $base.threshold11 \
		-command {SetThreshold 11} \
		-from -1.0 \
		-label {Threshold 11 mv} \
		-orient h \
		-to -255.0

	scale $base.threshold12 \
		-command {SetThreshold 12} \
		-from -1.0 \
		-label {Threshold 12 mv} \
		-orient h \
		-to -255.0

	scale $base.threshold13 \
		-command {SetThreshold 13} \
		-from -1.0 \
		-label {Threshold 13 mv} \
		-orient h \
		-to -255.0

	scale $base.threshold14 \
		-command {SetThreshold 14} \
		-from -1.0 \
		-label {Threshold 14 mv} \
		-orient h \
		-to -255.0

	scale $base.threshold15 \
		-command {SetThreshold 15} \
		-from -1.0 \
		-label {Threshold 15 mv} \
		-orient h \
		-to -255.0

	scale $base.width1 \
		-command {SetWidth 1} \
		-from 15.0 \
		-label {Widths Ch 8-15 } \
		-orient h \
		-to 250.0

	scale $base.deadtime1 \
		-command {SetDeadtime 1} \
		-from 150.0 \
		-label {Deadtimes Ch 8-15} \
		-orient h \
		-to 2000.0
##############
	scale $base.maskit \
		-command {SetMask} \
		-from 0.0 \
		-label {Mask 0-0xffff} \
		-orient h \
		-to 0xffff
##################$
	label $base.label#14 \
		-text {Majority threshold}
	
	
	radiobutton $base.majority1 \
		-command SetMajority \
		-text 1 \
		-value 1 \
		-variable MajorityLevel
	catch {
		$base.majority1 configure \
			-font -*-Times-Medium-R-Normal-*-*-140-*-*-*-*-*-*
	}

	radiobutton $base.majority5 \
		-command SetMajority \
		-text 5 \
		-value 5 \
		-variable MajorityLevel
	catch {
		$base.majority5 configure \
			-font -*-Times-Medium-R-Normal-*-*-140-*-*-*-*-*-*
	}

	radiobutton $base.majority9 \
		-command SetMajority \
		-text 9 \
		-value 9 \
		-variable MajorityLevel
	catch {
		$base.mnajority9 configure \
			-font -*-Times-Medium-R-Normal-*-*-140-*-*-*-*-*-*
	}

	radiobutton $base.majority13 \
		-command SetMajority \
		-text 13 \
		-value 13 \
		-variable MajorityLevel
	catch {
		$base.majority13 configure \
			-font -*-Times-Medium-R-Normal-*-*-140-*-*-*-*-*-*
	}

#       radiobutton $base.majority17 \
#  		-command SetMajority \
#  		-text 17 \
#  		-value 17 \
#  		-variable MajorityLevel
#  	catch {
#  		$base.majority17 configure \
#  			-font -*-Times-Medium-R-Normal-*-*-140-*-*-*-*-*-*
#  	}

	radiobutton $base.majority2 \
		-command SetMajority \
		-text 2 \
		-value 2 \
		-variable MajorityLevel
	catch {
		$base.radiobutton#17 configure \
			-font -*-Times-Medium-R-Normal-*-*-140-*-*-*-*-*-*
	}

	radiobutton $base.majority6 \
		-command SetMajority \
		-text 6 \
		-value 6 \
		-variable MajorityLevel
	catch {
		$base.majority6 configure \
			-font -*-Times-Medium-R-Normal-*-*-140-*-*-*-*-*-*
	}

	radiobutton $base.majority10 \
		-command SetMajority \
		-text 10 \
		-value 10 \
		-variable MajorityLevel
	catch {
		$base.majority10 configure \
			-font -*-Times-Medium-R-Normal-*-*-140-*-*-*-*-*-*
	}

	radiobutton $base.majority14 \
		-command SetMajority \
		-text 14 \
		-value 14 \
		-variable MajorityLevel
	catch {
		$base.majority14 configure \
			-font -*-Times-Medium-R-Normal-*-*-140-*-*-*-*-*-*
	}

#  	radiobutton $base.majority18 \
#  		-command SetMajority \
#  		-text 18 \
#  		-value 18 \
#  		-variable MajorityLevel
#  	catch {
#  		$base.majority18 configure \
#  			-font -*-Times-Medium-R-Normal-*-*-140-*-*-*-*-*-*
#  	}

	radiobutton $base.majority3 \
		-command SetMajority \
		-text 3 \
		-value 3 \
		-variable MajorityLevel
	catch {
		$base.majority3 configure \
			-font -*-Times-Medium-R-Normal-*-*-140-*-*-*-*-*-*
	}

	radiobutton $base.majority7 \
		-command SetMajority \
		-text 7 \
		-value 7 \
		-variable MajorityLevel
	catch {
		$base.majority7 configure \
			-font -*-Times-Medium-R-Normal-*-*-140-*-*-*-*-*-*
	}

	radiobutton $base.majority11 \
		-command SetMajority \
		-text 11 \
		-value 11 \
		-variable MajorityLevel
	catch {
		$base.majority11 configure \
			-font -*-Times-Medium-R-Normal-*-*-140-*-*-*-*-*-*
	}

	radiobutton $base.majority15 \
		-command SetMajority \
		-text 15 \
		-value 15 \
		-variable MajorityLevel
	catch {
		$base.majority15 configure \
			-font -*-Times-Medium-R-Normal-*-*-140-*-*-*-*-*-*
	}

#  	radiobutton $base.majority19 \
#  		-command SetMajority \
#  		-text 19 \
#  		-value 19 \
#  		-variable MajorityLevel
#  	catch {
#  		$base.majority19 configure \
#  			-font -*-Times-Medium-R-Normal-*-*-140-*-*-*-*-*-*
#  	}

	radiobutton $base.majority4 \
		-command SetMajority \
		-text 4 \
		-value 4 \
		-variable MajorityLevel
	catch {
		$base.majority4 configure \
			-font -*-Times-Medium-R-Normal-*-*-140-*-*-*-*-*-*
	}

	radiobutton $base.majority8 \
		-command SetMajority \
		-text 8 \
		-value 8 \
		-variable MajorityLevel
	catch {
		$base.majority8 configure \
			-font -*-Times-Medium-R-Normal-*-*-140-*-*-*-*-*-*
	}

	radiobutton $base.majority12 \
		-command SetMajority \
		-text 12 \
		-value 12 \
		-variable MajorityLevel
	catch {
		$base.majority12 configure \
			-font -*-Times-Medium-R-Normal-*-*-140-*-*-*-*-*-*
	}

  	radiobutton $base.majority16 \
  		-command SetMajority \
  		-text 16 \
  		-value 16 \
  		-variable MajorityLevel
  	catch {
  		$base.majority16 configure \
  			-font -*-Times-Medium-R-Normal-*-*-140-*-*-*-*-*-*
  	}

#  	radiobutton $base.majority20 \
#  		-command SetMajority \
#  		-text 20 \
#  		-value 20 \
#  		-variable MajorityLevel
#  	catch {
#  		$base.majority20 configure \
#  			-font -*-Times-Medium-R-Normal-*-*-140-*-*-*-*-*-*
#  	}

	button $base.save \
		-command SaveConfig \
		-text Save...

	button $base.restore \
	        -command RestoreConfiguration \
	        -text Restore...

        button $base.lock \
		-command "ToggleLock \"$base\"" \
		-text Lock

        # Geometry management

	grid $base.label#15 -in $root	-row 1 -column 3 
	grid $base.base -in $root	-row 1 -column 4 
	grid $base.label#17 -in $root	-row 1 -column 5 
	grid $base.serialno -in $root	-row 1 -column 6 
	grid $base.label#19 -in $root	-row 2 -column 3 
	grid $base.filename -in $root	-row 2 -column 4 
	grid $base.label#21 -in $root	-row 2 -column 5 
	grid $base.module -in $root	-row 2 -column 6 
	grid $base.threshold0 -in $root	-row 3 -column 1  \
		-sticky ew
	grid $base.threshold1 -in $root	-row 3 -column 2  \
		-sticky ew
	grid $base.threshold2 -in $root	-row 3 -column 3  \
		-sticky ew
	grid $base.threshold3 -in $root	-row 3 -column 4  \
		-sticky ew
	grid $base.threshold4 -in $root	-row 3 -column 5  \
		-sticky ew
	grid $base.threshold5 -in $root	-row 3 -column 6  \
		-sticky ew
	grid $base.threshold6 -in $root	-row 3 -column 7  \
		-sticky ew
	grid $base.threshold7 -in $root	-row 3 -column 8  \
		-sticky ew
        grid $base.lockstate -in $root  -row 4 -column 4  \
		-sticky ew
	grid $base.width0 -in $root	-row 4 -column 3  \
		-sticky ew
	grid $base.deadtime0 -in $root	-row 4 -column 6  \
		-sticky ew
	grid $base.threshold8 -in $root	-row 5 -column 1  \
		-sticky ew
	grid $base.threshold9 -in $root	-row 5 -column 2  \
		-sticky ew
	grid $base.threshold10 -in $root	-row 5 -column 3  \
		-sticky ew
	grid $base.threshold11 -in $root	-row 5 -column 4  \
		-sticky ew
	grid $base.threshold12 -in $root	-row 5 -column 5  \
		-sticky ew
	grid $base.threshold13 -in $root	-row 5 -column 6  \
		-sticky ew
	grid $base.threshold14 -in $root	-row 5 -column 7  \
		-sticky ew
	grid $base.threshold15 -in $root	-row 5 -column 8  \
		-sticky ew
	grid $base.width1 -in $root	-row 6 -column 3  \
		-sticky ew
	grid $base.deadtime1 -in $root	-row 6 -column 6  \
		-sticky ew
##################################
	#grid $base.maskit -in $root	-row 6 -column 7  \
		#-sticky ew
#######################
	grid $base.label#14 -in $root	-row 7 -column 3 
	
	grid $base.majority1 -in $root	-row 8 -column 1 
	grid $base.majority5 -in $root	-row 8 -column 2 
	grid $base.majority9 -in $root	-row 8 -column 3 
	grid $base.majority13 -in $root	-row 8 -column 4 
	#grid $base.majority17 -in $root	-row 8 -column 5 
	grid $base.majority2 -in $root	-row 9 -column 1 
	grid $base.majority6 -in $root	-row 9 -column 2 
	grid $base.majority10 -in $root	-row 9 -column 3 
	grid $base.majority14 -in $root	-row 9 -column 4 
	#grid $base.majority18 -in $root	-row 9 -column 5 
	grid $base.majority3 -in $root	-row 10 -column 1 
	grid $base.majority7 -in $root	-row 10 -column 2 
	grid $base.majority11 -in $root	-row 10 -column 3 
	grid $base.majority15 -in $root	-row 10 -column 4 
	#grid $base.majority19 -in $root	-row 10 -column 5 
	grid $base.majority4 -in $root	-row 11 -column 1 
	grid $base.majority8 -in $root	-row 11 -column 2 
	grid $base.majority12 -in $root	-row 11 -column 3 
	grid $base.majority16 -in $root	-row 11 -column 4 
	#grid $base.majority20 -in $root	-row 11 -column 5 
	grid $base.save -in $root	-row 13 -column 2 
	grid $base.restore -in $root    -row 13 -column 4
        grid $base.lock -in $root       -row 13 -column 6

	# Resize behavior management

	grid rowconfigure $root 1 -weight 0 -minsize 30
	grid rowconfigure $root 2 -weight 0 -minsize 30
	grid rowconfigure $root 3 -weight 0 -minsize 30
	grid rowconfigure $root 4 -weight 0 -minsize 53
	grid rowconfigure $root 5 -weight 0 -minsize 30
	grid rowconfigure $root 6 -weight 0 -minsize 18
	grid rowconfigure $root 7 -weight 0 -minsize 30
	grid rowconfigure $root 8 -weight 0 -minsize 30
	grid rowconfigure $root 9 -weight 0 -minsize 30
	grid rowconfigure $root 10 -weight 0 -minsize 30
	grid rowconfigure $root 11 -weight 0 -minsize 30
	grid rowconfigure $root 12 -weight 0 -minsize 30
	grid rowconfigure $root 13 -weight 0 -minsize 30
	grid columnconfigure $root 1 -weight 1 -minsize 30
	grid columnconfigure $root 2 -weight 1 -minsize 2
	grid columnconfigure $root 3 -weight 1 -minsize 95
	grid columnconfigure $root 4 -weight 1 -minsize 30
	grid columnconfigure $root 5 -weight 1 -minsize 30
	grid columnconfigure $root 6 -weight 1 -minsize 68
	grid columnconfigure $root 7 -weight 1 -minsize 30
	grid columnconfigure $root 8 -weight 1 -minsize 30

	
        label $base.label#70 -text {Mask}
	grid $base.label#70 -in $root -row 7 -column 6
	set x 5
	set y 8
	for {set i 0} {$i < 16} {incr i 1} {
	    global mask_arr
	    set mask_arr($i) 1
	    checkbutton $base.cbtnmask$i -text $i \
		    -variable mask_arr($i) -command MaskChecked
	    grid $base.cbtnmask$i -in $root -row $y -column $x
	    incr y
	    if { $y>11 } { 
		incr x 
		set y 8
	    }
	}
	
	
# additional interface code
#set InstallDir ~/llnl/clients/cfd
set InstallDir /user/miniball/p_n/cfd

global BaseAddress
global SerialNumber 
global Filename
global ModuleName
global MajorityLevel
global auto_path
global Configured
global ModuleMap
global argv0
global argv


set BaseAddress 0
set SerialNumber 1234
set Filename "-unknown-"
set ModuleName "-unknown-"
set MajorityLevel 1
set Device { }
set Configured 0

set me $argv0
set mypath [file dirname $me]
set auto_path "$mypath $auto_path"

package require CFD812
package require CFDState

proc MaskChecked {} {
    global mask_arr
    set calc_value [expr {$mask_arr(15)*32768+$mask_arr(14)*16384\
	    +$mask_arr(13)*8192+$mask_arr(12)*4096+$mask_arr(11)*2048\
	    +$mask_arr(10)*1024+$mask_arr(9)*512+$mask_arr(8)*256\
	    +$mask_arr(7)*128+$mask_arr(6)*64+$mask_arr(5)*32\
	    +$mask_arr(4)*16+$mask_arr(3)*8+$mask_arr(2)*4+$mask_arr(1)*2\
	    +$mask_arr(0)}]
    SetMask $calc_value
}
#  Conversion from a width value in ns
#  to a width register value (0-255).
#  The width register is 15 - 250ns.
proc WidthToRegister {ns} {
 
  set register [expr ($ns - 15)*256/235]
  if {$register < 0} { set register 0 }
  if {$register > 255} {set register 255}

  return $register
}
#  Conversion from a register value 
#  To a width value.
#
proc RegisterToWidth {reg} {

   set ns [expr $reg*235/256]
   set ns [expr $ns + 15]

   return $ns
}


# Conversion from a deadtime valuein ns
# To a register value (0-255) 
#
proc DeadtimeToRegister {ns} {
   set range [expr (2000 - 150) + 1]
   set reg [expr ($ns - 150)*256/$range]
   if {$reg < 0} {set reg 0}
   if {$reg > 255} {set reg 255}
   return $reg
}

proc RegisterToDeadtime {reg} {
   set range [expr (2000-150)+1]
   set ns [expr $reg*range/256 + 150]

   return $ns
}

#  Respond to changes in a threshold
# scale widget

proc SetThreshold {channel value} { 
  global Configured
  global ModuleName
  global ModuleMap

  if {$Configured} {
     CFDState::SetThreshold $ModuleName $channel $value
     CFD812::SetThreshold $ModuleMap $channel $value
  }
}

proc SetMask {value} { 
  global Configured
  global ModuleName
  global ModuleMap
  
  if {$Configured} {
     CFD812::SetMask $ModuleMap $value
  }
}

# Respond to changes in width scale widgets.

proc SetWidth {bank value} {
   global ModuleName
   global ModuleMap
   global Configured
   if {$Configured} {
     CFDState::SetWidth $ModuleName $bank $value
     CFD812::SetWidth $ModuleMap $bank [WidthToRegister $value]
   }
}

# Respond in changes in Deadtime scale widgets.

proc SetDeadtime {bank value} { 
   global Configured
   global ModuleName
   global ModuleMap

   if {$Configured } {
      CFDState::SetDeadtime $ModuleName $bank $value
      CFD812::SetDeadtime $ModuleMap $bank \
                       [DeadtimeToRegister $value]
   }
}

#Respond to changes in the majority level requested.

proc SetMajority {} {  
   global MajorityLevel
   global Configured
   global ModuleName
   global ModuleMap
   global Filename

   if {$Configured} {
      CFDState::SetMultiplicity $ModuleName $MajorityLevel
      CFD812::SetMultiplicityThreshold $ModuleMap \
                                        $MajorityLevel
   }
}

proc SaveFile {name} {
   global BaseAddress
   global SerialNumber 
   global Filename
   global ModuleName
   global BaseAddress
   global SerialNumber 
   global Filename
   global ModuleName
   global Crate
   global mask_arr

  set fd [open $name w]

   # Identify the file:

  set ticks [clock seconds]
  set time [clock format $ticks]

  puts $fd "# CFD configuration file written $time"
  puts $fd "set Name \"$ModuleName\""
  puts $fd "set ModuleBase [format 0x%x $BaseAddress]"
  puts $fd "set Crate      $Crate"

  #  Thresholds:

  for {set i 0} {$i < 16} {incr i} {
     puts $fd \
      "set Thresholds($i) [CFDState::GetThreshold $ModuleName $i]"
  }
  # widths:
 
  puts $fd "set WidthLow  [CFDState::GetWidth $ModuleName 0]"
  puts $fd "set WidthHigh [CFDState::GetWidth $ModuleName 1]"

  # Deadtimes:

  puts $fd "set DeadTimeLow  [CFDState::GetDeadtime $ModuleName 0]"
  puts $fd "set DeadTimeHigh [CFDState::GetDeadtime $ModuleName 1]"

  # Majority:

  puts $fd "set Majority  [CFDState::GetMultiplicity $ModuleName]"

  # Mask:

    for {set i 0} {$i < 16} {incr i} {
        puts $fd "set mask_arr($i) $mask_arr($i)"
    }

  close $fd
}

proc SaveConfig {} {
   global BaseAddress
   global SerialNumber 
   global Filename
   global ModuleName
   global BaseAddress
   global SerialNumber 
   global Filename
   global ModuleName

  set filename [tk_getSaveFile  -defaultextension .cfd \
		    -filetypes [getFiletypes] \
			-title "Save config file as.."]

  if {$filename == ""} return
  SaveFile $filename
  set Filename $filename

}
#
#  Read the configuration file.
#
proc ReadConfig {file} {
   global BaseAddress
   global Filename
   global ModuleName
   global MajorityLevel
   global Device
   global Crate
    global mask_arr

   # Initialize thresholds to defaults:

   for {set i 0} {$i < 16} {incr i} {
     set Thresholds($i) -5    ;# Default values.
   }

   # Inittialize other valuse to defaults.

   set WidthLow 128
   set WidthHigh 128

   set DeadTimeLow 200
   set DeadTimeHigh 200

   set Majority 1

   # Read the file to override defaults:

   catch "source $file"

  # set our globals according to the sourced setup file:

   set ModuleName   $Name
   set BaseAddress  $ModuleBase
   set MajorityLevel $Majority

  # Create and setup a new state object:


    catch {CFDState::Destroy $Name}
   catch "CFDState::Create $Name";	# May already exist.
   for {set i 0} {$i < 16} {incr i} {
     CFDState::SetThreshold $Name $i $Thresholds($i)
   }
   CFDState::SetWidth $Name 0 $WidthLow
   CFDState::SetWidth $Name 1 $WidthHigh

   CFDState::SetDeadtime $Name 0 $DeadTimeLow
   CFDState::SetDeadtime $Name 1 $DeadTimeHigh
   CFDState::SetBase  $Name $ModuleBase
   CFDState::SetMultiplicity $Name $Majority

   puts "Crate read as $Crate"
   
}

#  Set the configuration from the current settings.
#  By setting the config into the UI elements
#  The hardware setting functions will automatically
#  get set.

proc SetConfig {} {

   global ModuleName
   global ModuleMap
   global BaseAddress
   global SerialNumber
   global Crate
   global Configured

	# Access the hardware and set it up according
	# to the configuration as well.

   catch {
       puts "Map = $ModuleMap"
       CFD812::Unmap $ModuleMap
   }

    set status [catch "CFD812::Map $BaseAddress $ModuleName $Crate" ModuleMap]
    if {$status == 1} {
	puts "Unable to map the cfd: $ModuleMap"
	exit
    }
    puts "Module map: $ModuleMap"
    flush stdout

     # Thresholds:

  for {set i 0} {$i < 16} {incr i} {
     CFD812::SetThreshold $ModuleMap $i \
	[CFDState::GetThreshold $ModuleName $i]
  }

    # Widths:

  CFD812::SetWidth $ModuleMap 0 \
        [WidthToRegister [CFDState::GetWidth $ModuleName 0]]
  CFD812::SetWidth $ModuleMap 1 \
        [WidthToRegister [CFDState::GetWidth $ModuleName 1]] 

   # Deadtimes:

  CFD812::SetDeadtime $ModuleMap 0 \
  [DeadtimeToRegister [CFDState::GetDeadtime $ModuleName 0]]
  CFD812::SetDeadtime $ModuleMap 1 \
  [DeadtimeToRegister [CFDState::GetDeadtime $ModuleName 1]]

   # Majority level:

  CFD812::SetMultiplicityThreshold $ModuleMap \
    [CFDState::GetMultiplicity $ModuleName]

  # Finally read the serial number register and
  # put it in the corresponding lable widget's variable.

  set SerialNumber [CFD812::GetSerialNumber $ModuleMap]
  puts "New Serial no $SerialNumber"

	# Set threshold widgets.

   for {set i 0} {$i < 16} {incr i} {
      .threshold$i set [CFDState::GetThreshold $ModuleName $i]
   }

	# Set the width widgets.

   .width0 set [CFDState::GetWidth $ModuleName 0]
   .width1 set [CFDState::GetWidth $ModuleName 1]

	# Set the deadtime widgets.

   .deadtime0 set [CFDState::GetDeadtime $ModuleName 0]
   .deadtime1 set [CFDState::GetDeadtime $ModuleName 1]

	# Set the majority

   SetMajority
#   SetMask 0xffff
}

#  Select and read the config file:




#  Automated periodic save.
#


proc AutoSave {ms name} {
    SaveFile $name
    after $ms "AutoSave $ms $name"

}

proc getConfigFile {} {

    set Filename [tk_getOpenFile  -defaultextension .cfd \
		      -filetypes [getFiletypes] \
	    -title "Read Configuration"]
    return $Filename
}


proc getFiletypes {} {
    return {
	{{CFD Config}       {.cfd}        }
	{{All Files}        *             }
    }
}


if {$argv == "" } {
    set Filename [getConfigFile]
} else {
    set Filename $argv
}

#
#  Set the actual lock state.
#
proc SetLockState {base state text} {

    $base.lock configure -text $text

    for {set chan 0} {$chan < 16} {incr chan} {
	foreach item {threshold cbtnmask} {
	    set widget $base.$item
	    append widget $chan
	    $widget configure -state $state
	}
	set widget $base.majority
	append widget [expr $chan + 1]
	$widget configure -state $state
    }
    foreach item {width0 width1 deadtime0 deadtime1} {
	set widget $base.$item
	$widget configure -state $state
    }
    
}

#
#  Prompt for file to read/restore configuration from.
#
proc RestoreConfiguration {} {
    
    set name [getConfigFile]
    if {$name != ""} {
	ReadConfig $name
	SetConfig
	MaskChecked
    }
}

#
#   Toggle the lock on the ui.
#
proc ToggleLock {base} {
   set state [$base.lock  cget -text]

    if {$state == "Lock"} {
	SetLockState $base disabled Unlock
	$base.lockstate configure -text "Locked!!" -fg red
    } else {
        SetLockState $base normal Lock
	$base.lockstate configure -text ""
    }
}


ReadConfig $Filename

set Filename [file tail $Filename]

global Configured

set Configured 1

SetConfig


MaskChecked













# end additional interface code

}

# Allow interface to be run "stand-alone" for testing


	    wm title . "CFD control"
	    gui_ui .



AutoSave 10000 failsafe.cfd