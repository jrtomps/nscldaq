
DRIVERDest=@prefix@/TclLibs/tclutils

TCLDRIVERS= Utils.tcl
           

install-exec-local:
	$(mkinstalldirs) @prefix@/TclLibs
	$(mkinstalldirs) $(DRIVERDest)
	for f in $(TCLDRIVERS); do $(INSTALL_SCRIPT) @srcdir@/$$f  $(DRIVERDest) ; done
	echo "pkg_mkIndex -verbose $(DRIVERDest) *.tcl" | $(TCLSH_CMD)



check-TESTS:
	TCLLIBPATH="@prefix@/TclLibs @libdir@ @srcdir@" HERE=@srcdir@ tcl=@TCLSH_CMD@ @TCLSH_CMD@ @srcdir@/tclTests.tcl

EXTRA_DIST=$(TCLDRIVERS) tclTests.tcl

