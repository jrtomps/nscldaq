bin_PROGRAMS = bufdump
bufdump_SOURCES = bufdump.cc 
INCLUDES=-I@top_srcdir@/Headers -I@top_srcdir@/CopyrightTools \
	$(SPECTRODAQ_FLAGS)
bufdump_LDADD=-L@top_srcdir@/CopyrightTools -lLicense  \
	$(SPECTRODAQ_LDFLAGS) 
bufdump_LDFLAGS=-Wl,"-rpath=$(libdir)"

EXTRA_DIST = bufdump.1

man_MANS = bufdump.1
