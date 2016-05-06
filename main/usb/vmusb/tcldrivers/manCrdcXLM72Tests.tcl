
package require tcltest
package require cvmusb
package require crdcxlm72

proc acquireVMUSB {} {
  set modules [::cvmusb::CVMUSB_enumerate]
  set nmods [::cvmusb::usb_device_vector_size $modules]
  for {set index 0} {$index < $nmods} {incr index} {
    set module [::cvmusb::usb_device_vector_get $modules $index]
    set sn [cvmusb::string_to_char [::cvmusb::CVMUSB_serialNo $module]]
    set res [scan $sn {VM%d} number]
    if {$res==1} {
      return [::cvmusb::CVMUSBusb %AUTO% $module]
    }
  }
  error "Failed to find a VMUSB"
}

set ::ctlr  [acquireVMUSB]
set fw [file join /home tompkins s800conv crdc5v.bit]
set first 1
proc setup {} {
   ACrdcXLM72 ::dev $::ctlr 19 

   if {$::first} {
     puts -nonewline "Configuring..."
     flush stdout
     ::dev Configure $::fw 
     set ::first 0
     puts "Done"
   }

   ::dev AccessBus 0x10001
}

proc tearDown {} {
   ::dev ReleaseBus
   itcl::delete object ::dev
}


proc cleanupTestsHook {} {
  $::ctlr -delete
}


tcltest::test samples-0 {Test that we can configure the samples
} -setup {
  setup
} -cleanup {
  tearDown
} -body {
  ::dev WriteSamples 4
  ::dev ReadSamples
} -result 4


tcltest::test period-0 {Test that we can configure the period
} -setup {
  setup
} -cleanup {
  tearDown
} -body {
  ::dev WritePeriod 3
  ::dev ReadPeriod
} -result 3


tcltest::test delay-0 {Test that we can configure the delay
} -setup {
  setup
} -cleanup {
  tearDown
} -body {
  ::dev WriteDelay 4
  ::dev ReadDelay
} -result 4



tcltest::test width-0 {Test that we can configure the width 
} -setup {
  setup
} -cleanup {
  tearDown
} -body {
  ::dev WriteWidth 16
  ::dev ReadWidth
} -result 16



tcltest::test shift-0 {Test that we can configure the shift 
} -setup {
  setup
} -cleanup {
  tearDown
} -body {
  ::dev WriteShift 4
  ::dev ReadShift
} -result 4


tcltest::cleanupTests


proc w {type val} {
  return [::dev Write$type $val]
}
proc r {type} {
  return [::dev Read$type]
}
proc wr {type val} {
  w $type $val
  return [r $type]
}

