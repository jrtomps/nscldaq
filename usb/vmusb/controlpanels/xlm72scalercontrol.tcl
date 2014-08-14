#!/usr/bin/env tclsh

lappend auto_path [file dirname [file normalize [info script]]]
lappend auto_path [file join /usr devopt nscldaq 11.0-s800conv TclLibs] 

package require Itcl 
package require xlm72scalerpanel

toplevel .root
::XLM72ScalerGUI aPanel .root xlm72sclrctl localhost 27000
wm protocol .root WM_DELETE_WINDOW { aPanel OnExit } 

grid rowconfigure    .root 0 -weight 1
grid columnconfigure .root 0 -weight 1

