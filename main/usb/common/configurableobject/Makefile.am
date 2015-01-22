lib_LTLIBRARIES 		= libConfigurableObject.la
libConfigurableObject_la_SOURCES	= 	CConfigurableObject.cpp
include_HEADERS	= 	CConfigurableObject.h	

libConfigurableObject_la_CXXFLAGS=@THREADCXX_FLAGS@ @TCL_FLAGS@ @LIBTCLPLUS_CFLAGS@		

libConfigurableObject_la_LIBADD	= @LIBTCLPLUS_LDFLAGS@	\
					@TCL_LDFLAGS@ \
				-lm @THREADLD_FLAGS@

