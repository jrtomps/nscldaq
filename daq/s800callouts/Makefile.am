install-exec-local:
	$(mkinstalldirs) $(prefix)/TclLibs/s800
	$(INSTALL_SCRIPT) s800rdocallouts.tcl $(prefix)/TclLibs/s800
	$(INSTALL_SCRIPT) s800rctl.tcl        $(prefix)/TclLibs/s800
	$(INSTALL_SCRIPT) dummyrdo            $(bindir)
	echo pkg_mkIndex $(prefix)/TclLibs/s800 "*.tcl" | tclsh


EXTRA_DIST=s800rdocallouts.tcl s800rctl.tcl dummyrdo

