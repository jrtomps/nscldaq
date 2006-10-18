PKGDIR=$(prefix)/TclLibs/controlWidgets

PACKAGES = meter.tcl bcmMeter.tcl epicsLabel.tcl \
	radioMatrix.tcl epicsEnumeratedControl.tcl \
	typeNGo.tcl epicsTypeNGo.tcl

XMLFILES = meter.xml bcmMeter.xml epicsLabel.xml

MANPAGES = meter.1tcl bcmMeter.1tcl epicsLabel.1tcl

# Build rule for manpages:

.xml.1tcl:
	- $(MANDOCBOOK) man $< 
	touch $@


install-exec-local: $(MANPAGES)
	$(mkinstalldirs) $(PKGDIR) 
	chmod o+rx $(PKGDIR)
	$(INSTALL_SCRIPT) $(PACKAGES) $(PKGDIR)
	echo "pkg_mkIndex $(PKGDIR) *.tcl" >makeindex.tcl
	$(TCLSH_CMD) <makeindex.tcl

man_MANS = $(MANPAGES)

EXTRA_DIST= $(PACKAGES) $(XMLFILES)