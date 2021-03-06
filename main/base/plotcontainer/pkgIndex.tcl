# Tcl package index file, version 1.1
# This file is generated by the "pkg_mkIndex" command
# and sourced either when an application starts up or
# by a "package unknown" script.  It invokes the
# "package ifneeded" command to set up package-related
# information so that packages will be loaded automatically
# in response to "package require" commands.  When this
# script is sourced, the variable $dir must contain the
# full path name of this file's directory.

package ifneeded Plotchart::marker 1.0 [list source [file join $dir marker.tcl]]
package ifneeded Plotchart::series 1.0 [list source [file join $dir series.tcl]]
package ifneeded Plotchart::xyplotContainer 1.0 [list source [file join $dir xyplot.tcl]]
