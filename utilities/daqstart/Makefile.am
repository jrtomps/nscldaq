TESTABLEUNITS=CSink.cpp CFileSink.cpp CFileSinkCreator.cpp CSinkFactory.cpp \
	CMonitoredProgram.cpp CLocalMonitoredProgram.cpp
bin_PROGRAMS 	= daqstart
daqstart_SOURCES= cmdline.c daqstart.cpp CDAQStart.cpp $(TESTABLEUNITS)

noinst_HEADERS = cmdline.h CDAQStart.h  CFileSinkCreator.h \
		CMonitoredProgram.h  CSinkCreator.h \
		CFileSink.h  CLocalMonitoredProgram.h  CSink.h \
		CSinkFactory.h grep.h

INCLUDES	= -I@top_srcdir@/base/os \
	-I@top_srcdir@/base/headers -I@top_srcdir@/base/exception -I.

daqstart_LDADD = @top_builddir@/base/exception/libException.la	\
	@top_builddir@/base/os/libdaqshm.la  \
	$(THREADLD_FLAGS)

daqstart_CXXFLAGS=$(THREADCXX_FLAGS) -D_GNU_SOURCE $(AM_CXXFLAGS)



cmdline.c: daqstart.ggo
	$(GENGETOPT) --unamed-opts <daqstart.ggo

EXTRA_DIST=daqstart.ggo popup.tcl daqstart.xml Makefile.am


install-exec-local:
	$(INSTALL_SCRIPT) popup.tcl $(bindir)/PopUp


noinst_PROGRAMS = unittests

unittests_SOURCES = $(TESTABLEUNITS) CFileSinkTests.cpp SinkCreatorTests.cpp \
	FactoryTests.cpp LocalMonitoredProgram.cpp  grep.cpp TestRunner.cpp

unittests_LDADD = $(CPPUNIT_LDFLAGS) @top_builddir@/base/exception/libException.la \
	@top_builddir@/base/os/libdaqshm.la


TESTS = ./unittests

