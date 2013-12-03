#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2005.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Author:
#             Ron Fox
#	     NSCL
#	     Michigan State University
#	     East Lansing, MI 48824-1321



# This file contains code that manages a status display of the
# spectrodaq page usage for the readout GUI.  We look like
# pair of megawidgets, but only the spdaqMonitor widget should be
# used as it has a button that will bring up the spdaqUsage widget.
#

package provide spdaqMonitor 1.0
package require snit
package require spdaqwidgets


#--------------------------------------------------------------------
#
#  This widget provides a detailed status widget in a toplevel
#  with some titling information, and a dimsiss box.
#  dismiss and destruction of the toplevel will invoke a -destroy
#  callback if defined.
#
snit::widget  detailsMonitor {
    hulltype toplevel

    variable afterId -1

    option -host    localhost
    option -destroy {}




    method update {} {
	$win.usage update
	set afterId [after 2000 [mymethod update]]
    }
    method onDismiss {} {
	destroy $win
    }
    method onDestroy widget {
	if {$widget eq $win} {
	    if {$options(-destroy) ne ""} {
		eval $options(-destroy)
	    }
	}
    }
    onconfigure -host host {
	set options(-host) $host
	$win.usage configure -host $host
    }
    constructor args {
	message $win.legend -width 512 \
	    -text {
The bar graphs to the right of node names are spectrodaq free
page graphs.  If they are yellow, the free page count is getting
a bit low.  If they are red or nonexistent, the free page count
is very low and you may have a problem.  

Below each node may appear a list of numbers and blue bargraphs.
The numbers are process ids of clients running in that node.
The bargraphs are graphs of the pages that are held by that
client.  If you have a node in the red and a client whose
bar graph is pegged, you probably have a problem with that client.}

	spdaqwidgets::bufferusage $win.usage -host $options(-host)
	button $win.destroy -text Dismiss -command [mymethod onDismiss]

	bind $win <Destroy>  [mymethod onDestroy %W]

	grid $win.legend
	grid $win.usage
	grid $win.destroy

	$self configurelist $args

	after 100 [mymethod  update]

    }

    destructor {
	if {$afterId > 0} {
	    after cancel $afterId
	}
    }
}


#--------------------------------------------------------------------
#
#  This widget provides a spectrodaq status strip. The strip
#  consists of some text that describes the source host
#  a freepage graph of that  host and a button that will
#  bring up the detailed status widget.  The widget
#  auto updates every 2 seconds or so.
#
snit::widget spdaqMonitor {
    variable afterId -1
    variable details false

    option   -host    localhost


    constructor args {
	label $win.host  -text $options(-host) 
	label $win.text  -text " spectrodaq free pages: "
	spdaqwidgets::freepagegraph $win.pages -host $options(-host)
	button $win.details -text {Details...} -command [mymethod showDetails]

	$self configurelist $args
	$self update

	grid $win.host $win.text $win.pages $win.details -padx 3
    }

    destructor {
	if {$afterId > 0} {
	    after cancel $afterId
	}
    }

    onconfigure -host host {
	$win.host          config -text $host
	$win.pages configure -host $host
	if {$details} {
	    .spdaqdetails configure -host $host
	}
	set options(-host) $host
    }
    method update {} {
	$win.pages update
	set afterId [after 2000 [mymethod update]]
    }
    method showDetails {} {
	if {$details} {
	    destroy .spdaqdetails
	}
	detailsMonitor .spdaqdetails -host $options(-host) -destroy [mymethod onDetailsGone]
	set details true
    }
    method onDetailsGone {} {
	set details false
    }

}
