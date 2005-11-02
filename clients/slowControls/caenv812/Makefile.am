DRIVERDIR=$(prefix)/Scripts/ControlDrivers
APPDIR=$(prefix)/Scripts/ControlApplications
HELPDIR=$(prefix)/Scripts/ControlApplications/caenv812help

VERSION=2.0			#Must match the package versions!!!

PACKAGES=CFD812.tcl CFDState.tcl caenv812Gui.tcl pkgIndex.tcl
APPS=caenv812control.tcl loadcfd.tcl
MANPAGES=loadcfd.1 caenv812control.1 CFDState.3 CFD812.3 caenv812gui.3 \
	caenv812control.1 caen812configfile.5
HELPFILES=close.html exit.html filemenu.html gui.html helpmenu.html \
		new.html read.html write.html config.html overview.html

install-exec-local:
	$(mkinstalldirs)  $(DRIVERDIR) $(APPDIR) $(HELPDIR)
	chmod o+rx $(DRIVERDIR) $(APPDIR) $(HELPDIR)
	$(INSTALL_SCRIPT) $(PACKAGES) $(DRIVERDIR)
	$(INSTALL_SCRIPT) $(APPS)     $(APPDIR)
	$(INSTALL_DATA)   $(HELPFILES) $(HELPDIR)


EXTRA_DIST= $(PACKAGES) $(APPS) $(MANPAGES) $(HELPFILES)

man_MANS = $(MANPAGES)
