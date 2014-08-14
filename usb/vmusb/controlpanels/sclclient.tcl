
package provide ScalerClient 1.0
package require InstallRoot 

proc startScalerClient {{host localhost} {port 30999} {ringhost localhost} {ringname $::env(USER)}} {
# set up the name of binary and arguments
  set root [InstallRoot::Where]
  set sclClientBin [list [file join $root bin sclclient]]
  lappend sclClientBin "--host=$host"
  lappend sclClientBin "--port=$port"
  lappend sclClientBin "--source=tcp://$ringhost/$ringname"
  lappend sclClientBin "&"
  return [exec {*}$sclClientBin]
}
