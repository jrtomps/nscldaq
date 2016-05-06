#!/usr/bin/env tclsh

package require s800 
package require Thread 

proc end {} {
}

set port [lindex $argv 0]

s800rctl ::rctl -port $port

vwait forever
exit 0
