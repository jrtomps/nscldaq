
11.0 Introduction
================

 vhq is a Tcl/Tk loadable package which provides Tcl/Tk drivers for the vhq202m
or vhq203m2 channel High voltage controller module from Spezialelektronik.  
To use the driver you must execute a:

package require vhq

in your tcl scripts.  The vhq driver functions live in the vhq namespace.

2.0 Commands:
=============

vhq::create base 

 Creates a new controller handle and map located at the specified base address.
See the VHQ202M documentation for information about how to set the VME base
address.  The handle to the controller is returned and must be used in 
subsequent driver calls e.g:

set vhq1 [vhq::create 0xdd00]


vhq::delete module

   Destroys the resources associated with a module handle and deletes the map
to the module.




vhq::id module

  Returns the bcd encoded serial number of the module.  
Example:

set vhq [vhq:: create 0xdd00]
puts "The serial number of the module is [vhq::id $vhq]"


vhq::stat1 module

   Returns the information from the module's status register 1.  This register
contains bit encoded status information about both channels.  The status is 
returned as a list of status information: {channel1_info channel2_info}.
Information about each status bit for each channel is returned as a list of
{bit_name bit_value} pairs. For the meaning of each bit refer to the vhq202m
manual.

Example:

set vhq [vhq::create 0xdd00]
vhq::stat1 $vhq
{{vz 1} {manual 0} {plus 0} {off 1} {kill 0} {rampup 0} {stable 0} {error 0}} 
{{vz 1} {manual 0} {plus 0} {off 1} {kill 0} {rampup 0} {stable 0} {error 0}}


vhq::stat2 module

  Returns information from the module's status register 2.  This register
contains instantaneous change of status information.  Once read the register
is reset to zero.  The register contents are returned in a similar format to 
that used by the vhq::stat1 command, however the status for each register is
preceded by the module timout information {{tot ?value?} {chan1} {chan2}
  For example:

set vhq [vhq::create 0xdd00]
vhq::stat2 $vhq
{tot 0} 
{{ilimit 0} {OpComplete 0} {FpChanged 0} {Voverset 0} {Inhibited 0} 
{OverVorI 0} {BadQuality 0}} 
{{ilimit 0} {OpComplete 0} {FpChanged 0} {Voverset 0} {Inhibited 0} 
{OverVorI 0} {BadQuality 0}}

refer to the vhq202m manual for the meanings of each of these bits.


vhq::rampspeed module a|b ?value?

  If the optional ?value? parameter is supplied, modifies the ramp speed for the specified channel (a or b).  The command returns the current rampspeed value.
   The following example increases the rampspeed of each channel by 10 v/sec.

set vhq [vhq::create 0xdd00]
set cura [vhq::rampspeed $vhq a]         ;# Current speed on chan a
vhq::rampspeed $vhq a [expr $cura + 10]  ;# increased by 10v/s
set curb [vhq::rampspeed $vhq b]         ;# current speed on chan b
vhq::rampspeed $vhq b [expr $curb + 10]  ;# increased by 10v/s.



vhq::setv module a|b ?value?

  If the optional ?value? parameter is present, a new voltage target is set and
a ramp towards that target is started.  The command returns the current target
value.  When the ramp is complete, the status register 2  OpComplete bit will
be set.  While the ramp is in progress, it can be monitored by watching the 
actual voltage and the bits in status register 1 (stable will be nonzero during
a ramp and rampup will indicate the direction of the ramp).
   The following example initiates a ramp up of 200V from the present target
value of channel 1 and a ramp down of 100V from the present target of channel
2:

set vhq [vhq::create 0xdd00]
set vta [vhq::setv $vhq a]
vhq::setv $vhq a [expr $vta + 200]
set vtb [vhq::setv $vhq b]
vhq::setv $vhq b [expr vtb - 100]



vhq::limit module v|c|i  a|b ?value?

   Examines the voltage limit which is set in the imax rotary switch.  If 
?value? is present, sets a new current limit (for c or i). Examines the current
limit.  Return value depends on which limit is selected: Voltage limits (v) 
return the current hardware voltage limit in 10% of module capacity (2kV)
Current limits return a two element list: {soft_limit hard_limit}.  The 
hard_limit is set by the front panel Ilimit rotary switch.  Both limits are in
10% of module capacity (3mA).
Example:

set vhq [vhq::create 0xdd00]
puts "Channel A Vlimit is [expr [vhq::limit $vhq v a] * 2.0/100.0 ] Kv"
Channel A Vlimit is 0.2k Kv
vhq::limit $vhq c a 90    ;# current limit at 90% of 3ma for channel a
vhq::limit $vhq i b 85    ;# and 85% for chan b note that i and c are 
#                            synonymous.


vhq::actual module a|b

  Returns the actual voltage and current for the specified channel as a list
{voltage current}

vhq::actual $vhq a
1000 0  
#      Voltage at 1Kv with no current drawn.




