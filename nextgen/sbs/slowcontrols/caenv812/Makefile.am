DRIVERDIR=$(prefix)/TclLibs/caenv812
APPDIR=$(prefix)/bin
HELPDIR=$(prefix)/TclLibs/data/caenv812

VERSION=2.0			#Must match the package versions!!!

PACKAGES=CFD812.tcl CFDState.tcl caenv812Gui.tcl

APPS=caenv812control.tcl loadcfd.tcl

MANPAGES=loadcfd.1 caenv812control.1 CFDState.3 CFD812.3 caenv812gui.3 \
	caenv812control.1 caen812configfile.5

HELPFILES=close.html exit.html filemenu.html gui.html helpmenu.html \
		new.html read.html write.html config.html overview.html

install-exec-local:
	$(mkinstalldirs)  $(DRIVERDIR) $(APPDIR) $(HELPDIR)
	chmod o+rx $(DRIVERDIR) $(APPDIR) $(HELPDIR)
	$(INSTALL_SCRIPT) $(PACKAGES) $(DRIVERDIR)
	for f in $(APPS); do $(INSTALL_SCRIPT) $$f $(APPDIR)/`basename $$f .tcl`; done
	$(INSTALL_SCRIPT) $(APPS)     $(APPDIR)
	$(INSTALL_DATA)   $(HELPFILES) $(HELPDIR)
	echo "pkg_mkIndex $(DRIVERDIR) *.tcl" > makeindex.tcl
	$(TCLSH_CMD) <makeindex.tcl

EXTRA_DIST= $(PACKAGES) $(APPS) $(MANPAGES) $(HELPFILES)

man_MANS = $(MANPAGES)
