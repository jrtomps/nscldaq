#!/usr/bin/env tclsh

lappend auto_path [file dirname [file normalize [info script]]]
lappend auto_path [file join /usr devopt nscldaq 11.0-s800conv TclLibs] 

package require Itcl 
package require xlm72scalerpanel

set parent .root
toplevel $parent
::XLM72ScalerGUI aPanel $parent xlm72sclrctl localhost 27000
wm protocol $parent WM_DELETE_WINDOW { aPanel OnExit } 

grid rowconfigure    $parent 0 -weight 1
grid columnconfigure $parent 0 -weight 1

