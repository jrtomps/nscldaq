1.0 Introduction
================

camac is a Tcl/Tk package which provides access to CAMAC crates connectd to
the data acquisition Linux systems via the CES CBD 8210 branch highway driver.

The package is dynamically loadable and can be incorporated into a running
Tcl/Tk derived program by issuing the command:

package require camac


2.0 Interesting details
=======================

2.1 camac Namespace:
==============

 All of the CAMAC top level procs live in the ::camac namespace.  The procs
described in this document may be imported into the top level namespace by

namespace import ::camac::procname

for example

namespace import ::camac::cdreg

2.2 How camac modules are accessed
==================================

   The bulk of the procs provided by this package operate in close 
correspondence to software reccomendations of the ESONE CAMAC standards 
committee.  Normally a programmer will indicate interest in a module by
getting a handle to that slot via the ::camac::cdreg proc.  
   This handle will be used in subsequent camac calls to manipulate the module.
For example:

set b0c2s1 [::camac::cdreg 0 2 1]   ;# handle to B=0,C=2,S=1
::camac::cfsa $b0c2s1 10 12         ;# Issue F=10, A=12 to that module.
set data [::camac::qcan $b0c2s1 0 0 16] ;# Use a qscan to read the module.

3.0 Function descriptions:
==========================

3.1 Functions with good ESONE fortran analogs:
==============================================

       ::camac::cdreg  b c n             

Produces a handle to a camac module. The handle is used in other camac
calls to refer to the module 

Example:
       set module [::camac::cdreg 0 2 17]

The variable module holds a handle to the module in slot 17 of crate 2 on 
branch zero.


       cfsa   reg f a ?d?       

Performs a camac operation.  If the operation is a data transfer operation, 
24 bits of data are transferred:
   reg - a handle gotten from ::camac::cdreg
   f   - The camac function code to execute.
   a   - The module subaddress to affect.
   ?d? - Optional data.  If the operation is a write, the low order 24 bits
         of this parameter will be written.

Returns a three element list of the form:  {data q x} where:
   data  - depends on the type of function code:
           Read [0-7]    - the data read from the module.
           Write [16-23] - The value of the d parameter.
           Control [8-15],
                 [24-31] - 0.
   q     - The Q response of the operation.
   x     - The module X response.

Example:

set module  [camac::cdreg 0 2 17]
set info    [camac::cfsa $module 2 0]   ;# perform B=0 C=2 N=17 A=0 F=2.
set data [lindex $info 0]       ;# Data read.
set q    [lindex $info 1]       ;# Q response
set x    [lindex $info 2]       ;# X response.
...
set q [lindex [camac::cfsa $module  17 0 123] 1] ;# q of B=0 C=2 N=17 A=0 F=17



       cssa   reg f a ?d?       - perform a 16 bit camac operation

Exactly the same as cfsa however data transfers will only provide the bottom
16 bits of the data.

       cblock reg f a num       - Perform a counted block transfer read.

Performs a counted block transfer the f parameter must be a read function code.
num is the number of times this function will be repeated.  The returned value
is a list of data read from the module. All data transfers are 24 bits.

Example:

set module [camac::cdreg 0 2 17] ;# assume this is some sort of memory module.
camac::cssa $module 16 0 0       ;# reset memory read pointer.
set data [camac::cblock $reg 0 0 128] ;# Read 128 words from memory.


       qstop  reg f a ?maxn?    - Perform a qstop read.

Performs a "QSTOP" block transfer from the module.   The same functionality as
for cblock however, the transfer stops when the module stops giving a Q 
response, or if the optional maxn number of words was read.  If maxn is not
given, only Q disappearing stops the read.

Example:

set flashadc [camac::cdreg 0 2 17]
set data     [camac::qstop $flashadc 0 0 1024]

The example reads as many words as are available from the 'flashadc' module
however at most 1024 words will be read.


       qscan  reg f a ?maxn?   

Perform a qscan read.  After each read operation (f must be a read), a
is incremented until a Q is not received.  When this happens, the slot
is incremented and a reset to zero.  If no X response is received or if the
optional maxn number of successful reads is reached, the operation terminates.
The data read is stored as a Tcl formatted list in the return from this
procedure.  

Example:

set firstScaler [camac::cdreg 0 1 1]   ;# first scaler.
set scalers     [camac::qscan $firstScaler 0 0]  ;# read scalers until empty
#                                                   slot.


3.2 Functions manipulating the CES 8210
=======================================

       isOnline b c 

Returns a boolean indicating whether or not the selected crate is online.
Note that if there is not a CES8210 module corresponding to the branch, the 
return result is undefined.

       getGl    b 

Gets the branch graded lam register for the selected branch.  Note that if
there is no CES8210 module corresponding to the branch, the return result
is undefined.

3.3 Functions manipulating Bi Ra 1302 Crate controllers
=======================================================

       C        b c

Perform a C-cycle (clear) on the specified crate.  Individual module 
reactions to this are described in module documentation.

       Z        b c

Perform a Z-cycle (zero) on the specified crate.  Individual module
reactions to this are described in the module documentation.

       isInhibited b c

Returns a boolean indicating whether or not the specified crate has the 
inhibit enabled (on).  The ability to manipulate modules  when the crate
inhibit is enabled varies and is described in the documentation of the
individual modules.

       Inhibit  b c  bool       - Set crate inhibit.

Sets or clears the inhibit of the selected crate according to the value
of bool.  The example below removes the inhibit from all crates in 
branch 0  if it is set but leaves the inhibit alone if not.  Note that Tcl/Tk 
interprets 'on' and 'off' as valid booleans with TRUE and FALSE values
respectively.

set b 0
for {set i 1} {$i < 8} {incr i} {
  if {[camac::isOnline $b $i]} {
     if {[camac::isInhibited $b $i]} {
       camac::Inhibit $b $c off
     }
  }
}


       ReadLams b c

Returns the value of the LAM mask for the current controller.  If bit n-1 is
set in the Lam Mask, then slot n currently has a lam.  The following example
reads the lam mask and converts it into a TCl formatted list of slots with 
lams.

proc LamList {mask} {
  set LamSLots ""
  for {set i 0} {$i < 24} {incr i} {
    if {((1 << $i) & $mask) != 0} {
      lappend LamSlots [expr $i + 1]
    }
  }
}

# .. Code omitted.

set Lams [LamList [ReadLams 0 2]]
foreach slot $Lams {

# ... Processing for lam slots omitted...

}
