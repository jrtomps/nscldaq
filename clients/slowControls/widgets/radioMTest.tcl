source radioMatrix.tcl


set atest 5

proc Update {} {
    global atest
    set atest [.rm Get]
}

controlwidget::radioMatrix .rm -rows 2  -variable atest -command Update  \
    -values [list {red 1} {green 2} {blue 3} {purple 4} {chartruse 5} {black 6} {white 7}]

pack .rm
