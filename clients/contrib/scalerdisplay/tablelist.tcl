#==============================================================================
# Main Tablelist package module.
#
# Copyright (c) 2000-2002  Csaba Nemethi (E-mail: csaba.nemethi@t-online.de)
#==============================================================================

package require Tcl 8
package require Tk  8

namespace eval tablelist {
    #
    # Public variables:
    #
    variable version	2.6
    variable library	[file dirname [info script]]

    #
    # Creates a new tablelist widget:
    #
    namespace export	tablelist

    #
    # Sorts the items of a tablelist widget based on one of its columns:
    #
    namespace export	sortByColumn
}

package provide Tablelist $tablelist::version

lappend auto_path [file join $tablelist::library scripts]
