DRIVERDIR=@prefix@/TclLibs/vhq
APPDIR=@prefix@/bin
HELPDIR=@prefix@/TclLibs/data/vhq


PACKAGES=vhqPanel.tcl vhq202m.tcl
APPS=vhqControl.tcl

HELPFILES= configfile.html configilimit.html configmenu.html \
    configspeeds.html fileclose.html filemenu.html filenew.html \
    filequit.html fileread.html filewrite.html gui.html \
    helpmenu.html menubar.html overview.html

install-exec-local:
	$(mkinstalldirs)  $(DRIVERDIR) $(APPDIR) $(HELPDIR)
	chmod o+rx $(DRIVERDIR) $(APPDIR) $(HELPDIR)
	for f in $(PACKAGES); do $(INSTALL_SCRIPT) @srcdir@/$$f $(DRIVERDIR); done
	for f in $(APPS); do $(INSTALL_SCRIPT) @srcdir@/$$f $(APPDIR)/`basename $$f .tcl`; done
	for f in $(HELPFILES); do $(INSTALL_DATA)   @srcdir@/$$f $(HELPDIR); done
	echo "pkg_mkIndex $(DRIVERDIR) *.tcl" | $(TCLSH_CMD)



EXTRA_DIST= $(PACKAGES) $(APPS)  $(HELPFILES) vhq.xml

