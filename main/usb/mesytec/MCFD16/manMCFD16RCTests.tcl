
package require tcltest
package require Utils

tcltest::test pkgrequire-0 {Require the package
} {package require mcfd16rc} 1.0

proc setup {} {
  # use default localhost:27000
  MXDCRCProxy ::proxy -module rcbus -devno 0
  MCFD16RC ::dev ::proxy
}

proc tearDown {} {
  ::dev destroy
  ::proxy destroy
}

if {0} {
tcltest::test threshold-0 { Control over thresholds
} -setup {
  setup
} -cleanup {
  tearDown
} -body {

  set actual [list]

  ::dev SetMode individual
  for {set ch 0} {$ch < 16} {incr ch} {
    ::dev SetThreshold $ch $ch
    set val [::dev GetThreshold $ch]
    lappend actual $val
  }  

  ::dev SetMode common
  ::dev SetThreshold 16 16
  lappend actual [::dev GetThreshold 16]

  set actual
} -result [Utils::sequence 0 17 1]

tcltest::test gain-0 { Control over gain
} -setup {
  setup
} -cleanup {
  tearDown
} -body {

  set actual [list]

  ::dev SetMode individual
  for {set ch 0} {$ch < 8} {incr ch} {
    ::dev SetGain $ch [expr $ch%3]
    set val [::dev GetGain $ch]
    lappend actual $val
  }  

  ::dev SetMode common
  ::dev SetGain 8 2 
  lappend actual [::dev GetGain 8]

  set actual
} -result {0 1 2 0 1 2 0 1 2}

tcltest::test width-0 { Control over width 
} -setup {
  setup
} -cleanup {
  tearDown
} -body {

  set actual [list]

  ::dev SetMode individual
  for {set ch 0} {$ch < 8} {incr ch} {
    ::dev SetWidth $ch [expr 222+$ch*2]
    set val [::dev GetWidth $ch]
    lappend actual $val
  }  

  ::dev SetMode common
  ::dev SetWidth 8 255 
  lappend actual [::dev GetWidth 8]

  set actual
} -result {222 224 226 228 230 232 234 236 255}

tcltest::test deadtime-0 { Control over deadtime
} -setup {
  setup
} -cleanup {
  tearDown
} -body {

  set actual [list]

  ::dev SetMode individual
  for {set ch 0} {$ch < 8} {incr ch} {
    ::dev SetDeadtime $ch [expr 222+$ch*2]
    set val [::dev GetDeadtime $ch]
    lappend actual $val
  }  

  ::dev SetMode common
  ::dev SetDeadtime 8 255
  lappend actual [::dev GetDeadtime 8]

  set actual
} -result {222 224 226 228 230 232 234 236 255}

tcltest::test delay-0 { Control over delay
} -setup {
  setup
} -cleanup {
  tearDown
} -body {

  set actual [list]

  ::dev SetMode individual
  for {set ch 0} {$ch < 8} {incr ch} {
    ::dev SetDelay $ch [expr $ch%5]
    set val [::dev GetDelay $ch]
    lappend actual $val
  }  

  ::dev SetMode common
  ::dev SetDelay 8 4 
  lappend actual [::dev GetDelay 8]

  set actual
} -result {0 1 2 3 4 0 1 2 4}

tcltest::test fraction-0 { Control over fraction
} -setup {
  setup
} -cleanup {
  tearDown
} -body {

  set actual [list]

  ::dev SetMode individual
  for {set ch 0} {$ch < 8} {incr ch} {
    ::dev SetFraction $ch [lindex {20 40} [expr $ch%2]]
    set val [::dev GetFraction $ch]
    lappend actual $val
  }  

  ::dev SetMode common
  ::dev SetFraction 8 20 
  lappend actual [::dev GetFraction 8]

  set actual
} -result {20 40 20 40 20 40 20 40 20}


tcltest::test polarity-0 { Control over polarity
} -setup {
  setup
} -cleanup {
  tearDown
} -body {

  set actual [list]

  ::dev SetMode individual
  for {set ch 0} {$ch < 8} {incr ch} {
    ::dev SetPolarity $ch [lindex {pos neg} [expr $ch%2]]
    set val [::dev GetPolarity $ch]
    lappend actual $val
  }  

  ::dev SetMode common
  ::dev SetPolarity 8 neg
  lappend actual [::dev GetPolarity 8]

  set actual
} -result {pos neg pos neg pos neg pos neg neg}



tcltest::cleanupTests

}


setup

# for channel 0, try and set width values 0 to 255 and read back

for {set val 23} {$val < 212} {incr val} {
#  set ch [expr $val%8]
  set ch 0
  dev SetThreshold $ch $val
  set rdbk [dev GetThreshold $ch]

  if {$rdbk != $val} {
    puts "Wrote $val, Read $rdbk"
  }
  if {$val == 212} { puts "Completed all 255"}
}

