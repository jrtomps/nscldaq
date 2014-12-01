#!/usr/bin/env wish

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


# require the needed packages
package require mcfd16usb
package require mcfd16gui
package require mcfd16rc
package require cmdline ;# for the command line option parsing

# Handle the options
set options {
  {-protocol.arg   ""    "type of communication protocol (either \"usb\" or \"mxdcrcbus\")"}
  {-serialfile.arg ""     "name of serial file (e.g. /dev/ttyUSB0) \[MANDATORY for USB\]"}
  {-module.arg     ""         "name of module registered to slow-controls server \[MANDATORY for MxDC RCbus\]"}
  {-host.arg       "localhost"  "host running VMUSBReadout slow-controls server" }
  {-port.arg       27000       "port the slow-controls server is listening on" }
  {-devno.arg      0       "address of targeted device on rcbus"}
}

# Enforce the required options for the usb and mxdcrcbus protocols
proc assertProtocolDependencies {arrname} {
  upvar $arrname params

  if {[lindex [array get params -protocol] 1] eq ""} {
    puts [::cmdline::usage $options $usage]
    puts "User must pass either \"usb\" or \"rcbus\" for the --protocol option"
    exit
  }
  if {$params(-protocol) eq "usb"} {
    if {$params(-serialfile) eq ""} {
      puts "The --serialfile argument is mandatory for usb protocol but none was provided."
      exit
    }
  } elseif {$params(-protocol) eq "mxdcrcbus"} {
    if {$params(-module) eq ""} {
      puts "The --module argument is mandatory for mxdcrcbus protocol but none was provided."
      exit
    }
  } else {
    set msg "User provided a argument to the --protocol argument that is not "
    append msg "supported. Only \"usb\" and \"rcbus\" are allowed."
    puts $msg
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

## Fill the information frame at top of the gui with protocol dependent info
#
proc constructInfoFrame {arrname} {
  upvar $arrname params
  set protoLbl ""
  ttk::frame .info -style "Info.TFrame"
  if {$params(-protocol) eq "usb"} {
    set protoLbl "Protocol : USB"
    set serialFile "Serial file : $params(-serialfile)"
    
    ttk::label .info.protoLbl -text $protoLbl
    ttk::label .info.serialFile -text $serialFile

    grid .info.protoLbl .info.serialFile -sticky nsew
    grid columnconfigure .info {0 1} -weight 1
  } else {
    set protoLbl "Protocol : MxDC-RCbus"
    set module "Module name : $params(-module)"
    set host "Server host : $params(-host)"
    set port "Server port : $params(-port)"
    set devno "Device address : $params(-devno)"
    
    ttk::label .info.protoLbl -text $protoLbl
    ttk::label .info.module -text $module
    ttk::label .info.host -text $host
    ttk::label .info.port -text $port
    ttk::label .info.devno -text $devno

    grid .info.protoLbl .info.host -sticky nsew
    grid .info.module .info.port -sticky nsew
    grid .info.devno x -sticky nsew
    grid columnconfigure .info {0 1} -weight 1
  }


  return .info
}

########### START THE ACTUAL EXECUTIONAL PORTION OF THE SCRIPT ###############

set usage " --protocol value ?option value? :"
array set params [::cmdline::getoptions argv $options]

assertProtocolDependencies params ;# exits on for bad cmdline options

ConfigureStyle ;# make elements pretty with colorful styles

# Create the driver that serves as the backend of the gui
if {$params(-protocol) eq "usb"} {
  set serialFile $params(-serialfile)
  if {![file exists $serialFile]} {
    puts "Serial file \"$serialFile\" provided but does not exist."
  }

  MCFD16USB ::dev $params(-serialfile)
} else {
  # at this point the only other option is mxdcrcbus because 
  # assertProtocolDependencies would have exited otherwise.
  MXDCRCProxy ::proxy -server $params(-host) -port $params(-port) \
                      -module $params(-module) -devno $params(-devno)
  # use the proxy created to construct an MCFD16RC
  MCFD16RC dev ::proxy
}

########################### CONSTRUCT THE GUI #########################

ttk::label .title -text "MCFD-16 Controls" -style "Title.TLabel"
set infoFrame [constructInfoFrame params]


set control [MCFD16ControlPanel .ctl -handle ::dev]
PulserPresenter pulserCtlr [PulserView .pulser] dev

grid .title -sticky nsew -padx 8 -pady 8
grid $infoFrame -sticky nsew -padx 8 -pady 8
grid .ctl -sticky nsew -padx 8 -pady 8
grid .pulser -sticky nsew -padx 8 -pady 8

grid rowconfigure . {2} -weight 1
grid columnconfigure . 0 -weight 1

wm resizable . false false
