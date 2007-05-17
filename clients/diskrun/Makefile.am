bin_PROGRAMS = DiskFile
DiskFile_SOURCES = DiskFile.cpp
INCLUDES=-I@top_srcdir@/Headers \
	-I@top_srcdir@/SpectrodaqLite \
	-I@top_srcdir@/CopyrightTools \
	$(SPECTRODAQ_FLAGS)

DiskFile_LDADD= @top_srcdir@/CopyrightTools/libLicense.la 	\
	@top_srcdir@/SpectrodaqLite/libSpectrodaqCompatibility.la \
	$(SPECTRODAQ_LDFLAGS) 

DiskFile_LDFLAGS= -Wl,"-rpath=$(libdir)"
