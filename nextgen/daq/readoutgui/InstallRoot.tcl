package provide InstallRoot 1.0
namespace eval InstallRoot {
variable root /usr/opt/daq/8.2
proc Where {} {
variable root
return $root
}
}
