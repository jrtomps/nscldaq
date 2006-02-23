DRIVERDIR=$(prefix)/TclLibs/v568
APPDIR=$(prefix)/bin
HELPDIR=$(prefix)/TclLibs/data/n568

VERSION=2.0			#Must match the package versions!!!

PACKAGES=n568b.tcl n568Panel.tcl titledSpinBox.tcl
APPS=loadshaper.tcl n568Control.tcl
MANPAGES=n568b.3 n568panel.3 loadshaper.1 n568control.1 n568configfile.5

HELPFILES=gui.html overview.html close.html config.html exit.html filemenu.html \
    helpmenu.html new.html read.html write.html

install-exec-local:
	$(mkinstalldirs)  $(DRIVERDIR) $(APPDIR) $(HELPDIR)
	chmod o+rx $(DRIVERDIR) $(APPDIR) $(HELPDIR)
	$(INSTALL_SCRIPT) $(PACKAGES) $(DRIVERDIR)
	for f in $(APPS); do $(INSTALL_SCRIPT) $$f $(APPDIR)/`basename $$f .tcl`; done
	$(INSTALL_DATA)   $(HELPFILES) $(HELPDIR)
	echo "pkg_mkIndex $(DRIVERDIR) *.tcl" > makeindex.tcl
	$(TCLSH_CMD) <makeindex.tcl

EXTRA_DIST= $(PACKAGES) $(APPS) $(MANPAGES) $(HELPFILES)

man_MANS = $(MANPAGES)
