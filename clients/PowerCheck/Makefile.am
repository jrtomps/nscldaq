bin_PROGRAMS = powerdetect

powerdetect_SOURCES = powerdetect.cpp

INCLUDES = -I@top_srcdir@/Headers $(VMEBUILDHEADERS) \
	-I@top_srcdir@/Exception -I@top_srcdir@/VMEApi

EXTRA_DIST = cratemonitor.tcl Makefile.am

LDFLAGS = -L@top_srcdir@/VMEApi -lVmeAPI \
	-L@top_srcdir@/Exception -lException $(VMELIBRARY)

install-exec-local:
	$(mkinstalldirs) $(prefix)/bin
	$(INSTALL_SCRIPT) cratemonitor.tcl $(prefix)/bin/cratemonitor