proc onAlarm {who what which value} {
    puts "Alarm: $what for channel $which new state $value"
}
proc Log {who what which value} {
    puts "Log: $what for channel $which -> $value"
}

set auto_path [concat . $auto_path]
source vhqPanel.tcl

vhqPanel .a -crate 0 -base 0xdd00 -alarmscript onAlarm -model 202L-s -name PPAC-anode -ilimit {80 80} \
    -rampspeed {100 100} -command Log

pack .a

puts [.a getProperties]
