package require Vme

vme create XLM -device /dev/vme32d32 [expr 6<<27] 0xa00000

proc read {addr} {
    XLM set -l 0x800000 0x10003
    set x [XLM get -l 0x800000]
    puts "bus ctrl reg returns 0x[format %x $x]"
    set x [XLM get -l $addr]
    puts "[format %x $x]"
    XLM set -l 0x800000 0x0
    set x [XLM get -l 0x800000]
    puts "bus ctrl reg returns 0x[format %x $x]"
}

proc write {addr value} {
    XLM set -l $addr $value
}