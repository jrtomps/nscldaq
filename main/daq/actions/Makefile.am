install-exec-local:  testCompile.o
	$(mkinstalldirs) $(prefix)/bin
	$(mkinstalldirs) $(prefix)/TclLibs/Stager
	$(mkinstalldirs) @prefix@/share/html
	for f in  $(TCLPACKAGES); do install @srcdir@/$$f $(prefix)/TclLibs/Stager; done
	echo "pkg_mkIndex -verbose $(prefix)/TclLibs/Stager *.tcl" | $(TCLSH_CMD)
	for f in  $(CXXPACKAGES); do install @srcdir@/$$f $(prefix)/include; done


#  This is here just to let us shake out the compilation errors in Actions.h before
#  committing to installing it.  testCompile.cpp just includes Actions.h

testCompile.o: testCompile.cpp
	$(CXX) $(CXXFLAGS) -c -I@srcdir@ @srcdir@/testCompile.cpp

TCLPACKAGES=actions.tcl defaultActions.tcl

TCLTESTS =    actions.test tclTests.tcl

CXXPACKAGES = Actions.h

check-TESTS:
	TCLLIBPATH=@prefix@/TclLibs HERE=@srcdir@ tcl=@TCLSH_CMD@ @TCLSH_CMD@ @srcdir@/tclTests.tcl

EXTRA_DIST = $(TCLPACKAGES) $(TCLTESTS) $(CXXPACKAGES) testCompile.cpp
