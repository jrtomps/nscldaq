#
# iwidgets.tcl
# ----------------------------------------------------------------------
# Invoked automatically by [incr Tk] upon startup to initialize
# the [incr Widgets] package.
# ----------------------------------------------------------------------
#  AUTHOR: Mark L. Ulferts               EMAIL: mulferts@spd.dsccc.com
#
#  @(#) $Id$
# ----------------------------------------------------------------------
#                Copyright (c) 1995  Mark L. Ulferts
# ======================================================================
# See the file "license.terms" for information on usage and
# redistribution of this file, and for a DISCLAIMER OF ALL WARRANTIES.

package require Tcl 
package require Tk 
package require Itcl 
package require Itk 

namespace eval ::iwidgets {
    namespace export *

    variable library [file dirname [info script]]
    variable version 4.0.1
}

lappend auto_path $iwidgets::library \
		  [file join $iwidgets::library generic] \
		  [file join $iwidgets::library scripts]
package provide Iwidgets $iwidgets::version
