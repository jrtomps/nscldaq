bin_PROGRAMS = bufdump
bufdump_SOURCES = bufdump.cc 
INCLUDES=-I@top_srcdir@/Headers	\
	-I@top_srcdir@/SpectrodaqLite	\
	 -I@top_srcdir@/CopyrightTools \
	$(SPECTRODAQ_FLAGS)
bufdump_LDADD= @top_srcdir@/CopyrightTools/libLicense.la  \
	@top_srcdir@/SpectrodaqLite/libSpectrodaqCompatibility.la \
	$(SPECTRODAQ_LDFLAGS) 

bufdump_LDFLAGS=-Wl,"-rpath=$(libdir)"

EXTRA_DIST = bufdump.1

man_MANS = bufdump.1
