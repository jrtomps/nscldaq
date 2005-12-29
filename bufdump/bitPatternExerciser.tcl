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

set auto_path [concat . $auto_path]

package require bufdumpDialogs

frame .f -relief groove -borderwidth 4
bitPattern .f.b -bits 32
pack .f.b
button .b -text Get -command [list get .f.b]

pack .f .b

proc get widget {
    puts "Pattern: [$widget get]"
}
