install-exec-local:
	$(mkinstalldirs) $(prefix)/bin
	$(mkinstalldirs) $(prefix)/TclLibs/Stager
	$(mkinstalldirs) @prefix@/share/html
	for f in  $(TCLPACKAGES); do install @srcdir@/$$f $(prefix)/TclLibs/Stager; done
	echo "pkg_mkIndex -verbose $(prefix)/TclLibs/Stager *.tcl" | $(TCLSH_CMD)
	for f in  $(CXXPACKAGES); do install @srcdir@/$$f $(prefix)/include; done

TCLPACKAGES=actions.tcl defaultActions.tcl

TCLTESTS =    actions.test tclTests.tcl

CXXPACKAGES = Actions.h

check-TESTS:
	TCLLIBPATH=@prefix@/TclLibs HERE=@srcdir@ tcl=@TCLSH_CMD@ @TCLSH_CMD@ @srcdir@/tclTests.tcl

EXTRA_DIST = $(TCLPACKAGES) $(TCLTESTS) $(CXXPACKAGES)
