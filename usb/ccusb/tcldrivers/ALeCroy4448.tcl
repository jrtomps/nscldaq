#===================================================================
# class ALeCroy4448
#===================================================================

package provide lecroy4448 1.0 

package require cccusb
package require cccusbreadoutlist
package require CCUSBDriverSupport


##
# Driver for controlling a LeCroy 4448 coincidence register. The
# driver is designed for use with the the cccusb and cccusbreadoutlist
# packages. 
#
itcl::class ALeCroy4448 {
  ## a CCUSB controller
	private variable device 

  ## the slot in the crate
	private variable node

  ## a reference to the name of the instance
	private variable self
	
  ##
  # Constructor
  #
  # Creates an instance of this class. 
  #
  # \param de a swig-wrapped cccusb::CCCUSB device
  # \param no the slot number in which the device resides 
  #
  # \return the name of the object
	constructor {de no} {
		set device $de
    if {![::validSlotNumber $no]} {
      set node $no
    }
		set self [string trimleft $this :]
	}
	
  ## 
  # Destructor
  #
  # This does nothing.
	destructor {}
	
  ##
  # GetVariable
  #
  # The purpose of this is not clear at all but I will leave it.
  #
  # \param v the name of a value to dereference
  #
  # \return the value of the variable named in the argument
	public method GetVariable {v} {set $v}

  ##
  # Clear
  #
  # Interactively clear the module
  #
  # \return the QX code returned from the control operation 
	public method Clear {} {return [$device simpleControl $node 0 11]}

  ##
  # sClear
  #
  # Add a clear to the stack passed into it as an argument 
  #
  # \param stack a cccusbreadoutlist::CCCUSBReadoutList object
	public method sClear {stack}

  ## 
  # sRead
  #
  # Add procedures to read a register to the stack specified.
  #
  # \param stack    a cccusbreadoutlist::CCCUSBReadoutList object
  # \param register
  
	public method sRead {stack register}
}


##
# sClear 
#
# Adds a clear, A(0)F(11), of the module to a cccusbreadoutlist
# type of stack object.
itcl::body ALeCroy4448::sClear {stack} {
	set A 0
	set F 11
  $stack addControl $node $A $F 
}


##
# sRead
#
# Add a register readout
#
# \param stack a cccusbreadoutlist::CCCUSBReadoutList object
# \param register an identifier of which register to read. valid
#                 values are A, B, or C
itcl::body ALeCroy4448::sRead {stack register} {
	if {[string equal $register A]} {
    set reg 0
  } elseif {[string equal $register B]} {
    set reg 1
  } elseif {[string equal $register C]} {
    set reg 2
  } else {
    error "ALeCroy4448::sRead invalid argument. Register argument must be " \
          "either A, B, or C and user provided $register"
  }
	set A $reg
	set F 0
	$stack addRead16 $node $A $F 
}
