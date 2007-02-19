TESTABLEUNITS=CSink.cpp CFileSink.cpp CFileSinkCreator.cpp CSinkFactory.cpp \
	CMonitoredProgram.cpp CLocalMonitoredProgram.cpp
bin_PROGRAMS 	= daqstart
daqstart_SOURCES= cmdline.c daqstart.cpp CDAQStart.cpp $(TESTABLEUNITS)

noinst_HEADERS = cmdline.h CDAQStart.h  CFileSinkCreator.h \
		CMonitoredProgram.h  CSinkCreator.h \
		CFileSink.h  CLocalMonitoredProgram.h  CSink.h \
		CSinkFactory.h

INCLUDES	= -I@top_srcdir@/Headers -I@top_srcdir@/Exception -I.

daqstart_LDADD = -L@top_srcdir@/Exception -lException

AM_CXXFLAGS	= -D_GNU_SOURCE


cmdline.c: daqstart.ggo
	gengetopt --unamed-opts <daqstart.ggo

EXTRA_DIST=daqstart.ggo popup.tcl daqstart.1 Makefile.am

man_MANS = daqstart.1

install-exec-local:
	$(INSTALL_SCRIPT) popup.tcl $(bindir)/PopUp


if CPPUNIT
noinst_PROGRAMS = unittests

unittests_SOURCES = $(TESTABLEUNITS) CFileSinkTests.cpp SinkCreatorTests.cpp \
	FactoryTests.cpp LocalMonitoredProgram.cpp  grep.cpp TestRunner.cpp

unittests_LDADD = $(CPPUNIT_LDFLAGS) -L@top_srcdir@/Exception -lException -lpub


TESTS = ./unittests

endif