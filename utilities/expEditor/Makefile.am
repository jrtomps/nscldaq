TCLPACKAGES=properties.tcl propertyEditor.tcl

install-exec-local:
	$(mkinstalldirs) @prefix@/TclLibs
	$(mkinstalldirs) @prefix@/TclLibs/expeditor
	for f in $(TCLPACKAGES) ; do $(INSTALL_SCRIPT) @srcdir@/$$f @prefix@/TclLibs/expeditor; done
	echo pkg_mkIndex -verbose @prefix@/TclLibs/expeditor "*.tcl" | @TCLSH_CMD@


TCLTESTS=properties.test proplist.test

check-TESTS:
	for f in $(TCLTESTS); do @TCLSH_CMD@ $$f; done


EXTRA_DIST = $(TCLPACKAGES) $(TCLTESTS)