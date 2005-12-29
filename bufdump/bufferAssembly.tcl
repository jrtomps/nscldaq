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

package provide bufferAssembly 1.0

namespace eval ::bufferAssembly {
    namespace export nextChunk
}

#------------------------------------------------------------------------------
#  bufferAssembly::nextChunk input size chunk
#        Assembles a chunk into an input buffer.
# Parameters:
#    input   - The current input buffer.
#    size    - The size of the desired filled buffer.
#    chunk   - The chunk to put in the buffer.
# Returns:
#    A 1 or 2 element list:
#    1 element:   The chunk did not fill the buffer.  The element is the
#                 the chunk appended to the buffer.
#    2 element:   First element is a full buffer, the second element is
#                 the (possibly empty) residue from chunk.
#
proc ::bufferAssembly::nextChunk {input size chunk} {
    set chunksize     [llength $chunk]
    set inputsize     [llength $input]
    set residual      [expr {($inputsize + $chunksize) - $size}]

    # Adding chunk won't fill.

    if {$residual < 0} {
        return [list [concat $input $chunk]]
    } else {

        # Adding chunk fills:

        set copyCount [expr {$chunksize - $residual}]
        set residual [lrange $chunk $copyCount end]
        incr copyCount -1
        set input [concat $input [lrange $chunk 0 $copyCount]]

        return [list $input $residual]

    }
}
