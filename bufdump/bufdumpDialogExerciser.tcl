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

dataSourceDialog .d -command [list ok .d] -cancelcommand [list cancelled .d]

proc ok {widget} {
    set online [.d cget -online]
    if {$online} {
        set host [.d cget -host]
        puts "online connection to $host"
    } else {
        set path [.d cget -path]
        puts "offline connection to $path"
    }
    set size [.d cget -buffersize]
    puts "Buffersize is: $size"
}
proc cancelled {widget} {
    puts "cancelled"
}
