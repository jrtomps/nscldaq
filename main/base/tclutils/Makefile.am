
DRIVERDest=@prefix@/TclLibs/tclutils

TCLDRIVERS= Utils.tcl BlockCompleter.tcl TclSourceFilter.tcl Process.tcl
TCLTESTS= utils.test blockcompleter.test tclsourcefilter.test

install-exec-local:
	$(mkinstalldirs) @prefix@/TclLibs
	$(mkinstalldirs) $(DRIVERDest)
	for f in $(TCLDRIVERS); do $(INSTALL_SCRIPT) @srcdir@/$$f  $(DRIVERDest) ; done
	echo "pkg_mkIndex -verbose $(DRIVERDest) *.tcl" | $(TCLSH_CMD)


<<<<<<< HEAD
=======
DOCS=BlockCompleter.xml Process.xml TclSourceFilter.xml Utils.xml
>>>>>>> master

check-TESTS:
	TCLLIBPATH="@prefix@/TclLibs @libdir@ @srcdir@" HERE=@srcdir@ tcl=@TCLSH_CMD@ @TCLSH_CMD@ @srcdir@/tclTests.tcl

<<<<<<< HEAD
EXTRA_DIST=$(TCLDRIVERS) tclTests.tcl $(TCLTESTS)
=======
EXTRA_DIST=$(TCLDRIVERS) tclTests.tcl $(TCLTESTS) $(DOCS)
>>>>>>> master

