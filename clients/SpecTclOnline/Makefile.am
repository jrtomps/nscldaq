bin_PROGRAMS = spectcldaq

spectcldaq_SOURCES = spectcldaq.cpp

INCLUDES=-I@top_srcdir@/Headers -I@top_srcdir@/CopyrightTools \
	$(SPECTRODAQ_FLAGS)

spectcldaq_LDADD=-L@top_srcdir@/CopyrightTools -lLicense $(SPECTRODAQ_LDFLAGS)
spectcldaq_LDFLAGS= -Wl,"-rpath=$(libdir)"