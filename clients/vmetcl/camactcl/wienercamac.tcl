#
#
#   Package to support subset ESONE CAMAC access from TCL.
#   using the wiener vc32/cc32 cards...
#
#   Lives in the namespace ::camac::
#   Exported procs:
#       cdreg  b c n             - Produce a handle to a camac module.
#       cfsa   reg f a ?d?       - Perform a 24 bit camac operation.
#       cssa   reg f a ?d?       - perform a 16 bit camac operation
#       qstop  reg f a ?maxn?    - Perform a qstop read.
#       qscan  reg f a ?maxn?    - Perform a qscan read
#       cblock reg f a num       - Perform a counted block transfer read.
# 
#  The following functions have no corresponding operation in the standard:
#
#       isOnline b c             - Tests for crate online.
#       getGl    b               - Reads the branch graded lam register.
#
#  The following functions assume that the crate controller is a wiener cc32
#
#       C        b c             - C cycle a crate
#       Z        b c             - Z cycle a crate.
#       isInhibited b c          - Check inhibit status of crate.
#       Inhibit  b c  bool       - Set crate inhibit.
#       ReadLams b c             - Read controller lam mask.
#

package provide wienercamac 1.0

namespace eval wienercamac {
    package require Vme

#    camac will describe a mapping of the entire branch highway space.

#   variable camac [vme create ces8210 -device /dev/vme24d16 0x800000 0x300000]

    variable camac
#    variable camac0 [vme create vc32_0 -device /dev/vme24d32 0x550000 0x8000]
#    variable camac1 [vme create vc32_1 -device /dev/vme24d32 0x558000 0x8000]
#    array set camac [list 0 $camac0 1 $camac1]
    set base 0x800000
    for {set i 0} {$i < 8} {incr i} {
	set camac($i) [vme create vc32_$i -device standard $base 0x8000]
	incr base 0x8000
    }
#    array set camac "0 $camac0 1 $camac1"

    namespace export cdreg cfsa cssa qstop qscan cblock
    namespace export isOnline getGl
    namespace export C Z isInhibited Inhibit ReadLams

    proc isValid {b c n} {
	if {($b < 0) || ($b > 7)} {
	    error "Branch $b is out of range"
	}
#--ddc change to match number of vc32's
	if {($c < 0) || ($c > 7)} {
	    error "Crate $c is out of range"
	}
	if {($n < 0) || ($n > 31)} {
	    error "Slot $n is out of range"
	}
    }
    proc isValidf {f} {
	if {($f < 0) || ($f > 31)} {
	    error "Function code $f is out of range"
	}
    }
    proc isValida {a} {
	if { ($a < 0) || ($a > 15)} {
	    error "Subaddress $a is out of range"
	}
    }

    proc isRead  {f} {
	return [expr ($f < 8)]
    }
    proc isWrite {f} { 
	return [expr ($f > 15) && ($f < 24)]
    }
    proc isCtl   {f} {
	return [expr !([isWrite $f] || [isRead $f])]
    }

    proc Encode {b c n {a 0} {f 0}} {
#--ddc change to match coding for vc32... NOTE! assuming BYTE addressing!
#	set reg [expr ($b << 19) | ($c << 16) | ($n << 10)]
#	set reg [expr $reg | ($a << 6) | ($f << 2)]
	set reg [expr ( ($c<<15)|($n<<10) | ($a << 6) | (($f&0xf) <<2))]
	return $reg
    }   
#
# --ddc for cc32 naf for status is 0,0,0 (check status, q, x from cc32)
# q bit 3, x bit 2, inhibit bit 1, Lam bit 0
#
    proc CSR {b crate} {
	set reg [Encode $b 0 0 0 0]
	set csr [$::wienercamac::camac($crate) get -w $reg ]
	return $csr
    }
    proc Q {b crate} {
	set csr [CSR $b $crate]    
	return [expr ($csr & 0x8) != 0]
    }
    proc X {b crate} {
	set csr [CSR $b $crate]
	return [expr ($csr & 0x4) != 0]
    }
    proc ExtractB {reg} {
#	return  [expr ($reg >> 19) & 0x7]	
	return  [expr 0]	
    }
    proc ExtractC {reg} {
# --ddc changed for cc32
	return [expr ($reg >> 15) & 0x7]
    }
    proc ExtractN {reg} {
# --ddc changed for cc32
	return [expr ($reg >> 10) & 0x1f]
    }
    proc IncN {reg} {
	set b [ExtractB $reg]
	set c [ExtractC $reg]
	set n [ExtractN $reg]
	incr n
	return [Encode $b $c $n]   ;# Error if n now out of range!
    }
}

#
#  cdreg b c n
#   Produces a 'handle' to a module in camac space at b c n
#   this handle should be passed into other procs which have a 'reg'
#   parameter. Returns the handle.  Flags an error if any of b c n are
#   invalid.
#
proc ::wienercamac::cdreg {b c n}  {

    isValid $b $c $n       ;# Error's if invalid camac op.
    return [wienercamac::Encode $b $c $n]
}

#
# cfsa reg f a ?d?
#     Performs a single camac operation.  If the operation involves
#     data transfer 24 bits are transferred.  If the operation is a 
#     write, d contains the data to write and the low order 24 bits are 
#     written..
#     proc returns a list of the form:
#       {data q x}
#     data is:
#        0 - If the operation was a control operation.
#        d - If the operation was a write operation.
#        ? - The data read from the module if the operation was a read.
#
proc ::wienercamac::cfsa {reg f a {data 0}} {
#
#     Return errors if the fcode or subaddress are bad.
#    
    wienercamac::isValidf $f
    wienercamac::isValida $a
    set br  [wienercamac::ExtractB $reg]

#
#   --ddc strip off "crate" (is now index to array of camac)
#
# (reg passed by val).

    set crate [wienercamac::ExtractC $reg]
    set reg [expr $reg&0x7fff | ($a << 6) | (($f&0xf) << 2) ] ;


    if {[wienercamac::isRead $f]} {		;# Read operation for 32 bits:
# --ddc I don't believe all these gymnastics are necessary...
#      set hi [$::wienercamac::camac get -w $reg]
#      set lo [$::wienercamac::camac get -w [expr $reg + 2]]
#      set data [expr (($hi & 0xff) << 16) | ($lo & 0xffff)]
       set data [$::wienercamac::camac($crate) get $reg]
    }
    if {[wienercamac::isWrite $f]} {		;# Write operation for 32 bits:

# --ddc Again, I don't think this is necessary...
#      set hi [expr (($data >> 16) & 0xff)]
#      set lo [expr $data & 0xffff]
#      $::wienercamac::camac set -w $reg $hi
#      $::wienercamac::camac set -w [expr $reg + 2] $lo
       $::wienercamac::camac($crate) set $reg $data
       set data $data
    }



    if { [wienercamac::isCtl $f] } {
	if { $f < 16 } {
	    set data [$::wienercamac::camac($crate) get $reg]
	} 
	if { $f > 16 } {
	    $::wienercamac::camac($crate) set $reg 0
	}
	set data 0
    }

#    Now pick up the q and x and build the reply list:
#--ddc ... use modified q/x procedures (supply additional crate param)
#
    lappend data [wienercamac::Q $br $crate]
    lappend data [wienercamac::X $br $crate]

    return $data
}
#
#  cssa reg f a ?data?
#
#    Same as cfsa but all data transfers are done as 16 bit transfers.
#
#

proc ::wienercamac::cssa {reg f a {data 0}} {
#
#     Return errors if the fcode or subaddress are bad.
#    
 
    wienercamac::isValidf $f
    wienercamac::isValida $a

    set br  [wienercamac::ExtractB $reg]

#    set reg [expr $reg | ($a << 7) | ($f << 2) + 2 ]  ;# (reg passed by val).
#
# --ddc   strip off "crate" (which is now index to array of camac)
#

# (reg passed by val).
    set crate [wienercamac::ExtractC $reg]
    set reg [expr $reg&0x7fff | ($a << 6) | (($f&0xf) << 2) ] ;

    if {[wienercamac::isRead $f]} {		;# Read operation for 16 bits:
      set data [$::wienercamac::camac($crate) get -w $reg]
    }
    if {[wienercamac::isWrite $f]} {		;# Write operation for 16 bits:
      set lo [expr $data&0x7fff]
      $::wienercamac::camac($crate) set -w $reg $lo
      set data $data
    }

    if { [wienercamac::isCtl $f] } {
	if { $f < 16 } {
	    set data [$::wienercamac::camac($crate) get -w $reg]
	}
	if { $f >= 16} {
	    $::wienercamac::camac($crate) set -w $reg 0
	}
	set data 0
    }

#    Now pick up the q and x and build the reply list:

    lappend data [wienercamac::Q $br $crate]
    lappend data [wienercamac::X $br $crate]

    return $data
}
#
#  qstop reg f a ?maxn?
#     Performs a qstop read on the single f a on the module designated by reg.
#     The results of the read are placed in a list which is returned.
#     maxn if supplied limits the number of reads which can be performed.
#     f must be a read function. reads are 24 bit reads.
#

proc ::wienercamac::qstop {reg f a {maxn -1}} {

    if {![::wienercamac::isRead $f]} {
	error "Qstop functions must be read operations"
    }
    set result ""

#      Default of maxn = -1 will give lots of reps on this register:

    while {$maxn != 0} {    
	set op [wienercamac::cfsa $reg $f $a]
	set q  [lindex $op 1]
	if {!$q} {return $result}
	lappend result [lindex $op 0]
	incr maxn -1
   }
   return $result
}
#
#  qscan reg f a ?maxn?
#    Performs a qscan read operation.  Qscans read a module incrementing
#    the subaddress after each operation.  If Q goes away or a >16
#    the scan continues with the next slot.  If X goes away, the scan
#    terminates.  The scan also terminates if the optional maxn
#    count is exceeded.
#
proc ::wienercamac::qscan {reg f a {maxn -1}} {
    if {![::wienercamac::isRead $f]} {
	error "Qscan functions must be read operations."
    }
    set result ""

    while {$maxn != 0} {
	set op [wienercamac::cfsa $reg $f $a]
	set data [lindex $op 0]
	set q [lindex $op 1]
	set x [lindex $op 2]
	if {!$x} {return $result}
	if {!$q} {
	    set a 0
	    set reg [::wienercamac::IncN $reg]
	    continue
	}
	lappend result $data
	incr a
	incr maxn -1
    }
    return $result
}
#
#  cblock reg f a num
#    Peforms an unconditional block read of num items from the 
#    same camac bcnaf.  The data read are returned as a list.
#
proc wienercamac::cblock {reg f a num} {
    if {![wiener::camac::isRead $f]} {
	error "Counted block operations must be reads"
    }
    set result ""
    for {set i 0} {$i < $num} {incr i} {
	lappend result [lindex [::wienercamac::cfsa $reg $f $a] 0]
    }
    return $result
}
#
#   isOnline b c
#     Informs you if the specified crate is online.
#
proc wienercamac::isOnline {b c} {
# --ddc modified encode, and expr returned for vc32.
    set reg [::wienercamac::Encode $b 0 0 0 3]
    set onlines [$::wienercamac::camac($c) get -w $reg]

    return [expr ($onlines>>14 & 1) != 0]
}
#
#   getGl b 
#     Get the graded lam register for the specified branch.
#     this is a 24 bit item.m
# --ddc This no longer makes sense.  What should be done to fix it?
proc wienercamac::getGl b {
    set reg [::wienercamac::Encode $b 0 29 0 10]
    set glhi [$::wienercamac::camac get -w $reg]
    set gllo [$::wienercamac::camac get -w [expr $reg + 2]
    return [expr ($gllo &0xffff) | (($glhi && 0xff) << 16)]
    
}
#
# C  b c   
#     Performs a C cycle of the selected crate.
#
proc wienercamac::C {b c} {
#--ddc modified NAF for cc32
    set reg [::wienercamac::cdreg $b $c 0]
    ::wienercamac::cssa $reg 16 0
}
#
#  Z b c
#     performs a Z cycle of the selected crate
#
proc wienercamac::Z {b c} {
#--ddc modified NAF for cc32
    set reg [::wienercamac::cdreg $b $c 0]
    ::wienercamac::cssa $reg 16 1
}
#
#  isInhibited b c
#     Returns a bool indicating if the crate is inhibited.
#
proc wienercamac::isInhibited {b c} {
#--ddc modified NAF for cc32
#    set reg [::wienercamac::cdreg $b $c 0]
#    return [lindex [::wienercamac::cssa $reg 0 0] 1]
    set csr [CSR $b $c]    
    return [expr ($csr & 0x2) != 0]
}
#
#  Inhibit b c bool
#     If bool is true the selected crate is inhibited, otherwise it
#     is uninhibited.
#
proc wienercamac::Inhibit {b c bool} {
#--ddc modified NAF for cc32
    set reg [::wienercamac::cdreg $b $c 27]

    if {$bool} {
	::wienercamac::cssa $reg 16 0
    } else {
	::wienercamac::cssa $reg 16 1
    }
}
#
#   ReadLams b c
#    Reads the graded lams on the specified crate:
# --ddc modified for CC32, reads "lam-flipflop"
#       this has not worked _yet_(8may02) for wiener vc32/cc32.
proc wienercamac::ReadLams {b c} {
    set reg [::wienercamac::cdreg $b $c 28]
    return [::wienercamac::cfsa $reg 0 1]
}







