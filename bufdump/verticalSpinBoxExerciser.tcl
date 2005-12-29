#
#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2005.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#     Author:
#             Ron Fox
#            NSCL
#            Michigan State University
#            East Lansing, MI 48824-1321
#


# Exercise/test program for the vertical spinbox item.

set auto_path [concat . $auto_path]
package require bufdumpDialogs

frame .spinbox -relief groove -borderwidth 4
verticalSpinBox .spinbox.sb   -values {0 1 x}

pack .spinbox.sb
button .get -text Get -command [list get .spinbox.sb]

pack .spinbox .get

proc get {widget} {
    puts "Spinbox: [$widget get]"
}
