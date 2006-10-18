source radioMatrix.tcl


set atest 5

proc Update {} {
    global atest
    set atest [.rm Get]
}

controlwidget::radioMatrix .rm -rows 2 -columns 3 -variable atest -command Update

pack .rm
