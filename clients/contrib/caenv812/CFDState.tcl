#
#   CFDState.tcl
#     This file provides the package CFDState
#   CFDState holds the state of a set of CFD V812 modules.
#   It does so by keeping an internal set of data structures:
#        Modules      -  A Tcl list of modules that have been created.
#        Thresholds -  An array indexed by module name.  Each element
#                                 is a list of the 16 threshold values for the module.
#         Widths       - An array indexed by module name.  Each element 
#			    is a 2 element list of the width values for the
#                                two banks of the module.
#	Deadtimes - An array indexed by module name.  Each element
#			   is a 2 element list of dead time values for the two
#                              banks of the module.
#	Multiplicities - An array indexed by module name.  Each element
#			   is the majority threshold value.
#       Bases   	- An array indexed by module name.  Each element
#			is the base address of the corresponding module.

package provide CFDState 1.0

namespace eval CFDState {
   variable Modules         ""
   variable Thresholds      
   variable Widths          
   variable Deadtimes     
   variable Multiplicities  
   variable Bases           

   set Thresholds(0) ""
   set Widths(0)     ""
   set Deadtimes(0)  ""
   set Multiplicities(0) ""
   set Bases(0) ""

   # Create a new module.  All data for the module
   # are set to default values (via a call to reset).
   # It is an error to create a module that already exists.
   # use Destroy to get rid an existing module.
   #  name  - The name of the module.
   proc Create {name} {
      variable Modules
      if {[lsearch -exact $Modules $name] == -1 } {
	 lappend Modules $name
	 Reset $name
      } else {
	 error "Duplicate module $name" CFDState::Create
      }
   }
   #
   #   Destroy a module.  All the data for the module are
   #   deleted. If the module does not exist, an error is thrown.
   #
   #   name - The name of the module to destroy.
   proc Destroy {name} {
       variable Modules    
      variable Thresholds  
      variable Widths      
      variable Deadtimes   
      variable Multiplicities  
      variable Bases       


       set element [lsearch -exact $Modules $name]
      if {$element != -1 } {
	 unset Thresholds($name) Widths($name) Deadtimes($name)
	 unset Multiplicities($name) Bases($name)
	 set Modules [lreplace $Modules $element $element]
      } else {
	 error "Module $name does not exist" CFDState::Destroy
      }
   }
   #  Reset - sets an existing module to its default state:
   #     Thresholds are all the way up.
   #     Widths are all the way down.
   #     Deadtimes are zero.
   #	 Multiplicities are set to 1.
   #	Bases, are left alone if they exist, or are
   #    set to 0 if they don't.
   #
   #  name - The name of the module to reset.
   #  If the module does not exist, an error is signaled?
   proc Reset {name} {
      variable Modules
      variable Thresholds
      variable Widths
      variable Deadtimes
      variable Multiplicities
      variable Bases
 
      if {[lsearch -exact $Modules $name] != -1} {
	 set Thresholds($name) ""
	 for {set i 0} {$i < 16} {incr i} {
	    lappend Thresholds($name) 0
	 }
	 set Widths($name)     "0 0"
	 set Deadtimes($name)  "0 0"
	 set Multiplicities($name) 1
	 if { ! [array exist Bases]}  {  ;# No array.
	    set Bases($name) 0
	 }
	 if {[lsearch -exact [array names Base $name]] == -1} {
	    set Bases($name) 0
	 }
      } else {
	 error "Module $name does not exist" CFDState::Reset
      }
   }
   
   # Set the value of a single threshold.
   #   name    - Name of the CFD.
   #   channel - Number of the channel to set.
   #   value   - Value of the threshold.
   #  Errors are signalled if:
   #     The name does not correspond to an existing cfd or
   #     The channel is invalid.
   proc SetThreshold {name channel value} {
      variable Modules 
      variable Thresholds

       
      if {[lsearch -exact $Modules $name] != -1} {
	 if {($channel >= 0) && ($channel  <= 15) } {
	    set Thresholds($name) [lreplace $Thresholds($name) $channel $channel $value]
	 } else {
	    error "Channel $channel is out of range" CFDState::
	 }
      } else {
	 error "Module $name does not exist" CFDState::SetThreshold
      }
   }
   # Gets the value of a singel threshold.
   #   name	- Name of the CFD
   #   channel	- Number of the channel to set.
   #  Errors are signalled if:
   #     The name does not correspond to an existing cfd or
   #     The channel is invalid.
   proc GetThreshold {name channel} {
      variable Modules 
      variable Thresholds
      
      if {[lsearch -exact $Modules $name] != -1} {
	 if {($channel >= 0) && ($channel  <= 15) } {
	    return [lindex $Thresholds($name) $channel]
	 } else {
	    error "Channel $channel is out of range" CFDState::
	 }
      } else {
	 error "Module $name does not exist" CFDState::SetThreshold
      }
   }
   #  Sets the value of a bank of widths. 
   #  A bank of widths is a set of 8 channels.
   #   name 	- Name of the CFD.
   #   bank     - 0 - to set channels 0-7,
   # 		  1 - to set channels 8-15.
   #   value	- Value to set for the width.
   # Errors are sigalled if:
   #	The module has not been defined.
   #    The bank is invalid.
   proc SetWidth {name bank value} {
      variable Modules
      variable Widths
   
      if {[lsearch -exact $Modules $name] != -1} {
	 if {($bank == 0) || ($bank == 1) } {
	    set Widths($name) [lreplace $Widths($name) \
			   $bank $bank $value]
	 } else {
	    error "Bank $bank is invalid" CFDState::SetWidth
	 }
      } else {
	 error "Module $name does not exist" CFDState::SetWidth
      }
   }
   #  Gets a width from a bank of a module.
   #  A bank is 8 channels of them odule.
   #   name	- Name of CFD module.
   #   bank     - 0 - to set channels 0-7,
   # 		  1 - to set channels 8-15.
   proc GetWidth {name bank} {
      variable Modules
      variable Widths

   
      if {[lsearch -exact $Modules $name] != -1} {
	 if {($bank == 0) || ($bank == 1) } {
	    return [lindex $Widths($name) $bank] 
	 } else {
	    error "Bank $bank is invalid" CFDState::GetWidth
	 }
      } else {
	 error "Module $name does not exist" CFDState::GetWidth
      }

   }
   
   #  Sets the deadtime for a bank.  A bank consists of
   #  8 channels of the module.
   #  name	- Name of the CFD
   #  bank	- 0 - set channels 0-7,
   #		  1 - set channels 8-15.
   #  value	- The value to set the deadtime to.
   proc SetDeadtime {name bank  value} {
      variable Modules
      variable Deadtimes


      if {[lsearch -exact $Modules $name] != -1} {
	 if {($bank == 0) || ($bank == 1) } {
	    set Deadtimes($name) [lreplace $Deadtimes($name) \
			   $bank $bank $value]
	 } else {
	    error "Bank $bank is invalid" CFDState::SetDeadtime
	 }
      } else {
	 error "Module $name does not exist" CFDState::SetDeadtime
      }
   }
   #  Gets the dead time for a bank.  A bank consists of
   #	8 channels of the module.
   #  name	- Name of the CFD
   #  bank	- 0 - set channels 0-7,
   #		  1 - set channels 8-15.
    proc GetDeadtime {name bank} {
      variable Modules
      variable Deadtimes
      if {[lsearch -exact $Modules $name] != -1} {
	 if {($bank == 0) || ($bank == 1) } {
	    return [lindex $Deadtimes($name) $bank]
	 } else {
	    error "Bank $bank is invalid" CFDState::GetDeadtime
	 }
      } else {
	 error "Module $name does not exist" CFDState::GetDeadtime
      }

    }
   # Set the multiplicity threshold.
   # name	- Name of the CFD.
   # value	- Multiplicity value. 
   # Errors are thrown if:
   #   The module does not exist.
   #   The value is not valid.
   proc SetMultiplicity {name value} {
      variable Modules
      variable Multiplicities
      
      if {[lsearch $Modules $name] != -1} {
	 if {($value >= 1) && ($value <= 20)} {
	    set Multiplicities($name) $value
	 } else {
	    error "Multiplicity $value is invalid" $CFDState::SetMultplicity
	 }
      } else {
	 error "Module $name does not exist." $CFDState::SetMultiplicity
      }
   }
   # Get the multiplicity threshold.
   # name	- Name of the CFD.
   # Errors are thrown if:
   #   The module does not exist.
   #   The value is not valid.
   proc GetMultiplicity {name} {
      variable Modules
      variable Multiplicities

      if {[lsearch $Modules $name] != -1} {
	 return  $Multiplicities($name) 
      } else {
	 error "Module $name does not exist." $CFDState::SetMultiplicity
      }
   }

   #  Set the module base address.
   #  name	- The module name.
   #  base	- The new base address.
   proc SetBase {name base} {
      variable Modules 
      variable Bases
      
      if {[lsearch -exact $Modules $name] != -1} {
	 set Bases($name) $base
      } else {
	 error "No such module $name" CFDState::SetBase
      }
   }
   #  Get the module base address.
   # name	- the module name.
   proc GetBase {name} {
       variable Modules
       variable Bases

      if {[lsearch -exact $Modules $name] != -1} {
	 return $Bases($name)
      } else {
	 error "No such module $name" CFDState::SetBase
      }
   }
   #  Write a module to file so that it can be read back in.
   #  The data are written so that a simple source can
   #  pull it all back in.
   #  file	- File descriptor open on the file.
   #  name	- The name of the module to write.
   proc Write {file name} {
      variable Modules
      variable Thresholds
      variable Widths
      variable Deadtimes
      variable Multiplicities
      variable Bases
      
      if {[lsearch -exact $Modules $name] != -1} {
	 puts $file "package require CFDState"
	 puts $file "catch \"CFDState::Destroy $name\""
	 puts $file "CFDState::Create $name"
	 for {set i 0} {$i < 16} {incr i} {
	    puts $file "CFDState::SetThreshold $name $i [ \
		  lindex $Thresholds($name) $i]"
	 }
	 puts $file "CFDState::SetWidth $name 0 [ \
		  lindex $Widths($name) 0]"
	 puts $file "CFDState::SetWidth $name 1 [ \
		  lindex $Widths($name) 1]"
	 puts $file "CFDState::SetDeadtime $name 0 [ \
		  lindex $Deadtimes($name) 0]"
	 puts $file "CFDState::SetDeadtime $name 1 [ \
		  lindex $Deadtimes($name) 1]"
	 puts $file "CFDState::SetMultiplicity $name \
	       $Multiplicities($name)"
	 puts $file "CFDState::SetBase $name $Bases($name)"
      } else {
	 error "No such module $name" CFDState::Write
      }
   }
}
