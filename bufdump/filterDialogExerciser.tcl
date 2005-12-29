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

filterSetupDialog .fd -buffertypes {begin pause end resume event scaler snapshot} \
                      -command ok -cancelcommand cancel -clearcommand clear


proc ok {} {
    puts ok:
    if {[.fd cget -filteronbuffers]} {
        puts "buffer filtering relevant."
        puts "Buffers selected [.fd cget -buffertypesrequired]"
    }
    puts "Bit patterns:  [.fd cget -patternlist]"
    puts "matchtype:     [.fd cget -patternmatchtype]"

}
proc cancel {} {
    puts "cancel"
}
proc clear {} {
    puts clear
}


button .b -command {puts hi} -text hi
pack .b

.fd modal
