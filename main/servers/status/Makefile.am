lib_LTLIBRARIES = libStatusMessages.la libTclStatusMessages.la

COMPILATION_FLAGS= -I@top_srcdir@/base/os
COMMON_LIBS      = @top_builddir@/base/os/libdaqshm.la  @ZMQ_LDFLAGS@ @LIBEXCEPTION_LDFLAGS@

libStatusMessages_la_SOURCES=Constants.cpp \
	CRingStatisticsMessage.cpp \
	CReadoutStatistics.cpp \
	CStatusDefinitions.cpp \
	LogMessage.cpp		\
	StateChange.cpp


libStatusMessages_la_CPPFLAGS = $(COMPILATION_FLAGS)
libStatusMessages_la_LDFLAGS  = $(COMMON_LIBS)

include_HEADERS = CStatusMessage.h

#--------------------Tcl Bindings:

libTclStatusMessages_la_SOURCES= TclPackage.cpp \
	CTCLRingStatistics.cpp CTCLRingStatistics.h

libTclStatusMessages_la_CPPFLAGS=$(COMPILATION_FLAGS) @TCL_FLAGS@ @LIBTCLPLUS_CFLAGS@
libTclStatusMessages_la_LDFLAGS=$(COMMON_LIBS)  	\
				@builddir@/libStatusMessages.la   \
				@LIBTCLPLUS_LDFLAGS@ 	\
				@TCL_LDFLAGS@

tclpkgInstall:
	$(mkinstalldirs) @prefix@/TclLibs @prefix@/TclLibs/status
	rm -f @prefix@/TclLibs/status/libTclStatusMessages.so
	$(LN_S) @libdir@/libTclStatusMessages.so @prefix@/TclLibs/status/libTclStatusMessages.so
	echo pkg_mkIndex -verbose "@prefix@/TclLibs/status *.so" |@TCLSH@


#-------------------- Tests -----


TEST_PROGS=ringstattests tclpkgtests

noinst_PROGRAMS=$(TEST_PROGS)


ringstattests_SOURCES=TestRunner.cpp Asserts.h ringstattests.cpp \
	readoutstattests.cpp logmsgtests.cpp statemsgtests.cpp

ringstattests_CPPFLAGS=$(COMPILATION_FLAGS) @CPPUNIT_CFLAGS@
ringstattests_LDFLAGS=libStatusMessages.la $(COMMON_LIBS) @CPPUNIT_LDFLAGS@

tclpkgtests_SOURCES = TestRunner.cpp Asserts.h
tclpkgtests_CPPFLAGS=$(COMPILATION_FLAGS) @CPPUNIT_CFLAGS@ @LIBTCLPLUS_CFLAGS@ @TCL_FLAGS@
tclpkgtests_LDFLAGS=$(COMMON_LIBS) @LIBTCLPLUS_LDFLAGS@ @TCL_LDFLAGS@ @CPPUNIT_LDFLAGS@

TESTS=$(TEST_PROGS)


install-exec-hook: tclpkgInstall
