DRIVERDIR=@prefix@/TclLibs/caenv812
APPDIR=@prefix@/bin
HELPDIR=@prefix@/TclLibs/data/caenv812


PACKAGES=CFD812.tcl CFDState.tcl caenv812Gui.tcl

APPS=caenv812control.tcl loadcfd.tcl

HELPFILES=close.html exit.html filemenu.html gui.html helpmenu.html \
		new.html read.html write.html config.html overview.html

install-exec-local:
	$(mkinstalldirs)  $(DRIVERDIR) $(APPDIR) $(HELPDIR)
	chmod o+rx $(DRIVERDIR) $(APPDIR) $(HELPDIR)
	for f in $(PACKAGES); do $(INSTALL_SCRIPT) @srcdir@/$$f $(DRIVERDIR); done
	for f in $(APPS); do $(INSTALL_SCRIPT) @srcdir@/$$f $(APPDIR)/`basename $$f .tcl`; done
	for f in $(APPS); do $(INSTALL_SCRIPT) @srcdir@/$$f $(APPDIR); done
	for f in $(HELPFILES); do $(INSTALL_DATA)   @srcdir@/$$f $(HELPDIR); done
	echo "pkg_mkIndex $(DRIVERDIR) *.tcl" | $(TCLSH_CMD)


EXTRA_DIST= $(PACKAGES) $(APPS)  $(HELPFILES) caenv812.xml

