#===================================================================
# class AXLM72Scaler
# D. Bazin, Dec. 2009
# These functions are based on the FPGA configuration named ech32x24
# which contains 32 scalers of 24 bit depth each, as well as a trigger
# output selectable from the bits set in the trigger variable
#===================================================================

package provide scalerxlm72 1.0
package require xlm72
package require Itcl
package require snit 
package require VMUSBDriverSupport 


## Low-level driver for interactions with the XLM72 running the ech32x24.bit
# firmware
#
# This device is mostly just a 32-bit latching scaler but has the feature
# that channel inputs can be redirected as a trigger output. The trigger 
# output is the result of OR'ing all of the input signals of channels
# whose trigger bits are set. 
#
# The scaler can be enabled/disabled and atomically cleared (aka reset).
#
#
#
#


itcl::class AXLM72Scaler {
	inherit AXLM72
	
  private variable trigger

	constructor {sl} {
		AXLM72::constructor $sl
	} {
		# This is for a SpartanXL version of the module (24 bits scalers)
		for {set i 0} {$i < 32} {incr i} {
			set trigger($i) 0
		}
  }

  ##########################################################
  ##########################################################
  #
  # Interactive functions
  ###

  ## Read the firmware value
  #
  # @param ctlr a cvmusb::CVMUSB object
  # @returns integer
  # @retval the firmware signature 
	public method GetFirmware {ctlr}
  
  ## Atomically clear all scalers
  #
  # This is achieved by writing a 1 and then a 0
  # to the lowest address in the fpga. 
  # 
  # @param ctlr a cvmusb::CVMUSB object
  # 
  # @return int
  # @retval  0 - success
  # @retval -1 - failure during either of the two writes
  # 
	public method Reset {ctlr}

  ## Latch the scaler values into SRAMA 
  # 
  # @param ctlr a cvmusb::CVMUSB object
  # 
  # @return int
  # @retval  0 - success
  # @retval -1 - failure during either of the two writes
  # 
	public method Latch {ctlr}

  ###########################################################
  ###########################################################
  # Trigger register manipulators

  ## Write the value of trigger($bit) to the trigger reg 
  #
  # As a result of history, this relies on the state of the
  # trigger array to determine what to write. This ultimately
  # calls SetTriggerBits after properly setting the requested
  # bit.
  #
  # @param ctlr a cvmusb::CVMUSB object
  # @param bit  the bit to change
  #
  # @return int
  # @retval  0 - success
  # @retval -1 - failed during write
	public method SetTrigger {ctlr bit}

  ## Set a specific value for a certain trigger bit
  #
  # This first sets the value of the trigger($bit)
  # and then calls SetTrigger.
  #
  # @param ctlr a cvmusb::CVMUSB object
  # @param bit  the bit to change
  # 
  # @return int
  # @retval  0 - success
  # @retval -1 - failure during write
	public method SetTriggerBit {ctlr bit value}

  ## Write the trigger register
  # 
  # As opposed to the previous two methods, this replaces the
  # entire trigger register value rather than manipulating 
  # specific bits. It is therefore a lower level method but
  # it enables one to write all bits at once.
  #
  # @param ctlr a cvmusb::CVMUSB object
  # @param bitset  the bitset to write
  # 
  # @return int
  # @retval  0 - success
  # @retval -1 - failure during write
  public method SetTriggerBits {ctlr bitset}

  ## Read the trigger register
  # 
  # @param ctlr a cvmusb::CVMUSB object
  # 
  # @return int
  # @retval the value of the trigger register
	public method ReadTrigger {ctlr}
 
  ## Enable or disable the scaler channels from counting 
  # 
  # Write the value of the enable register
  #
  # @param ctlr a cvmusb::CVMUSB object
  # @param onoff boolean value determing whether scalers are enabled 
  # 
  # @return int
  # @retval  0 - success
  # @retval -1 - failure during write
	public method SetEnable {ctlr onoff}

  ## Read whether scalers are enabled or disabled
  # 
  # Read the value of the enable register
  # 
  # @param ctlr a cvmusb::CVMUSB object
  #
  # @return int
  # @retval 0 - scalers are disabled
  # @retval 1 - scalers are enabled
	public method ReadEnable {ctlr}

  ## Read all of the scaler channels 
  #  
  # A readout cycle produced by this method begins with
  # the latching of the scaler values into SRAMA. Then 
  # a block read (A32/D32) is executed to read the first
  # 32 integers from the SRAMA memory.
  #
  # @param ctlr a cvmusb::CVMUSB object
  # 
  # @return list of all 32 scaler values beginning with ch.0
	public method ReadAll {ctlr}

#########################################################
########################################################
#
# Stack manipulation functions
#
# These will be a bit more terse because they do exactly the
# same thing as their interactive forms
###

  ## add a write to stack that sets enable register to 1 
  #
  # @param stack a cvmusbreadoutlist::CVMUSBReadoutList object
	public method sEnable  {stack}
  
  ## Add a write to stack that sets enable register to 0
  #
  # @param stack a cvmusbreadoutlist::CVMUSBReadoutList object
	public method sDisable {stack}
  
  ## Add a write to the stack that latches the scaler values
  #
  # @param stack a cvmusbreadoutlist::CVMUSBReadoutList object
	public method sLatch   {stack}
 
  ## Add a read out cycle to the stack (latch, delay, blt)
  # 
  # This differs slightly from the interactive form ReadAll
  # because it adds a 4us execution delay into the stack.
  # @param stack a cvmusbreadoutlist::CVMUSBReadoutList object
	public method sReadAll {stack}
  
  ## Add a write to the stack to clear the scalers
  #
  # @param stack a cvmusbreadoutlist::CVMUSBReadoutList object
	public method sReset   {stack}
} 
## End of AXLM72Scaler class

# Interactive functions implementation
itcl::body AXLM72Scaler::GetFirmware {ctlr} {
	AccessBus $ctlr 0x10000
	set value [Read $ctlr fpga 0]
	ReleaseBus $ctlr
  set fw [format 0x%8x $value]
	return $fw 
}

##
# Reset 
# 
# Clears the scalers by toggling a bit
itcl::body AXLM72Scaler::Reset {ctlr} {
	AccessBus $ctlr 0x10000
	set st0 [Write $ctlr fpga 0 1] 
  set st1 [Write $ctlr fpga 0 0]
	ReleaseBus $ctlr

  set status [expr $st0==0 && $st1==0]
  if {$status == 0 } {
    return -1 
  } else {
    return 0
  }
}

##
# Latch Scaler values
#
# Causes the current scaler values to be written 
# into SRAMA for the next readout cycle.
#
itcl::body AXLM72Scaler::Latch {ctlr} {
	AccessBus $ctlr 0x10000
	set st0 [Write $ctlr fpga 4 1]; # set latch bit
	set st1 [Write $ctlr fpga 4 0]; # reset it
	ReleaseBus $ctlr; # once bus is released, FPGA takes over and writes scalers to SRAMA

  set status [expr $st0==0 && $st1==0]
  if {$status == 0 } {
    return -1 
  } else {
    return 0
  }
}

## 
# SetTriggerBit
#
# Updates the shadow state of the trigger register
# and then writes the value to the module.
#
itcl::body AXLM72Scaler::SetTriggerBit {ctlr bit value} {
	set trigger($bit) $value
	return [SetTrigger $ctlr $bit]
}

##
# Based on the shadow state of the trigger register
# which is contained in the trigger array, the bit
# specified is either turned on or off.
#
itcl::body AXLM72Scaler::SetTrigger {ctlr bit} {
	AccessBus $ctlr 0x10000

  # read the trigger register and then manipulate the
  # desired bit
	set value [Read $ctlr fpga 0xc]
	if {$trigger($bit)} {
		set value [expr $value|(1<<$bit)]
	} else {
		set value [expr $value&(~(1<<$bit))]
	}
	ReleaseBus $ctlr

  # write the new value back to the register
  return [SetTriggerBits $ctlr $value]
}

##
# Quick way to write value to the entire trigger register 
#
itcl::body AXLM72Scaler::SetTriggerBits {ctlr bitset} {
	AccessBus $ctlr 0x10000
	set status [Write $ctlr fpga 0x0c $bitset]
	ReleaseBus $ctlr
  return $status
}

# 
itcl::body AXLM72Scaler::ReadTrigger {ctlr} {
	AccessBus $ctlr 0x10000
	set value [Read $ctlr fpga 0xc]
	ReleaseBus $ctlr
	return $value 
}

##
# Set enable register
#
#
itcl::body AXLM72Scaler::SetEnable {ctlr onoff} {
	AccessBus $ctlr 0x10000
	set status [Write $ctlr fpga 8 $onoff]
	ReleaseBus $ctlr
  return $status
}

##
# Read the enable register
#
# Not entirely sure what the purpose of this is.
#
itcl::body AXLM72Scaler::ReadEnable {ctlr} {
	AccessBus $ctlr 0x10000
  set reg [Read $ctlr fpga 8]
	set enable [expr $reg&1]
	ReleaseBus $ctlr
  return $enable
}

##
# ReadAll scaler values
#
# Latches the current scaler values and then
# performs a block transfer read from the SRAMA
# address. There are 32 transfers for the 32
# channels.
#
itcl::body AXLM72Scaler::ReadAll {ctlr} {

	Latch $ctlr
	AccessBus $ctlr 1
	set data [ReadSBLT $ctlr srama 0 32]
	ReleaseBus $ctlr
  
  return [::VMUSBDriverSupport::convertBytesListToTclList data]
}

# Stack functions implementation
itcl::body AXLM72Scaler::sEnable {stack} {
	sAccessBus $stack 0x10000
	sWrite $stack fpga 8 1
	sReleaseBus $stack
}

itcl::body AXLM72Scaler::sDisable {stack} {
	sAccessBus $stack 0x10000
	sWrite $stack fpga 8 0
	sReleaseBus $stack
}

itcl::body AXLM72Scaler::sLatch {stack} {
	sAccessBus $stack 0x10000
	sWrite $stack fpga 4 1
	sWrite $stack fpga 4 0
	sReleaseBus $stack
}

itcl::body AXLM72Scaler::sReadAll {stack} {
	sLatch $stack
	$stack addDelay 20
	sAccessBus $stack 1
	sReadSBLT $stack srama 0 32
	sReleaseBus $stack
}

itcl::body AXLM72Scaler::sReset {stack} {
	sAccessBus $stack 0x10000
	sWrite $stack fpga 0 1
  sWrite $stack fpga 0 0
	sReleaseBus $stack
}


#############################################################
############################################################
#
# AXLM72ScalerControl 
#

## Plug in for the slow controls server. 
#
# This is the companion class of the XLM72SclrGUICtlr class. 
# When properly configured, the slow controls server will 
# direct requests to AXLM72Scaler driver that it owns. It is 
# actually a very thin class that maintains no state. It is mostly
# just a relay. 
#
# The AXLM72ScalerControl class is a slow controls driver and
# implements the Get, Set, Update, addMonitorList, processMonitorList
# interface. At the moment it only implements the Get and Set 
# methods.
#

snit::type AXLM72ScalerControl {
  option -slot -default 0   ;#< the slot in which the XLM72 resides

  variable driver ""        ;#< an instance of the AXLM72Scaler driver

  ## 
  # Construct the instance
  # 
  # It is here that we instantiate a driver 
  #
  constructor args {
    $self configurelist $args
    set driver [AXLM72Scaler #auto $options(-slot)]
  }

  # The CControlHardware interface

  ## 
  # Initialize
  #
  # Handle initialization routines that are needed by the
  # device. This will execute the script defined by the -initscript option.
  # This script is also executed if the init command is called without any
  # alternate control configuration file. 
  #
  # \param driverPtr a pointer to a VMUSB controller
  #
  # \return OK always. 
  method Initialize driverPtr {
    $self Update $driverPtr

  }

  ## 
  # Update
  #
  # When an update command is called, the script located by the -updatescript
  # is executed in the global namespace.
  #
  # \param driverPtr a pointer to a VMUSB controller
  #
  # \return OK always. 
  method Update driverPtr {
    set ctlr [::VMUSBDriverSupport::convertVmUSB $driverPtr]
  }

  ##
  # Response to a Set command
  # 
  # The AXLM72ScalerControl handles 3 parameters:
  #  trigger%d (where %d is an integer for the desired channel)
  #  enable
  #  reset  
  # All other parameters will result in an error
  #  
  # \return transaction result
  #
  # Exceptional return when parameter is not understood  
  method Set {ctlr parameter value} {
    # convert the ctlr to something usable
    set ctlr [::VMUSBDriverSupport::convertVmUSB $ctlr]

    # if the value is a list, take the first argument
    set key $parameter
    if {[scan $parameter {trigger%d} subkey] == 1} {
      set key trigger
      # we are already done with retrieving the subkey
    }

    # switch yard for relaying the different responses to appropriate handlers
    switch $key {
      enable  {return [$self SetEnable $ctlr $value]}
      reset   {return [$self SetReset $ctlr]} 
      trigger {return [$self SetTrigger $ctlr $subkey $value]}
      default {error "AXLM72ScalerControl::Set does not support '$key' as a valid parameter"}
    }

  }

  ######################
  ## Set Handlers

  method SetEnable {ctlr value} {
    return [$driver SetEnable $ctlr $value]
  }

  method SetReset {ctlr} {
    return [$driver Reset $ctlr]
  }

  method SetTrigger {ctlr ch value} {
    return [$driver SetTriggerBit $ctlr $ch $value]
    
  }

  ##
  # Respond to a Get command
  # 
  # Given a valid parameter name, this calls the appropriate handler.
  # Five separate parameters are supported by this:
  #  # enable 
  #  # alltriggers
  #  # firmware
  #  # runstate
  #  # allscalers
  # Any parameter name other than these will result in an exceptional return
  #
  # @param ctlr       a cvmusb::CVMUSB object
  # @param parameter  name of parameter requested
  #
  # @return result of request
  method Get {ctlr parameter} {
    # convert swig pointer into something useful
    set ctlr [::VMUSBDriverSupport::convertVmUSB $ctlr]
    
    # relay parameters to appropriate handlers
    switch $parameter {
      enable {return [$self GetEnable $ctlr]}
      alltriggers {return [$self GetAllTriggers $ctlr]}
      firmware {return [$self GetFirmware $ctlr]}
      runstate {return [$self GetRunState $ctlr]}
      allscalers {return [$self GetAllScalers $ctlr]}
      default {error "$parameter is not supported in XLMScalerControl::Get"}
    }

  }
 
  #######################
  ## Get Handlers 

  method GetEnable {ctlr} {
    return [$driver ReadEnable $ctlr] 
  }

  method GetAllTriggers {ctlr} {
    set data [$driver ReadTrigger $ctlr] 
    return $data
  }

  method GetFirmware {ctlr} {
    set fw [$driver GetFirmware $ctlr]
    return $fw 
  }

  method GetRunState {ctlr} {
    set state [runstate]
    return $state 
  }

  method GetAllScalers {ctlr} {
    set data [$driver ReadAll $ctlr] 

    return $data
  }

  ## 
  # addMonitorList
  # 
  #
  # NO-OP
  #
  # \param aList list to a VMUSB controller 
  method addMonitorList aList {

  }

  ## 
  # processMonitorList
  #
  # NOOP becase we didn't define a monitor list
  #
  # \param data a tcl list of data bytes remaining to be processed 
  # \return number of bytes processed
  method processMonitorList {data} {
    return 0
  }


  

}

