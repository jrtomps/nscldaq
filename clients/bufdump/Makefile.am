bin_PROGRAMS = bufdump
bufdump_SOURCES = bufdump.cc 
INCLUDES=-I@top_srcdir@/Headers -I@top_srcdir@/CopyrightTools \
	$(SPECTRODAQ_FLAGS)
bufdump_LDADD=-L@top_srcdir@/CopyrightTools -lLicense  \
	$(SPECTRODAQ_LDFLAGS)
