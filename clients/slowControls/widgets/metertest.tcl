set metervar      0.5
set metervar2     -0.5
set jiggleCount   0
set jiggleMax     5
set jiggleAmount  0.05

source meter.tcl
namespace import controlwidget::*
meter .meter -variable metervar
pack .meter

proc jiggle ms {
    global metervar 
    global metervar2
    global jiggleCount
    global jiggleMax
    global jiggleAmount

    after $ms [list jiggle $ms]

    set metervar [expr $metervar + $jiggleAmount]
    set metervar2 [expr $metervar2 + $jiggleAmount]
    incr jiggleCount
    if {$jiggleCount > $jiggleMax} {
	set jiggleAmount [expr -$jiggleAmount]
	set jiggleCount 0
    }
    
}

jiggle 10

after 5000 [list .meter configure -from -2.0]
after 10000 [list .meter configure -to 2.0]
after 15000 [list .meter configure -variable metervar2]
after 16000 [list .meter configure -majorticks 2.0]
after 17000 [list .meter configure -minorticks 1]
after 18000 [list .meter configure -variable [list]]
after 19000 [list .meter set 0.0]
after 19500 {list puts "value: [.meter get]"}
