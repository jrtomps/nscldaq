source epicsEnumeratedControl.tcl

# NSCL aio test channel is ABTEST

::controlwidget::epicsEnumeratedControl .tgt -channel ABTEST       \
    -rows 3 -values [list 0 1 2 3 4 5 6]
pack .tgt
