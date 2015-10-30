TCLPACKAGES=properties.tcl

install-exec-local:
	$(mkinstalldirs) @prefix@/TclLibs
	$(mkinstalldirs) @prefix@/TclLibs/expeditor
	$(INSTALL_SCRIPT) properties.tcl @prefix@/TclLibs/expeditor
	echo pkg_mkIndex @prefix@/TclLibs/expeditor "*.tcl" | @TCLSH_CMD@


TCLTESTS=properties.test proplist.test

check-TESTS:
	for f in $(TCLTESTS); do @TCLSH_CMD@ $$f; done


EXTRA_DIST = $(TCLPACKAGES) $(TCLTESTS)