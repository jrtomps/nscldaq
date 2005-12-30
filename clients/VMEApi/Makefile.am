lib_LTLIBRARIES = libVmeAPI.la
libVmeAPI_la_SOURCES = @VMEDEVICE@API.cpp  @SEPERABLELOCK@.cpp

# Locking.cpp


libVmeAPI_la_LDFLAGS = -version-info $(SOVERSION):0 $(VMEBUILDLIBS) \
	-Wl"-rpath=$(libdir)"
include_HEADERS = CVMEInterface.h SBSBit3API.h WienerAPI.h \
	WienerUSBVMEInterface.h

INCLUDES = -I@top_srcdir@/Headers -I@top_srcdir@/Exception $(VMEBUILDHEADERS)

EXTRA_DIST = NSCLBIT3API.cpp NULLAPI.cpp SBSBIT3API.cpp WIENERAPI.cpp \
	WIENERUSBAPI.cpp Locking.cpp  NullLock.cpp

bin_SCRIPTS = VmeDevice

LDFLAGS = $(VMEBUILDLIBS) 


VmeDevice: 
	echo #!/bin/sh		      > VmeDevice
	echo "echo $(VMEDEVICE)"     >> VmeDevice


