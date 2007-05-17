bin_PROGRAMS = eventlog
eventlog_SOURCES = eventlog.cpp
INCLUDES=-I@top_srcdir@/Headers -I@top_srcdir@/CopyrightTools \
	-I@top_srcdir@/SpectrodaqLite				\
	$(SPECTRODAQ_FLAGS) -I@top_srcdir@/Ftp 

eventlog_LDADD=@top_srcdir@/CopyrightTools/libLicense.la \
		@top_srcdir@/Ftp/libftp.la \
		@top_srcdir@/SpectrodaqLite/libSpectrodaqCompatibility.la \
		$(SPECTRODAQ_LDFLAGS)  

eventlog_LDFLAGS=-Wl,"-rpath=$(libdir)"

man_MANS = eventlog.1

EXTRA_DIST = eventlog.1