
package provide scriptheadergenerator 1.0

package require snit


snit::type ScriptHeaderGenerator {

  variable _options

  constructor {options} {
    set _options $options 
  }

  method generateHeader {} {
    set protocol [$_options cget -protocol]

    set header [list]
    if {$protocol eq "usb"} {
      set header [$self generateUSBHeader]
    } else {
      set header [$self generateMxDCRCBusHeader]
    }

    return $header
  }

  method generateUSBHeader {} {
    set header [list]
    lappend header {package require mcfd16usb}
    lappend header [list set serialFile [$_options cget -serialfile]]
    lappend header {}
    lappend header "if \{!\[file exists \$serialFile\]\} \{"
    lappend header {  puts "Serial file \"$serialFile\" provided but does not exist."}
    lappend header {  exit}
    lappend header "\}"
    lappend header {MCFD16USB ::dev $serialFile}
    return $header
  }

  method generateMxDCRCBusHeader {} {
    set header [list]

    lappend header {package require mcfd16rc}
    lappend header [list MXDCRCProxy ::proxy -server [$_options cget -host] \
      -port [$_options cget -port] \
      -module [$_options cget -module] \
      -devno [$_options cget -devno]] 
    lappend header {# use the proxy created to construct an MCFD16RC} 
    lappend header {MCFD16RC dev ::proxy} 

    return $header
  }
}
