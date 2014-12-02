#!/usr/bin/env tclsh

#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2014.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Author:
#    Jeromy Tompkins
#	   NSCL
#	   Michigan State University
#	   East Lansing, MI 48824-1321

set here [file dirname [file normalize [info script]]]
lappend auto_path $here

# Tk package parses argv when it is required... It will cause a crash 
# when it encounters unknown commands, so we have to hide the real arguments
# from it.
set argv_tmp $argv
set argv [list]
package require Tk
#set argv $argv_tmp

# require the needed packages
package require mcfd16usb
package require mcfd16gui
package require mcfd16rc
package require mcfd16commandlogger
package require FrameSequencer


package require cmdline ;# for the command line option parsing

# Handle the options
set options {
  {-protocol.arg   ""          "type of communication protocol (either \"usb\" or \"mxdcrcbus\")"}
  {-serialfile.arg ""          "name of serial file (e.g. /dev/ttyUSB0) \[MANDATORY for USB\]"}
  {-module.arg     ""          "name of module registered to slow-controls server \[MANDATORY for MxDC RCbus\]"}
  {-host.arg       "localhost" "host running VMUSBReadout slow-controls server" }
  {-port.arg       27000       "port the slow-controls server is listening on" }
  {-devno.arg      0           "address of targeted device on rcbus"}
}
set usage " --protocol value ?option value? :"

if {("-help" in $argv_tmp) || ("-?" in $argv_tmp)} {
  puts [cmdline::usage $options $usage]
  exit
}


# Enforce the required options for the usb and mxdcrcbus protocols
proc assertProtocolDependencies {arrname} {
  global params
#  puts $arrname
#  upvar 1 $arrname params

  set fatal 0
  if {$params(-protocol) eq ""} {
    puts "User must pass either \"usb\" or \"rcbus\" for the --protocol option"
    set fatal 1
  }
  if {$params(-protocol) eq "usb"} {
    if {$params(-serialfile) eq ""} {
      puts "The --serialfile argument is mandatory for usb protocol but none was provided."
      set fatal 1
    }
  } elseif {$params(-protocol) eq "mxdcrcbus"} {
    if {$params(-module) eq ""} {
      puts "The --module argument is mandatory for mxdcrcbus protocol but none was provided."
      set fatal 1
    }
  } else {
    set msg "User provided a argument to the --protocol argument that is not "
    append msg "supported. Only \"usb\" and \"rcbus\" are allowed."
    puts $msg
    set fatal 1
  }

  if {$fatal == 1} {
    flush stdout
    exit
  }
}
################## GLOBAL STUFF #############################################
#

proc ConfigureStyle {} {
  ttk::style configure "Title.TLabel" -foreground "firebrick" \
                                      -font "helvetica 28 bold"

  ttk::style configure Header.TLabel -background {orange red} 
  ttk::style configure Header.TFrame -background {orange red}

  ttk::style configure Even.TEntry -background snow3
  ttk::style configure Even.TRadiobutton -background snow3
  ttk::style configure Even.TSpinbox -background snow3
  ttk::style configure Even.TFrame -background snow3
  ttk::style configure Even.TLabel -background snow3

  ttk::style configure Odd.TEntry -background snow3
  ttk::style configure Odd.TRadiobutton -background snow3
  ttk::style configure Odd.TSpinbox -background snow3
  ttk::style configure Odd.TFrame -background snow3
  ttk::style configure Odd.TLabel -background snow3

  ttk::style configure Commit.TButton -background orange
}

########### START THE ACTUAL EXECUTIONAL PORTION OF THE SCRIPT ###############


# try to parse... do so in a catch block because -help will cause a thrown 
# exception
if {[llength $argv_tmp]==0} {
  puts [::cmdline::usage $::options $::usage]
  exit
}

set res [catch {
  array set ::params [::cmdline::getoptions argv_tmp $::options]
} msg]
if {$res == 1} {
  puts $msg
  exit
}

assertProtocolDependencies ::params ;# exits on for bad cmdline options

ConfigureStyle ;# make elements pretty with colorful styles

snit::type MCFD16AppOptions {
  option -protocol
  option -serialfile
  option -module
  option -host
  option -port 
  option -devno

  constructor {args} {
    $self configurelist $args 
  }
}


snit::type MCFD16GuiApp {
  option -optionarray -default ::params
  option -widgetname -default .app

  component _handle
  component _options 
  
  delegate option * to _options

  variable _controlPrsntr 
  variable _configFr
  variable _saveFr
  variable _sequencer

  constructor {args} {
    install _options using MCFD16AppOptions %AUTO% 

    $self configurelist $args

    $self processCmdlineOptions

    $self setUpMenu
    $self BuildGUI

  }

  destructor {
    $_handle destroy
  }

  method processCmdlineOptions {} {

  # Create the driver that serves as the backend of the gui
    set paramDict [array get [$self cget -optionarray]]
    set protocol [dict get $paramDict -protocol]
    if {$protocol eq "usb"} {
      set serialFile [dict get $paramDict -serialfile]
      if {![file exists $serialFile]} {
        puts "Serial file \"$serialFile\" provided but does not exist."
      }

      install _handle using MCFD16USB %AUTO% [dict get $paramDict -serialfile]
    } else {
    # at this point the only other option is mxdcrcbus because 
    # assertProtocolDependencies would have exited otherwise.
      MXDCRCProxy ::proxy -server [dict get $paramDict -host] \
                          -port [dict get $paramDict -port] \
                          -module [dict get $paramDict -module] \
                          -devno [dict get $paramDict -devno]
        # use the proxy created to construct an MCFD16RC
      install _handle using MCFD16RC %AUTO% ::proxy
    }
  }

  method setUpMenu {} {
    option add *tearOff 0

    # get the menu for the toplevel
    set menu [[winfo toplevel [$self cget -widgetname]] cget -menu]
    if {$menu eq ""} {
      set m .menu
      menu $m
      menu $m.file 
      $m.file add command -command [mymethod ToSaveAs] -label "Save as..."
      $m add cascade -menu $m.file -label "File"
      . configure -menu $m
    }
  }

  method BuildGUI {} {
    set win [$self cget -widgetname]

    ttk::label $win.title -text "MCFD-16 Controls" -style "Title.TLabel"

    set _sequencer $win.frames
    FrameSequencer $_sequencer

    
    set _configFr [$self BuildControlFrame $_sequencer]
    set _saveFr [$self BuildSaveAsFrame $_sequencer]

    $_sequencer add config $_configFr
    $_sequencer select config

    grid $_sequencer -sticky nsew -padx 8 -pady 8

  }

  method BuildControlFrame {top} {
    set configFr $top.config
    ttk::frame $configFr
    set title [ttk::label $configFr.title -text "MCFD-16 Controls" -style "Title.TLabel"]
    set infoFrame [$self constructInfoFrame $configFr]
    set _controlPrsntr $configFr.ctl
    set control [MCFD16ControlPanel $_controlPrsntr -handle $_handle]

    grid $title -sticky nsew -padx 8 -pady 8
    grid $infoFrame -sticky nsew -padx 8 -pady 8
    grid $control -sticky nsew -padx 8 -pady 8
    grid rowconfigure $configFr {2} -weight 1
    grid columnconfigure $configFr 0 -weight 1

    return $configFr
  }

  method BuildSaveAsFrame {top} {
    set saveFr $top.save
    SaveToFileForm $top.save $self

    return $saveFr
  }

  method constructInfoFrame {top} {
    set paramDict [array get [$self cget -optionarray]]
    set protoLbl ""
    ttk::frame $top.info -style "Info.TFrame"
    if {[dict get $paramDict -protocol] eq "usb"} {
      set protoLbl "Protocol : USB"
      set serialFile "Serial file : [dict get $paramDict -serialfile]"

      ttk::label $top.info.protoLbl -text $protoLbl
      ttk::label $top.info.serialFile -text $serialFile

      grid $top.info.protoLbl $top.info.serialFile -sticky nsew
      grid columnconfigure $top.info {0 1} -weight 1
    } else {
      set protoLbl "Protocol : MxDC-RCbus"
      set module "Module name : [dict get $paramDict -module]"
      set host "Server host : [dict get $paramDict -host]"
      set port "Server port : [dict get $paramDict -port]"
      set devno "Device address : [dict get $paramDict -devno]"

      ttk::label $top.info.protoLbl -text $protoLbl
      ttk::label $top.info.module -text $module
      ttk::label $top.info.host -text $host
      ttk::label $top.info.port -text $port
      ttk::label $top.info.devno -text $devno

      grid $top.info.protoLbl $top.info.host -sticky nsew
      grid $top.info.module $top.info.port -sticky nsew
      grid $top.info.devno x -sticky nsew
      grid columnconfigure $top.info {0 1} -weight 1
    }


    return $top.info
  }

  method ToSaveAs {} {

    $_sequencer add save $_saveFr
    $_sequencer select save
  }

  method GetHandle {} {
    return $_handle
  }

  method GetControlPresenter {} {
    return $_controlPrsntr
  }
}


ttk::frame .app
MCFD16GuiApp app -widgetname .app
app configure {*}[array get ::params]

grid .app -sticky nsew
grid rowconfigure . 0 -weight 1
grid columnconfigure . 0 -weight 1
wm resizable . false false
