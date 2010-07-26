package provide InstallRoot 1.0
namespace eval InstallRoot {
variable root /usr/local
proc Where {} {
variable root
return $root
}
}
