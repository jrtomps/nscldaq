package require Tk

source snitwiz.tcl
source enterNode.tcl
source readoutEntry.tcl
source nodeTimingEntry.tcl
source newNodeWizard.tcl


package require NodeWizard

set wiz [newNodeWizard %AUTO%]

set result [$wiz start]

if {$result eq "Finished"} {
    foreach option [list -node -id -path -args -istrigger -matchwindow -matchoffset] {
	puts "$option  : [$wiz cget $option]"
    }
} else {
    puts "Cancelled or deleted by user."
}

$wiz destroy

exit

