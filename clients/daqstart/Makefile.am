bin_PROGRAMS 	= daqstart
daqstart_SOURCES= cmdline.c CDAQStart.cpp  CFileSinkCreator.cpp \
	        CMonitoredProgram.cpp  CSinkFactory.cpp         \
		CFileSink.cpp  CLocalMonitoredProgram.cpp  CSink.cpp \
		daqstart.cpp

include_HEADERS = cmdline.h CDAQStart.h  CFileSinkCreator.h \
		CMonitoredProgram.h  CSinkCreator.h \
		CFileSink.h  CLocalMonitoredProgram.h  CSink.h \
		CSinkFactory.h

INCLUDES	= -I@top_srcdir@/Headers -I@top_srcdir@/Exception -I.

daqstart_LDADD = -L@top_srcdir@/Exception -lException -lpub

AM_CXXFLAGS	= -D_GNU_SOURCE


cmdline.c: daqstart.ggo
	gengetopt --unamed-opts <daqstart.ggo

EXTRA_DIST=daqstart.ggo popup.tcl daqstart.1 Makefile.am

man_MANS = daqstart.1

install-exec-local:
	$(mkinstalldirs) $(prefix)/Scripts
	$(INSTALL_SCRIPT) popup.tcl $(prefix)/Scripts
	rm -f $(bindir)/PopUp
	$(LN_S)  $(prefix)/Scripts/popup.tcl $(bindir)/PopUp

