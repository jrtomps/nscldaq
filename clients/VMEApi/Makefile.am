lib_LTLIBRARIES = libVmeAPI.la
libVmeAPI_la_SOURCES = $(VMEDEVICE)API.cpp Locking.cpp
libVmeAPI_la_LDFLAGS = -version-info $(SOVERSION):0 $(VMEBUILDLIBS) \
	-Wl"-rpath-$(libdir)"
include_HEADERS = CVMEInterface.h SBSBit3API.h

INCLUDES = -I@top_srcdir@/Headers -I@top_srcdir@/Exception $(VMEBUILDHEADERS)


bin_SCRIPTS = VmeDevice

LDFLAGS = $(VMEBUILDLIBS) 


VmeDevice: 
	echo #!/bin/sh		      > VmeDevice
	echo "echo $(VMEDEVICE)"     >> VmeDevice



