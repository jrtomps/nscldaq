bin_PROGRAMS = DiskFile
DiskFile_SOURCES = DiskFile.cpp
INCLUDES=-I@top_srcdir@/Headers -I@top_srcdir@/CopyrightTools \
	$(SPECTRODAQ_FLAGS)
DiskFile_LDADD=-L@top_srcdir@/CopyrightTools -lLicense \
	$(SPECTRODAQ_LDFLAGS) 

DiskFile_LDFLAGS= -Wl,"-rpath=$(libdir)"
