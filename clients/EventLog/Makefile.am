bin_PROGRAMS = eventlog
eventlog_SOURCES = eventlog.cpp
INCLUDES=-I@top_srcdir@/Headers -I@top_srcdir@/CopyrightTools \
	$(SPECTRODAQ_FLAGS) -I@top_srcdir@/Ftp 

eventlog_LDADD=-L@top_srcdir@/CopyrightTools -L@top_srcdir@/Ftp -lftp \
	-lLicense $(SPECTRODAQ_LDFLAGS)  
eventlog_LDFLAGS=-Wl,"-rpath=$(libdir)"

man_MANS = eventlog.1

EXTRA_DIST = eventlog.1

