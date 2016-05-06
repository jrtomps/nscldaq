#
#  Provies a filter that accepts everything.  This removes
#  some special case code by always requiring that the
#  data pass through _some_ sort of filter.
#

package require snit
snit::type AcceptAll {
    method applyFilter {itemType body} {
        return 1
    }
}