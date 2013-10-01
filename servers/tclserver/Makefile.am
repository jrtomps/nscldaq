bin_PROGRAMS = tclserver
tclserver_SOURCES = TclAuthorizer.cpp serverinstance.cpp tclAppInit.cpp \
	tkAppInit.cpp Server.cpp

tclserver_LDADD = @LIBTCLPLUS_LDFLAGS@ \
	 @top_builddir@/servers/portmanager/libPortManager.la \
	 @top_builddir@/base/CopyrightTools/libLicense.la    \
	 @top_builddir@/base/os/libdaqshm.la	\
	 @top_builddir@/base/exception/libException.la    \
	$(TCL_LDFLAGS) \
	$(X11LIBS)  	\
	$(THREADLD_FLAGS)

tclserver_CXXFLAGS	=	$(THREADCXX_FLAGS) $(AM_CXXFLAGS)

tclserver_LDFLAGS=-Wl,"-rpath=$(libdir)"

INCLUDES = -I@top_srcdir@/base/headers -I@top_srcdir@/base/exception \
	-I@top_srcdir@/servers/portmanager -I@top_srcdir@/base/os \
	@LIBTCLPLUS_CFLAGS@ -I@top_srcdir@/base/CopyrightTools $(TCL_FLAGS)

noinst_HEADERS = TclAuthorizer.h \
	server.h \
	serverinstance.h


EXTRA_DIST = tclserver.xml
