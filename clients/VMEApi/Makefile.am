lib_LTLIBRARIES = libVmeApi.la
libVmeApi_la_SOURCES = $(VMEDEVICE)API.cpp
libVmeApi_la_LDFLAGS = -version-info 1:0:0
include_HEADERS = CVMEInterface.h

INCLUDES = -I@top_srcdir@/Headers -I@top_srcdir@/Exception $(VMELIBRARYHEADERS)


bin_SCRIPTS = VmeDevice

LDFLAGS = $(VMELIBRARY) 


VmeDevice: 
	echo #!/bin/sh		      > VmeDevice
	echo "echo $(VMEDEVICE)"     >> VmeDevice



