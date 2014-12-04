
package provide mcfd16factory  1.0

package require snit
package require mcfd16usb
package require mcfd16rc
package require mcfd16commandlogger
package require mcfd16memorizer
package require mcfd16guiapp

snit::type MCFD16Factory {

  variable _appOpts

  constructor {opts} {
    set _appOpts $opts
  }

  method create {proto {extra {}}} {

    set instance ""
    switch $proto {
      usb { set instance [$self createUSB] }
      mxdcrcbus { set instance [$self createMxDCRCBus] }
      cmdlogger { set instance [$self createCmdLogger $extra] }
      memorizer { set instance [$self createMemorizer] }
      default {
        return -code error "MCFD16Factory::create passed unknown type."
      }
    }

    return $instance
  }

  method createUSB {} {
    return [MCFD16USB %AUTO% [$_appOpts cget -serialfile]]
  }

  method createMxDCRCBus {} {
    set prxy [MXDCRCProxy %AUTO% -server [$_appOpts cget -host] \
                     -port [$_appOpts cget -port] \
                     -module [$_appOpts cget -module] \
                     -devno [$_appOpts cget -devno]]
    return [MCFD16RC %AUTO% $prxy]
  }

  method createCmdLogger {channel} {
    return [MCFD16CommandLogger %AUTO% $channel]
  }

  method createMemorizer {} {
    return [MCFD16Memorizer %AUTO%]
  }
}
