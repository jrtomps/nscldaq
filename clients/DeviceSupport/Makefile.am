lib_LTLIBRARIES = libDeviceSupport.la
libDeviceSupport_la_SOURCES = VmeModule.cpp MmapError.cpp \
	CModule32.cpp CSIS3820.cpp sis3300.cpp CAENcard.cpp

include_HEADERS = VmeModule.h CVME.h CVMEptr.h MmapError.h CSIS3820.h \
	sis3300.h CModule32.h CAENcard.h

libDeviceSupport_la_LDFLAGS = -version-info 1:0:0 -L@top_srcdir@/VMEApi \
	-lVmeAPI $(SPECTRODAQ_LDFLAGS) -g 

INCLUDES = $(VMELIBRARYHEADERS) \
	-I@top_srcdir@/VMEApi -DINSTDIR=\"$(prefix)\" $(CAMAC) -g  \
	$(SPECTRODAQ_FLAGS) 


CXXFLAGS=-g
CCFLAGS=-g