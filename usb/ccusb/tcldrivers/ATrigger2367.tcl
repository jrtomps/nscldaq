
#
#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2014.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#     Author:
#       NSCL DAQ Development Group 
#       NSCL
#       Michigan State University
#       East Lansing, MI 48824-1321
# @author Jeromy Tompkins and Daniel Bazin
#
# @note The original version of this code was written by Daniel Bazin
#       and it has been modified extensively to work with the 
#       NSCLDAQ libraries. 


# Define the package
package provide ATrigger2367 1.0

package require Itcl

## @class ATrigger2367
#
# This is the low-level driver to support the usbtrig.bit firmware loaded
# into a LeCroy Universal Logic Module 2367. The firmware is written and 
# maintained by Daniel Bazin at the National Superconducting Cyclotron 
# Laboratory. The firmware that this supports provides trigger logic for
# handling multiple different raw triggers and choosing what will constitute
# the formation of a live trigger. The user can sample raw triggers as well
# as delay them. A GUI for controlling the configuration of the firmware
# is provided as a client to the CCUSBReadout slow controls server. It is 
# possible for the user to manipulate the device in tcl scripts through
# the readoutscript or controlscript packages or through an interpreter.
#
# Because this device is used for both building stacks and executing 
# commands in interactive mode, it is not required that the user provide
# a valid reference to a controller at construction. One reason for this
# is if a version is created specifically for building stacks. In that
# scenario, there is no interaction with a controller ever and there may
# not be a live cccusb::CCCUSB object to use. So to get around this, all
# methods that will use a controller check to see that the user has provided 
# a valid instance. It is the user's responsibility to ensure that this
# is up to date because the stored reference will persist after the 
# cccusb::CCCUSB object has been destroyed.
#
itcl::class ATrigger2367 {
	private variable device     ;#< a reference to the cccusb::CCCUSB object
	private variable node       ;#< the slot of the ULM
	private variable self       ;#< the name of the instance

	

  ## @brief Constructor
  #
  # @param de   a cccusb::CCCUSB object. If this is not provided at 
  #             construction, then it can be defined later with the
  #             SetController method.
  # @param no   slot where ULM device resides.
  #
  # @returns name of the instance 
	constructor {de no} {
		set device $de 
		set node $no
		set self [string trimleft $this :]
	}
	
  ## @brief a no-op
	destructor {}

  # --------------- Interactive methods ---------------------------

 
  ## @brief Store a reference to a controller for later one-shot operations
  # 
  # Because it is not demanded that the constructor accepts a valid ctlr
  # object, the user must ensure that a valid controller is provided before 
  # calling the various interactive methods. 
  # 
  # @param ctlr   a cccusb::CCCUSB object
  # 
  # @return ""
  public method SetController {ctlr}

  
  ## @brief Load firmware into the ULM
  #
  # @param firmware   a path to a valid firmware file
  #
  # @returns ""
  #
  # Exceptional returns:
  # - Error if firmware file doesn't exit
  # - Error if firmware file cannot be read
  # - Error if user has not previously set a valid ctlr
  # - Error if the signature values fail to be validated after the load 
	public method Configure {firmware}

  ## @brief Clear the module
  # 
  # I believe that this actually clears the trigger register. 
  # Not sure what else it clears...
  #
  # @returns integer encoded with Q and X response 
  #
  # Exceptional returns:
  # - Error if the user has not set the controller.
  public method Clear {} 

  ## @brief Manipulate the GO bit
  #
  # When the Go bit is set to 1, the ULM can output triggers and a clock.
  # If it is 0, it does not accept any raw triggers.
  #
  # @param bit  the value to set the trigger bit to
  # 
  # @returns integer encoded with Q and X response 
  #
  # Exceptional returns:
  # - Error if user passes a non-boolean value 
  # - Error if user has not set the controller 
  # 
	public method Go {bit} 

  ## @brief Select timestamp mode
  # 
  # Sets whether the ULM will accept an external timestamp clock and external timestamp latch.
  #
  # bit 0 : accepts an external timestamp clock
  # bit 1 : accepts an external timestamp latch
  #
  # To use an external clock, the code should be 1 and to use an internal 
  # clock, the code should be 0.
  #
  # @param code  the code to write. must be in range [0,3]
  #
  # @returns integer encoded with Q and X response 
  #
  # Exceptional returns:
  # - Error is user has not set the controller.
  # - Error if argument is out of range.
  public method Select {bit} 


  ## @brief Enable ... 
  #
  # @param bits value to write
  #
  # @returns integer encoded with Q and X response 
  #
  # Exceptional returns:
  # - Error is user has not set the controller 
  #
  public method Enable {bits} 


  ## @brief Check that the 2 signatures are 2367 and 5800. 
  # 
  # @return boolean
  # 
  public method IsConfigured {}


  ##################################################################
  #                                                                #
  # ----------------- stack methods -------------------------------#
  #                                                                #
  ##################################################################

  ## @brief Adds a clear operation to the stack
  #
  # @param stack  a cccusbreadoutlist::CCCUSBReadoutList object 
  #
  # @return ""
	public method sClear {stack}

  ## @brief Adds a clear register operation to the stack
  #
  # @param stack  a cccusbreadoutlist::CCCUSBReadoutList object 
  #
  # @return ""
	public method sClearRegister {stack}


  ## @brief Adds a read register operation to the stack
  #
  # @param stack  a cccusbreadoutlist::CCCUSBReadoutList object 
  #
  # @return ""
  public method sRead {stack}


  ## @brief Adds a series of reads to the stack for the 64-bit timestamp 
  #
  # @param stack  a cccusbreadoutlist::CCCUSBReadoutList object 
  #
  # @return ""
	public method sStamp {stack}


  # --------------- Utility method -------------------------------

	public method GetVariable {v} {set $v}
}

# --- END OF INTERFACE ---

#######################################################
#######################################################

# IMPLEMENTATIONS


#
#
#
itcl::body ATrigger2367::sClear {stack} {
  $stack addControl $node 0 9
}

#
#
#
itcl::body ATrigger2367::sClearRegister {stack} {
  $stack addControl $node 0 10
}

#
#
#
itcl::body ATrigger2367::sRead {stack} {
  $stack addRead24 $node 0 3
}

#
#
#
itcl::body ATrigger2367::sStamp {stack} {
  $stack addRead24 $node 1 3   ;# bits 0-15
  $stack addRead24 $node 2 3   ;# bits 16-31
  $stack addRead24 $node 3 3   ;# bits 32-47
  $stack addRead24 $node 4 3   ;# bits 48-63
}

#
#
#
itcl::body ATrigger2367::SetController {ctlr} {
  set device $ctlr 
}

#
#
#
itcl::body ATrigger2367::Clear {} {
  if {$device ne ""} {
    returns [$device simpleControl $node 0 9]
  } else {
    return -code error "ATrigget2367::Clear user must set the controller first with SetController" 
  }
}

#
#
#
itcl::body ATrigger2367::Go {bit} {
  if {![string is boolean $bit]} {
    return -code error "ATrigger2367::Go passed a non-boolean value"
  }

  set value [string is true $bit]

  if {$device ne ""} {
    return [$device simpleWrite24 $node 11 16 $value]
  } else {
    return -code error "ATrigget2367::Go user must set the controller first with SetController" 
  }
}

#
#
#
itcl::body ATrigger2367::Select {code} {
  if {$code<0 || $code>3} {
    return -code error "ATrigger2367::Select passed a value of range."
  }
  if {$device ne ""} {
    return [$device simpleWrite24 $node 12 16 $code]
  } else {
    return -code error "ATrigget2367::Select user must set the controller first with SetController" 
  }
}

#
#
#
itcl::body ATrigger2367::Enable {bits} {
  if {$device ne ""} {
    return [$device simpleWrite24 $node 13 16 $bits]
  } else {
    return -code error "ATrigget2367::Select user must set the controller first with SetController" 
  }
}


#
#
#
itcl::body ATrigger2367::ReadSignature1 {} {
  if {$device ne ""} {
    return [$device simpleRead24 $node 14 0 $bits]
  } else {
    return -code error "ATrigget2367::ReadSignature1 user must set the controller first with SetController" 
  }
}

#
#
#
itcl::body ATrigger2367::ReadSignature2 {} {
  if {$device ne ""} {
    return [$device simpleRead24 $node 15 0 $bits]
  } else {
    return -code error "ATrigget2367::ReadSignature2 user must set the controller first with SetController" 
  }
}

#
#
#
itcl::body ATrigger2367::Configure {firmware} {
# Read firmware bit file
  if {![file exists $firmware]} {
    return -code error -message "ATrigger2367::Configure firmware file $firmware for $self doesn't exist!"
  }

  # at this point, the file exists so let's open it
	if {[catch {set file [open $firmware r]} message]} {
    # oops we couldn't open it. 
		return -code error "ATrigger2367::Configure Cannot open firmware file $firmware for $self"
	}

  # check to make sure that we have a controller to talk to
  if {$device eq ""} {
    return -code error "ATrigger2367::Configure user must set the controller first with SetController"
  }

	fconfigure $file -translation binary
	set config [read $file]
	close $file
	set size [file size $firmware]

  # Read the entire file into a single array of bytes
	binary scan $config c$size bytes

  # Find first 0xff in firmware file
	set index 0
	while {[lindex $bytes $index] != -1} {incr index}

  # Init FPGA programming sequence in Xilinx chip
	$device simpleControl $node 0 30  ;# enter general programming mode
	$device simpleControl $node 0 28  ;# select CAMAC configuration mode
	$device simpleControl $node 0 25  ;# program Xilinx chip
	after 10                          ;# wait 10 ms before proceeding

  # Loop on firmware configuration until end
	for {set i $index} {$i < $size} {incr i} {
		$device simpleWrite24 $node 0 16 [lindex $bytes $i]
	}
  # Exit programming mode by clearing NA(0)F(9)
  Clear

  # Check configuration
  if {![IsConfigured]} {
    set msg "ATrigger2367::Configure Failed to configure module $self! "
    append msg "Firmware signatures were not verified."
		return -code error $msg 
	}
}

#
#
#
itcl::body ATrigger2367::IsConfigured {} {
  set type    [expr [ReadSignature2] & 0xffffff]
  set version [expr [ReadSignature1] & 0xffffff]
  return [expr ($type!=2367) || ($version!=5800)]
}
