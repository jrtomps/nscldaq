DRIVERDIR=$(prefix)/Scripts/ControlDrivers
APPDIR=$(prefix)/Scripts/ControlApplications
HELPDIR=$(prefix)/Scripts/ControlApplications/caenv812help

SUBDIRS=caenv812 n568b vhq

#  After installation of the subdirs, I need to
#  generate the package indices.

install-exec-local:
	echo pkg_mkIndex $(DRIVERDIR) >pkgscript.tcl
	tclsh <pkgscript.tcl
