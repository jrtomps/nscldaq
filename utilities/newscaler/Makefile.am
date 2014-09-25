# The pure TCl parts of this.

PACKAGE_FILES=nameMap.tcl channel.tcl scalerconfig.tcl page.tcl \
	singleModel.tcl ratioModel.tcl scalermain.tcl header.tcl reports.tcl\
	scalerUtil.tcl stripparam.tcl stripratio.tcl emptyModel.tcl
TEST_FILES=nameMap.test channel.test channelcmd.test singleModel.test \
	ratioModel.test stripparam.test stripratio.test emptyModel.test
APP_FILES=ScalerDisplay.tcl.in
IMAGES=red.gif green.gif amber.gif
DOCS=tclringcommand.xml scalerdisplay.xml

INTERACTIVE_TESTS=pagetest.tcl 




INSTDIR=@prefix@/TclLibs/newscaler
GRAPHDIR=@prefix@/share/scalerdisplay

install-exec-hook:
	$(mkinstalldirs) $(INSTDIR) 
	$(mkinstalldirs) $(GRAPHDIR)
	install @libdir@/libTclRingBuffer.so $(INSTDIR)
	(cd @srcdir@; $(INSTALL_SCRIPT) $(PACKAGE_FILES) $(INSTDIR))
	echo pkg_mkIndex -verbose $(INSTDIR) "*.tcl" | @TCLSH_CMD@
	echo "package ifneeded TclRingBuffer 1.0 [list load [file join \$$dir libTclRingBuffer.so]]" >> $(INSTDIR)/pkgIndex.tcl
	install @builddir@/ScalerDisplay.tcl @bindir@/ScalerDisplay
	install $(IMAGES) $(GRAPHDIR)


check-TESTS: check_PROGRAMS
	for f in  @srcdir@/*.test; do echo Testing $$f  ; @TCLSH_CMD@ @srcdir@/$$f; done


EXTRA_DIST=$(PACKAGE_FILES) $(TEST_FILES) $(INTERACTIVE_TESTS) $(APP_FILES) $(IMAGES) \
	$(DOCS)

#  The Tcl API to get data from rings...and tests.

lib_LTLIBRARIES=libTclRingBuffer.la

libTclRingBuffer_la_SOURCES=CTclRingCommand.cpp
libTclRingBuffer_la_LIBADD=	@top_builddir@/daq/format/libdataformat.la     \
				@top_builddir@/base/dataflow/libDataFlow.la    \
				@top_builddir@/base/tcpip/libTcp.la		\
				@top_builddir@/base/os/libdaqshm.la 		\
				@LIBTCLPLUS_LDFLAGS@  @TCL_LDFLAGS@ @THREADLD_FLAGS@


libTclRingBuffer_la_CPPFLAGS=@LIBTCLPLUS_CFLAGS@ @TCL_FLAGS@ @THREADCXX_FLAGS@ \
				-I@top_srcdir@/base/dataflow -I@top_srcdir@/daq/format

noinst_HEADERS=CTclRingCommand.h

noinst_PROGRAMS   = ringTests
ringTests_SOURCES = TestRunner.cpp ringTests.cpp
ringTests_CPPFLAGS = @LIBTCLPLUS_CFLAGS@ @CPPUNIT_CFLAGS@ @TCL_FLAGS@ @THREADCXX_FLAGS@ \
			-I@top_srcdir@/daq/format -I@top_srcdir@/base/dataflow
ringTests_LDADD    =  @CPPUNIT_LDFLAGS@ libTclRingBuffer.la \
			@top_builddir@/daq/format/libdataformat.la     \
				@top_builddir@/base/dataflow/libDataFlow.la    \
				@LIBTCLPLUS_LDFLAGS@  @TCL_LDFLAGS@ @THREADLD_FLAGS@


TESTS=ringTests
check_PROGRAMS:  
	@builddir@/ringTests
