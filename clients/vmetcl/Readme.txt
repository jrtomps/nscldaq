  This directory supplies a dynamically loadable .so package to support
access to the VME space and a package which uses this package to drive
a VME Caennet controller.  The following commands are useful and important:

package require Vme    ;# Load the Vme package into the interpreter.
package require caennet ;# load the caennet driver (loads Vme if needed).

1.0 Vme Package:
==================
   This package defines a single command "vme" which defines commands which
represent 'address maps' in the VME bus:

vme create mapname ?-device /dev/vme....? base size

   Creates a new command "mapname" which supplies access to a chunk of VME
address space starting at base and at least size bytes big.  -device specifies
the actual linux device driver through which the mapping is performed (and
hence the address modifiers):

shortio16    - A16D16 space (commonly called short I/O).
shortio      - A16D32 space short I/O with wide transfers.
standard16   - A24D16
standard     - A24D32
extended16   - A32D16
extended     - A32D32.
geographical - D32 geographical addressing.
multicast    - D32 Multicast control writes.
chainedblock - D32 Chained block transfer.
vme list

Lists the set of existing maps.  Each map is a list containing the name of the
map and the process virtual address of the base of the map.

vme delete mapname

Deletes a map.

The command mapname can be used as follows:

mapname set ?-l|-w|-b? offset value
  Deposits 'value' in the vme address at 'offset' relative to the requested
base of the map.  -l -w -b indicate longword, word, byte operations 
respectively with the default -l.

mapname -get ?-l|-w|-b offset
  Returns the value at 'offset' relative to the requested base of the map.
-l, -w -b indicatge long, word and byte reads respectively.

Example:

#
#   create a map to a caennet controller and reset it.
#
vme create caennet -device /dev/vme24d16 0x200000 10
caennet set -w 6 0xffff      ;# Reset the controller.
vme delete caennet

2.0 caennet driver
==================

  The caennet driver is a script package which requires, and loads the
Vme package.  

package require caennet

loads the driver.  All of the caennet top level scripts live in the caennet 
namespace and are exported:

::caennet::create base

Example:
   set ctl [::caennet::create 0x200000]

Creates a new controller handle for a caennet vme controller located at 'base'
offset in the VME space.

::caennet::reset device

Resets the caennet controller specified by 'device'.

Example:

set ctl [::caennet::create 0x200000]
::caennet::reset $ctl

::caennet::send device slave function ?data?


Sends a command packet to a caennet slave device and return sthe
reply data.  
  device   - handle to the caennet controller.
  slave    - number of slave (0-99)
  function - function code of the transfer.
  ?data?   - optional additional data (tcl list).

The command returns the reply data as a tcl list.  The first value in
the list is the status (should be zero).

Example:


#
#  Put out the device identifier string from the module in 
#  slave address 9:
#
set ctl [::caennet::create 0x200000]
::caennet::reset $ctl
set reply [::caennet::send $ctl 9 0]    ;# Request device id.
if {[lindex $reply 0] != 0} {
   error "Error returned from slave, [lindex $reply 0]"
}
set replydata [lrange $reply 1 end]
puts -nonewline "Module ids as: "
foreach character $replydata {
  puts -nonewline [format %c $character]
}
puts ""


::caennet::delete device

  Deletes the device.. disposing of the vme map corresponding to it.

Example:

set ctl [::caennet::create 0x200000]

...

::caennet::delete $ctl      ;# done with the controller.

