lib_LTLIBRARIES = libVmeAPI.la
libVmeAPI_la_SOURCES = $(VMEDEVICE)API.cpp
libVmeAPI_la_LDFLAGS = -version-info 1:0:0
include_HEADERS = CVMEInterface.h SBSBit3API.h

INCLUDES = -I@top_srcdir@/Headers -I@top_srcdir@/Exception $(VMELIBRARYHEADERS)


bin_SCRIPTS = VmeDevice

LDFLAGS = $(VMELIBRARY) 


VmeDevice: 
	echo #!/bin/sh		      > VmeDevice
	echo "echo $(VMEDEVICE)"     >> VmeDevice



