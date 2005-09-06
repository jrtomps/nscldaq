package provide InstallRoot 1.0
namespace eval InstallRoot {
variable root /scratch/fox/daq/8.1test
proc Where {} {
variable root
return $root
}
}
