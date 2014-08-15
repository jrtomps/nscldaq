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

# Interactive functions
	public method GetFirmware {ctlr}
	public method Reset {ctlr}
	public method Latch {ctlr}
	public method SetTrigger {ctlr bit}
	public method SetTriggerBit {ctlr bit value}
	public method ReadTrigger {ctlr}
	public method SetEnable {ctlr onoff}
	public method ReadEnable {ctlr}
	public method SetLive {ctlr}
	public method ReadAll {ctlr}

  public method SetTriggerBits {ctlr bitset}
# Stack functions
	public method sEnable  {stack}
	public method sDisable {stack}
	public method sLatch   {stack}
	public method sReadAll {stack}
	public method sReset   {stack}
}

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
# What does this do?
#
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
	set value [Read $ctlr fpga 0xc]
	if {$trigger($bit)} {
		set value [expr $value|(1<<$bit)]
	} else {
		set value [expr $value&(~(1<<$bit))]
	}
	ReleaseBus $ctlr

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

itcl::body AXLM72Scaler::ReadTrigger {ctlr} {
	AccessBus $ctlr 0x10000
	set value [Read $ctlr fpga 0xc]
	ReleaseBus $ctlr
	return $value 
}

##
# SetEnable
#
# Not sure what the purpose of this is.
#
#
itcl::body AXLM72Scaler::SetEnable {ctlr onoff} {
	AccessBus $ctlr 0x10000
	set status [Write $ctlr fpga 8 $onoff]
	ReleaseBus $ctlr
  return $status
}

##
# ReadEnable
#
# Not entirely sure what the purpose of this is.
#
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
##
## Control

snit::type AXLM72ScalerControl {
  option -slot 0

  variable scalers
  variable triggers
  variable enable
  variable firmware
  variable runstate 
  variable driver "" 

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
#    set ctlr [::VMUSBDriverSupport::convertVmUSB $driverPtr]
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
  # Set
  # 
  # Flags that are understood
  #  
  # \return transaction result  
  method Set {ctlr parameter value} {
    set ctlr [::VMUSBDriverSupport::convertVmUSB $ctlr]
    flush stdout

      # if the value is a list, take the first argument
    set key $parameter
    if {[scan $parameter {trigger%d} subkey] == 1} {
      set key trigger
      #subkey is already set
    }
    flush stdout

    switch $key {
      enable  {return [$self SetEnable $ctlr $value]}
      reset   {return [$self SetReset $ctlr]} 
      trigger {return [$self SetTrigger $ctlr $subkey $value]}
    }

  }

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
  # Get
  # 
  # This method is not used in this driver
  # 
  # \return error always
  method Get {ctlr parameter} {
    set ctlr [::VMUSBDriverSupport::convertVmUSB $ctlr]
    
    switch $parameter {
      enable {return [$self GetEnable $ctlr]}
      alltriggers {return [$self GetAllTriggers $ctlr]}
      firmware {return [$self GetFirmware $ctlr]}
      runstate {return [$self GetRunState $ctlr]}
      allscalers {return [$self GetAllScalers $ctlr]}
      default {error "$parameter is not supported in XLMScalerControl::Get"}
    }

  }

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
  # Add to the monitor list that is executed periodically by the tclserver. This creates
  # a new stack that the script will fill and then appends the result to the 
  # list passed as an argument.
  #
  # \param aList list to a VMUSB controller 
  method addMonitorList aList {

  }

  ## 
  # processMonitorList
  #
  # Handle the data returned from monitor list
  #
  # \param data a tcl list of data bytes remaining to be processed 
  # \return number of bytes processed
  method processMonitorList {data} {
    return 0
  }


  

}

