#!/usr/bin/env tclsh

lappend auto_path [file dirname [file normalize [info script]]]
lappend auto_path [file join /usr devopt nscldaq 11.0-s800conv TclLibs] 

package require Itcl 
package require xlm72scalerpanel

set parent .root
toplevel $parent -background lightblue

# Create the widget
::XLM72ScalerGUI aPanel $parent xlm72sclrctl localhost 27000

wm protocol $parent WM_DELETE_WINDOW { aPanel OnExit } 
wm title $parent "AXLM72Scaler Control Panel: ::aPanel"
wm resizable $parent false false

