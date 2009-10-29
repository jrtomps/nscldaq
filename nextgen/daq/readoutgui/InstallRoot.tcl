package provide InstallRoot 1.0
namespace eval InstallRoot {
variable root /usr/opt/daq/10.0/
proc Where {} {
variable root
return $root
}
}
