lib_LTLIBRARIES	= libDeviceSupport.la

libDeviceSupport_la_SOURCES	= CCAMACModule.cpp	\
				CVMEModule.cpp		\
				CCAMACReadableObject.cpp 

include_HEADERS     = CReadableObject.h	\
			CCAMACModule.h	\
			CVMEModule.h 	\
			CCAMACReadableObject.h

AM_CXXFLAGS	= -I@top_builddir@/vme    \
		-I@top_builddir@/camac

libDeviceSupport_la_LIBADD = @top_builddir@/camac/libCamac.la 	\
			     @top_builddir@/vme/libVme.la	\
			     @top_builddir@/exceptions/libException.la

ivp:
