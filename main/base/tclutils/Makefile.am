
DRIVERDest=@prefix@/TclLibs/tclutils

TCLDRIVERS= Utils.tcl BlockCompleter.tcl TclSourceFilter.tcl Process.tcl
TCLTESTS= utils.test blockcompleter.test tclsourcefilter.test

install-exec-local:
	$(mkinstalldirs) @prefix@/TclLibs
	$(mkinstalldirs) $(DRIVERDest)
	for f in $(TCLDRIVERS); do $(INSTALL_SCRIPT) @srcdir@/$$f  $(DRIVERDest) ; done
	echo "pkg_mkIndex -verbose $(DRIVERDest) *.tcl" | $(TCLSH_CMD)


DOCS=BlockCompleter.xml Process.xml TCLSourceFilter.xml Utils.xml

check-TESTS:
	TCLLIBPATH="@prefix@/TclLibs @libdir@ @srcdir@" HERE=@srcdir@ tcl=@TCLSH_CMD@ @TCLSH_CMD@ @srcdir@/tclTests.tcl

EXTRA_DIST=$(TCLDRIVERS) tclTests.tcl $(TCLTESTS) $(DOCS)

